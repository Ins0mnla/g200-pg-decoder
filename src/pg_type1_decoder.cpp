#include "pg_decoder/pg_type1.h"
#include <sstream>

namespace pg {

Type1Header Type1Header::decode(uint32_t dw0) {
    Type1Header hdr;
    hdr.pkt_id     = static_cast<uint8_t>(extract_bits(dw0, 31, 24));
    hdr.fence_scope = static_cast<FenceScope>(extract_bits(dw0, 23, 22));
    hdr.barrier    = extract_bits(dw0, 21, 21) != 0;
    hdr.reserved   = extract_bits(dw0, 20, 0);
    return hdr;
}

std::string Type1Header::to_string() const {
    std::ostringstream oss;
    oss << "pkt_id=0x" << std::hex << static_cast<int>(pkt_id)
        << " fence=" << fence_scope_name(fence_scope)
        << " barrier=" << (barrier ? "Y" : "N")
        << " res=0x" << reserved;
    return oss.str();
}

Type1Nop Type1Nop::decode(const uint32_t* dwords) {
    Type1Nop result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1Nop: invalid pkt_id");
    return result;
}

std::string Type1Nop::to_string() const {
    return "Type1_NOP { " + header.to_string() + " }";
}

Type1IndirectBuffer Type1IndirectBuffer::decode(const uint32_t* dwords) {
    Type1IndirectBuffer result;
    result.header   = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1IndirectBuffer: invalid pkt_id");
    result.ib_addr_lo = dwords[1];
    result.ib_addr_hi = dwords[2];
    result.ib_size    = extract_bits(dwords[3], 21, 0);
    result.reserved4  = dwords[4];
    result.completion.decode(dwords[5], dwords[6]);
    return result;
}

std::string Type1IndirectBuffer::to_string() const {
    std::ostringstream oss;
    oss << "Type1_INDIRECT_BUFFER { " << header.to_string()
        << " ib_addr=0x" << std::hex << (static_cast<uint64_t>(ib_addr_hi) << 32 | ib_addr_lo)
        << " ib_size=" << std::dec << ib_size
        << " comp=(" << completion.to_string() << ") }";
    return oss.str();
}

Type1SetLoadReg Type1SetLoadReg::decode(const uint32_t* dwords) {
    Type1SetLoadReg result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1SetLoadReg: invalid pkt_id");
    result.reg_block_id = static_cast<RegBlockId>(extract_bits(dwords[1], 31, 28));
    result.function     = extract_bits(dwords[1], 27, 27) != 0;
    result.cim_mask     = dwords[2];
    result.base_addr    = dwords[3];
    result.reg_offset   = static_cast<uint16_t>(extract_bits(dwords[4], 31, 16));
    result.num_dwords   = static_cast<uint16_t>(extract_bits(dwords[4], 15, 0));
    result.completion.decode(dwords[6], dwords[7]);
    return result;
}

std::string Type1SetLoadReg::to_string() const {
    std::ostringstream oss;
    oss << "Type1_SET_LOAD_REG { " << header.to_string()
        << " block=" << reg_block_id_name(reg_block_id)
        << " fn=" << (function ? "Set" : "Load")
        << " cim_mask=0x" << std::hex << cim_mask
        << " base=0x" << base_addr
        << std::dec << " off=" << reg_offset << " num=" << num_dwords
        << " comp=(" << completion.to_string() << ") }";
    return oss.str();
}

Type1CondExec Type1CondExec::decode(const uint32_t* dwords) {
    Type1CondExec result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1CondExec: invalid pkt_id");
    result.operation      = extract_bits(dwords[1], 0, 0) != 0;
    result.cond_exec_addr = (static_cast<uint64_t>(dwords[3]) << 32) | dwords[2];
    result.exec_pkt_count = static_cast<uint8_t>(extract_bits(dwords[4], 7, 0));
    result.completion.decode(dwords[6], dwords[7]);
    return result;
}

std::string Type1CondExec::to_string() const {
    std::ostringstream oss;
    oss << "Type1_COND_EXEC { " << header.to_string()
        << " op=" << (operation ? "ExecIfAny" : "Discard")
        << " cond_addr=0x" << std::hex << cond_exec_addr
        << " count=" << std::dec << (exec_pkt_count + 1)
        << " comp=(" << completion.to_string() << ") }";
    return oss.str();
}

Type1MemSemaphore Type1MemSemaphore::decode(const uint32_t* dwords) {
    Type1MemSemaphore result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1MemSemaphore: invalid pkt_id");
    result.sem_type    = static_cast<SemType>(extract_bits(dwords[1], 0, 0));
    result.use_mailbox = extract_bits(dwords[1], 1, 1) != 0;
    result.address     = (static_cast<uint64_t>(dwords[3]) << 32) | dwords[2];
    result.completion.decode(dwords[6], dwords[7]);
    return result;
}

std::string Type1MemSemaphore::to_string() const {
    std::ostringstream oss;
    oss << "Type1_MEM_SEMAPHORE { " << header.to_string()
        << " sem=" << sem_type_name(sem_type)
        << " mailbox=" << (use_mailbox ? "Y" : "N")
        << " addr=0x" << std::hex << address
        << " comp=(" << completion.to_string() << ") }";
    return oss.str();
}

Type1Sync Type1Sync::decode(const uint32_t* dwords) {
    Type1Sync result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1Sync: invalid pkt_id");
    result.function      = static_cast<SyncFunction>(extract_bits(dwords[1], 2, 0));
    result.mem_location  = extract_bits(dwords[1], 3, 3) != 0;
    result.poll_addr     = (static_cast<uint64_t>(dwords[3]) << 32) | dwords[2];
    result.ref_value     = dwords[4];
    result.mask          = dwords[5];
    result.poll_interval = dwords[6];
    return result;
}

std::string Type1Sync::to_string() const {
    std::ostringstream oss;
    oss << "Type1_SYNC { " << header.to_string()
        << " fn=" << sync_function_name(function)
        << " mem_loc=" << (mem_location ? "Host" : "NonHost")
        << " poll_addr=0x" << std::hex << poll_addr
        << " ref=0x" << ref_value
        << " mask=0x" << mask
        << std::dec << " poll_int=" << poll_interval << " }";
    return oss.str();
}

Type1EventCreate Type1EventCreate::decode(const uint32_t* dwords) {
    Type1EventCreate result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1EventCreate: invalid pkt_id");
    result.int_sel     = static_cast<IntSel>(extract_bits(dwords[1], 1, 0));
    result.event_type  = static_cast<EventType>(extract_bits(dwords[1], 2, 2));
    result.fence_addr  = (static_cast<uint64_t>(dwords[3]) << 32) | dwords[2];
    result.fence_seq   = dwords[4];
    result.cu_mask     = dwords[5];
    result.completion.decode(dwords[6], dwords[7]);
    return result;
}

std::string Type1EventCreate::to_string() const {
    std::ostringstream oss;
    oss << "Type1_EVENT_CREATE { " << header.to_string()
        << " int=" << int_sel_name(int_sel)
        << " type=" << event_type_name(event_type)
        << " f_addr=0x" << std::hex << fence_addr
        << " f_seq=0x" << fence_seq
        << " cu_mask=0x" << cu_mask
        << " comp=(" << completion.to_string() << ") }";
    return oss.str();
}

Type1CopyData Type1CopyData::decode(const uint32_t* dwords) {
    Type1CopyData result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1CopyData: invalid pkt_id");
    result.dword_count   = static_cast<uint16_t>(extract_bits(dwords[1], 11, 0));
    result.context_state = static_cast<ContextState>(extract_bits(dwords[1], 12, 12));
    result.src_type      = static_cast<CopySrcType>(extract_bits(dwords[1], 14, 13));
    result.dst_type      = static_cast<CopyDstType>(extract_bits(dwords[1], 15, 15));
    result.src_addr      = dwords[2];
    result.dst_addr      = dwords[3];
    result.cu_mask       = dwords[4];
    result.completion.decode(dwords[6], dwords[7]);
    return result;
}

std::string Type1CopyData::to_string() const {
    std::ostringstream oss;
    oss << "Type1_COPY_DATA { " << header.to_string()
        << " count=" << dword_count
        << " ctx=" << (context_state == ContextState::Global ? "Global" : "Contextual")
        << " src=" << copy_src_type_name(src_type)
        << " dst=" << copy_dst_type_name(dst_type)
        << " src_addr=0x" << std::hex << src_addr
        << " dst_addr=0x" << dst_addr
        << " cu=0x" << cu_mask
        << " comp=(" << completion.to_string() << ") }";
    return oss.str();
}

Type1BarrierObj Type1BarrierObj::decode(const uint32_t* dwords) {
    Type1BarrierObj result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1BarrierObj: invalid pkt_id");
    result.barrier_op      = static_cast<BarrierOp>(extract_bits(dwords[1], 1, 0));
    result.bar_init_value  = static_cast<uint8_t>(extract_bits(dwords[1], 9, 2));
    result.barobj_scope    = static_cast<BarrierScope>(extract_bits(dwords[2], 31, 30));
    result.barobj_id_value = extract_bits(dwords[2], 29, 0);
    result.cu_mask         = dwords[3];
    result.completion.decode(dwords[6], dwords[7]);
    return result;
}

std::string Type1BarrierObj::to_string() const {
    std::ostringstream oss;
    oss << "Type1_BARRIER_OBJ { " << header.to_string()
        << " op=" << barrier_op_name(barrier_op)
        << " init_val=" << (int)bar_init_value
        << " scope=" << barrier_scope_name(barobj_scope)
        << " id=" << barobj_id_value
        << " cu=0x" << std::hex << cu_mask
        << " comp=(" << completion.to_string() << ") }";
    return oss.str();
}

Type1SdmaCopyLinear Type1SdmaCopyLinear::decode(const uint32_t* dwords) {
    Type1SdmaCopyLinear result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1SdmaCopyLinear: invalid pkt_id");
    result.count    = extract_bits(dwords[1], 23, 0);
    result.src_addr = (static_cast<uint64_t>(dwords[3]) << 32) | dwords[2];
    result.dst_addr = (static_cast<uint64_t>(dwords[5]) << 32) | dwords[4];
    return result;
}

std::string Type1SdmaCopyLinear::to_string() const {
    std::ostringstream oss;
    oss << "Type1_SDMA_COPY_LINEAR { " << header.to_string()
        << " count=" << (count + 1) << "B"
        << " src=0x" << std::hex << src_addr
        << " dst=0x" << dst_addr << " }";
    return oss.str();
}

Type1Fence Type1Fence::decode(const uint32_t* dwords) {
    Type1Fence result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1Fence: invalid pkt_id");
    result.mtype   = static_cast<MemType>(extract_bits(dwords[1], 2, 0));
    result.addr    = (static_cast<uint64_t>(dwords[3]) << 32) | dwords[2];
    result.data    = dwords[4];
    result.cu_mask = dwords[5];
    result.completion.decode(dwords[6], dwords[7]);
    return result;
}

std::string Type1Fence::to_string() const {
    std::ostringstream oss;
    oss << "Type1_FENCE { " << header.to_string()
        << " mtype=" << mem_type_name(mtype)
        << " addr=0x" << std::hex << addr
        << " data=0x" << data
        << " cu=0x" << cu_mask
        << " comp=(" << completion.to_string() << ") }";
    return oss.str();
}

Type1SdmaTrap Type1SdmaTrap::decode(const uint32_t* dwords) {
    Type1SdmaTrap result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1SdmaTrap: invalid pkt_id");
    result.int_context = extract_bits(dwords[1], 27, 0);
    return result;
}

std::string Type1SdmaTrap::to_string() const {
    std::ostringstream oss;
    oss << "Type1_SDMA_TRAP { " << header.to_string()
        << " int_ctx=0x" << std::hex << int_context << " }";
    return oss.str();
}

Type1SdmaConstFill Type1SdmaConstFill::decode(const uint32_t* dwords) {
    Type1SdmaConstFill result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1SdmaConstFill: invalid pkt_id");
    result.fill_size   = static_cast<FillSize>(extract_bits(dwords[1], 31, 30));
    result.fill_count  = extract_bits(dwords[1], 29, 8);
    result.dst_addr    = (static_cast<uint64_t>(dwords[3]) << 32) | dwords[2];
    result.src_data_lo = dwords[4];
    result.src_data_hi = dwords[5];
    return result;
}

std::string Type1SdmaConstFill::to_string() const {
    std::ostringstream oss;
    oss << "Type1_SDMA_CONST_FILL { " << header.to_string()
        << " size=" << fill_size_name(fill_size)
        << " count=" << (fill_count + 1)
        << " dst=0x" << std::hex << dst_addr
        << " data_lo=0x" << src_data_lo
        << " data_hi=0x" << src_data_hi << " }";
    return oss.str();
}

Type1Timestamp Type1Timestamp::decode(const uint32_t* dwords) {
    Type1Timestamp result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1Timestamp: invalid pkt_id");
    result.addr = (static_cast<uint64_t>(dwords[2]) << 32) | dwords[1];
    result.completion.decode(dwords[6], dwords[7]);
    return result;
}

std::string Type1Timestamp::to_string() const {
    std::ostringstream oss;
    oss << "Type1_TIMESTAMP { " << header.to_string()
        << " addr=0x" << std::hex << addr
        << " comp=(" << completion.to_string() << ") }";
    return oss.str();
}

Type1RdmaWriteInline Type1RdmaWriteInline::decode(const uint32_t* dwords) {
    Type1RdmaWriteInline result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1RdmaWriteInline: invalid pkt_id");
    result.dst_addr  = (static_cast<uint64_t>(dwords[2]) << 32) | dwords[1];
    result.length    = extract_bits(dwords[3], 19, 0);
    result.data_addr = (static_cast<uint64_t>(dwords[5]) << 32) | dwords[4];
    return result;
}

std::string Type1RdmaWriteInline::to_string() const {
    std::ostringstream oss;
    oss << "Type1_RDMA_WRITE_INLINE { " << header.to_string()
        << " dst=0x" << std::hex << dst_addr
        << " len=" << std::dec << (length + 1) << "B"
        << " data=0x" << std::hex << data_addr << " }";
    return oss.str();
}

Type1HwCountersSetProfile Type1HwCountersSetProfile::decode(const uint32_t* dwords) {
    Type1HwCountersSetProfile result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1HwCountersSetProfile: invalid pkt_id");
    result.profiling_interval = static_cast<uint8_t>(extract_bits(dwords[1], 7, 0));
    result.threshold_size     = static_cast<uint16_t>(extract_bits(dwords[1], 17, 8));
    result.cim_die_cg_sel     = static_cast<CounterGroup>(extract_bits(dwords[1], 19, 18));
    result.cp_io_die_cg_sel   = static_cast<CounterGroup>(extract_bits(dwords[1], 21, 20));
    result.profiling_mode     = static_cast<ProfilingMode>(extract_bits(dwords[1], 22, 22));
    result.op_mode            = static_cast<OpMode>(extract_bits(dwords[1], 24, 23));
    result.base_address       = (static_cast<uint64_t>(dwords[3]) << 32) | dwords[2];
    result.memory_size        = extract_bits(dwords[4], 9, 0);
    result.completion.decode(dwords[6], dwords[7]);
    return result;
}

std::string Type1HwCountersSetProfile::to_string() const {
    std::ostringstream oss;
    oss << "Type1_HWCOUNTERS_SET_PROFILE { " << header.to_string()
        << " interval=" << (int)profiling_interval
        << " thresh=" << threshold_size
        << " cim_cg=" << counter_group_name(cim_die_cg_sel)
        << " cp_cg=" << counter_group_name(cp_io_die_cg_sel)
        << " mode=" << profiling_mode_name(profiling_mode)
        << " op=" << op_mode_name(op_mode)
        << " base=0x" << std::hex << base_address
        << " mem=" << std::dec << (memory_size + 1) << "MB"
        << " comp=(" << completion.to_string() << ") }";
    return oss.str();
}

Type1InvTlb Type1InvTlb::decode(const uint32_t* dwords) {
    Type1InvTlb result;
    result.header = Type1Header::decode(dwords[0]);
    if (result.header.pkt_id != PKT_ID)
        throw std::runtime_error("Type1InvTlb: invalid pkt_id");
    result.vpn0      = extract_bits(dwords[1], 9, 0);
    result.vpn1      = extract_bits(dwords[1], 19, 10);
    result.vpn2      = extract_bits(dwords[1], 29, 20);
    result.vpn3      = extract_bits(dwords[2], 18, 0);
    result.vpn_vld   = extract_bits(dwords[2], 19, 19) != 0;
    result.mask_vpn0 = static_cast<uint16_t>(extract_bits(dwords[3], 9, 0));
    result.mask_vpn1 = static_cast<uint16_t>(extract_bits(dwords[3], 19, 10));
    result.mask_vpn2 = static_cast<uint16_t>(extract_bits(dwords[3], 29, 20));
    result.mask_vpn3 = extract_bits(dwords[4], 18, 0);
    result.vmid      = static_cast<uint8_t>(extract_bits(dwords[5], 3, 0));
    result.vmid_vld  = extract_bits(dwords[5], 4, 4) != 0;
    result.completion.decode(dwords[6], dwords[7]);
    return result;
}

std::string Type1InvTlb::to_string() const {
    std::ostringstream oss;
    oss << "Type1_INV_TLB { " << header.to_string()
        << " vpn=(" << vpn3 << "," << vpn2 << "," << vpn1 << "," << vpn0 << ")"
        << " vpn_vld=" << vpn_vld
        << " mask_vpn=(" << mask_vpn3 << "," << mask_vpn2 << "," << mask_vpn1 << "," << mask_vpn0 << ")"
        << " vmid=" << (int)vmid << " vmid_vld=" << vmid_vld
        << " comp=(" << completion.to_string() << ") }";
    return oss.str();
}

} // namespace pg
