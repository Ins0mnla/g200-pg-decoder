#include "pg_decoder/pg_type0.h"
#include <sstream>
#include <stdexcept>

namespace pg {

Type0Header Type0Header::decode(uint16_t val) {
    Type0Header h;
    h.pkt_id = static_cast<uint8_t>(extract_bits(val, 5, 0));
    h.queue_barrier = extract_bits(val, 6, 6) != 0;
    h.mem_acquire_fence = static_cast<Type0FenceScope>(extract_bits(val, 8, 7));
    h.mem_release_fence = static_cast<Type0FenceScope>(extract_bits(val, 10, 9));
    h.reserved_b11 = extract_bits(val, 11, 11) != 0;
    h.reserved = static_cast<uint8_t>(extract_bits(val, 15, 12));
    return h;
}

std::string Type0Header::to_string() const {
    std::ostringstream oss;
    oss << "pkt_id=0x" << std::hex << static_cast<unsigned>(pkt_id)
        << std::dec << " q_barrier=" << queue_barrier
        << " acq_fence=" << static_cast<unsigned>(mem_acquire_fence)
        << " rel_fence=" << static_cast<unsigned>(mem_release_fence);
    return oss.str();
}

Type0CompletionSignal Type0CompletionSignal::decode(uint32_t dw_lo, uint32_t dw_hi) {
    Type0CompletionSignal cs;
    uint64_t hi_addr = extract_bits(dw_hi, 19, 0);
    cs.signal_addr = (hi_addr << 32) | dw_lo;
    cs.barobj_id = static_cast<uint16_t>(extract_bits(dw_hi, 29, 20));
    cs.barrier_scope = static_cast<BarrierScope>(extract_bits(dw_hi, 31, 30));
    return cs;
}

std::string Type0CompletionSignal::to_string() const {
    std::ostringstream oss;
    oss << "COMPLETION{addr=0x" << std::hex << signal_addr
        << " barobj_id=" << std::dec << barobj_id
        << " scope=" << barrier_scope_name(barrier_scope) << "}";
    return oss.str();
}

Type0KernelDispatch Type0KernelDispatch::decode(const uint32_t* dwords) {
    Type0KernelDispatch kd;
    kd.header = Type0Header::decode(static_cast<uint16_t>(dwords[0] & 0xFFFFu));
    if (kd.header.pkt_id != 1)
        throw std::runtime_error("KernelDispatch: bad pkt_id");

    kd.workgroup_size_x = static_cast<uint16_t>(extract_bits(dwords[0], 31, 16));
    kd.workgroup_size_y = static_cast<uint16_t>(extract_bits(dwords[1], 15, 0));
    kd.workgroup_size_z = static_cast<uint16_t>(extract_bits(dwords[1], 31, 16));
    kd.cu_mask = dwords[2];
    kd.grid_size_x = dwords[3];
    kd.grid_size_y = dwords[4];
    kd.grid_size_z = dwords[5];

    kd.kernel_dim = static_cast<uint8_t>(extract_bits(dwords[6], 15, 14));
    kd.ker_salu_arg_size = static_cast<uint8_t>(extract_bits(dwords[6], 7, 0));
    kd.group_segment_size = static_cast<uint16_t>(extract_bits(dwords[6], 31, 16));

    kd.swg_size_x = extract_bits(dwords[7], 27, 0);
    kd.swg_size_y = static_cast<uint16_t>(extract_bits(dwords[8], 15, 0));
    kd.swg_size_z = static_cast<uint16_t>(extract_bits(dwords[8], 31, 16));

    kd.cwg_x = static_cast<uint8_t>(extract_bits(dwords[9], 3, 0));
    kd.cwg_y = static_cast<uint8_t>(extract_bits(dwords[9], 7, 4));
    kd.cwg_z = static_cast<uint8_t>(extract_bits(dwords[9], 11, 8));
    kd.scx = static_cast<uint8_t>(extract_bits(dwords[9], 15, 12));
    kd.scy = static_cast<uint8_t>(extract_bits(dwords[9], 19, 16));
    kd.scz = static_cast<uint8_t>(extract_bits(dwords[9], 23, 20));

    kd.kernel_object = make_u64(dwords[10], dwords[11]);
    kd.kernarg_address = make_u64(dwords[12], dwords[13]);
    kd.completion = Type0CompletionSignal::decode(dwords[14], dwords[15]);
    return kd;
}

std::string Type0KernelDispatch::to_string() const {
    std::ostringstream oss;
    oss << "KERNEL_DISPATCH{ " << header.to_string()
        << " wg=(" << workgroup_size_x << "," << workgroup_size_y << "," << workgroup_size_z << ")"
        << " grid=(" << grid_size_x << "," << grid_size_y << "," << grid_size_z << ")"
        << " dim=" << (int)kernel_dim
        << " cu_mask=0x" << std::hex << cu_mask << std::dec
        << " ker_salu_arg=" << (int)ker_salu_arg_size
        << " group_seg=" << group_segment_size << "KB"
        << " swg=(" << swg_size_x+1 << "," << swg_size_y+1 << "," << swg_size_z+1 << ")"
        << " cwg=(" << (cwg_x+1) << "," << (cwg_y+1) << "," << (cwg_z+1) << ")"
        << " sc=(" << (int)scx << "," << (int)scy << "," << (int)scz << ")"
        << " kern_obj=0x" << std::hex << kernel_object
        << " kernarg=0x" << kernarg_address
        << " " << completion.to_string() << " }";
    return oss.str();
}

Type0BarrierAndOr Type0BarrierAndOr::decode(const uint32_t* dwords) {
    Type0BarrierAndOr bao;
    bao.header = Type0Header::decode(static_cast<uint16_t>(dwords[0] & 0xFFFFu));
    if (bao.header.pkt_id != 2)
        throw std::runtime_error("BarrierAndOr: bad pkt_id");

    bao.barrier_type = static_cast<BarrierType>(extract_bits(dwords[0], 16, 16));
    bao.dep_signal[0] = make_u64(dwords[4], dwords[5]);
    bao.dep_signal[1] = make_u64(dwords[6], dwords[7]);
    bao.dep_signal[2] = make_u64(dwords[8], dwords[9]);
    bao.dep_signal[3] = make_u64(dwords[10], dwords[11]);
    bao.dep_signal[4] = make_u64(dwords[12], dwords[13]);
    bao.completion = Type0CompletionSignal::decode(dwords[14], dwords[15]);
    return bao;
}

std::string Type0BarrierAndOr::to_string() const {
    std::ostringstream oss;
    oss << "BARRIER_AND_OR{ " << header.to_string()
        << " type=" << (barrier_type == BarrierType::AND ? "AND" : "OR");
    for (int i = 0; i < 5; i++)
        oss << " dep" << i << "=0x" << std::hex << dep_signal[i];
    oss << std::dec << " " << completion.to_string() << " }";
    return oss.str();
}

Type0BindReleaseQueues Type0BindReleaseQueues::decode(const uint32_t* dwords) {
    Type0BindReleaseQueues brq;
    brq.header = Type0Header::decode(static_cast<uint16_t>(dwords[0] & 0xFFFFu));
    if (brq.header.pkt_id != 3)
        throw std::runtime_error("BindReleaseQueues: bad pkt_id");

    brq.hcqd_id = static_cast<uint8_t>(extract_bits(dwords[0], 23, 16));
    brq.active = extract_bits(dwords[0], 24, 24) != 0;
    brq.bind_operation = extract_bits(dwords[0], 25, 25) != 0;
    brq.doorbell_id_mask = extract_bits(dwords[0], 26, 26) != 0;
    brq.auto_polling = extract_bits(dwords[0], 27, 27) != 0;
    brq.vmid = static_cast<uint8_t>(extract_bits(dwords[1], 3, 0));
    brq.time_weight = static_cast<uint8_t>(extract_bits(dwords[1], 11, 4));
    brq.polling_interval = dwords[2];
    brq.mcqd_addr = make_u64(dwords[3], dwords[4]);
    brq.ring_addr = make_u64(dwords[5], dwords[6]);
    brq.wptr_addr = make_u64(dwords[7], dwords[8]);
    brq.rptr_addr = make_u64(dwords[9], dwords[10]);
    brq.write_pointer = make_u64(dwords[11], dwords[12]);
    brq.ring_len = dwords[13];
    brq.completion = Type0CompletionSignal::decode(dwords[14], dwords[15]);
    return brq;
}

std::string Type0BindReleaseQueues::to_string() const {
    std::ostringstream oss;
    oss << "BIND_RELEASE_QUEUES{ " << header.to_string()
        << " hcqd=" << (int)hcqd_id << " act=" << active
        << " op=" << (bind_operation ? "BIND" : "RELEASE")
        << " db_mask=" << doorbell_id_mask << " auto_poll=" << auto_polling
        << " vmid=" << (int)vmid << " tw=" << (int)time_weight
        << " poll_int=" << polling_interval
        << " mcqd=0x" << std::hex << mcqd_addr
        << " ring=0x" << ring_addr
        << " wptr_addr=0x" << wptr_addr
        << " rptr_addr=0x" << rptr_addr
        << " wptr=0x" << write_pointer
        << std::dec << " ring_len=" << (ring_len+1)
        << " " << completion.to_string() << " }";
    return oss.str();
}

Type0Atomic Type0Atomic::decode(const uint32_t* dwords) {
    Type0Atomic at;
    at.header = Type0Header::decode(static_cast<uint16_t>(dwords[0] & 0xFFFFu));
    if (at.header.pkt_id != 4)
        throw std::runtime_error("Atomic: bad pkt_id");

    at.operation = static_cast<AtomicOp>(extract_bits(dwords[1], 6, 0));
    at.loop_mode = extract_bits(dwords[1], 7, 7) != 0;
    at.loop_interval = static_cast<uint16_t>(extract_bits(dwords[1], 23, 8));
    at.dst_addr = make_u64(dwords[2], dwords[3]);
    at.data = make_u64(dwords[4], dwords[5]);
    at.cmp_data = make_u64(dwords[6], dwords[7]);
    at.completion = Type0CompletionSignal::decode(dwords[14], dwords[15]);
    return at;
}

std::string Type0Atomic::to_string() const {
    std::ostringstream oss;
    oss << "ATOMIC{ " << header.to_string()
        << " op=0x" << std::hex << (int)static_cast<uint8_t>(operation)
        << std::dec << " loop=" << loop_mode
        << " interval=" << (loop_interval + 1)
        << " dst=0x" << std::hex << dst_addr
        << " data=0x" << data
        << " cmp=0x" << cmp_data
        << " " << completion.to_string() << " }";
    return oss.str();
}

Type0SdmaCcl Type0SdmaCcl::decode(const uint32_t* dwords) {
    Type0SdmaCcl sc;
    sc.header = Type0Header::decode(static_cast<uint16_t>(dwords[0] & 0xFFFFu));
    if (sc.header.pkt_id != 5)
        throw std::runtime_error("SdmaCcl: bad pkt_id");

    sc.dma_type = static_cast<CclDmaType>(extract_bits(dwords[0], 17, 16));
    sc.op_dir = static_cast<CclOpDir>(extract_bits(dwords[0], 18, 18));
    sc.data_format = static_cast<CclDataFormat>(extract_bits(dwords[0], 21, 19));
    sc.reduce_op = static_cast<CclReduceOp>(extract_bits(dwords[0], 23, 22));
    sc.ccl_operation = static_cast<CclOperation>(extract_bits(dwords[0], 26, 24));
    sc.rank_id = static_cast<uint8_t>(extract_bits(dwords[0], 31, 27));
    sc.data_len = extract_bits(dwords[1], 23, 0);
    sc.addr_mode = extract_bits(dwords[1], 30, 30) != 0;
    sc.header_enable = extract_bits(dwords[1], 31, 31) != 0;
    sc.gpu_id = static_cast<uint16_t>(extract_bits(dwords[2], 10, 0));
    sc.unique_id = static_cast<uint16_t>(extract_bits(dwords[2], 31, 16));
    sc.cu_mask = dwords[3];
    sc.src_dst_addr0 = make_u64(dwords[4], dwords[5]);
    sc.src_dst_addr1 = make_u64(dwords[6], dwords[7]);
    sc.src_dst_addr2 = make_u64(dwords[8], dwords[9]);
    sc.dst_src_addr0 = make_u64(dwords[10], dwords[11]);
    sc.dst_src_addr1 = make_u64(dwords[12], dwords[13]);
    sc.completion = Type0CompletionSignal::decode(dwords[14], dwords[15]);
    return sc;
}

std::string Type0SdmaCcl::to_string() const {
    std::ostringstream oss;
    oss << "SDMA_CCL{ " << header.to_string()
        << " type=" << (int)static_cast<uint8_t>(dma_type)
        << " dir=" << (int)static_cast<uint8_t>(op_dir)
        << " fmt=" << (int)static_cast<uint8_t>(data_format)
        << " rop=" << (int)static_cast<uint8_t>(reduce_op)
        << " op=" << (int)static_cast<uint8_t>(ccl_operation)
        << " rank=" << (int)rank_id
        << " data_len=" << (data_len+1) << "B"
        << " hdr_en=" << header_enable << " am=" << addr_mode
        << " uid=" << unique_id << " gpu=" << gpu_id
        << " cu=0x" << std::hex << cu_mask
        << " sda0=0x" << src_dst_addr0
        << " sda1=0x" << src_dst_addr1
        << " sda2=0x" << src_dst_addr2
        << " dsa0=0x" << dst_src_addr0
        << " dsa1=0x" << dst_src_addr1
        << " " << completion.to_string() << " }";
    return oss.str();
}

Type0SdmaPtepde Type0SdmaPtepde::decode(const uint32_t* dwords) {
    Type0SdmaPtepde sp;
    sp.header = Type0Header::decode(static_cast<uint16_t>(dwords[0] & 0xFFFFu));
    if (sp.header.pkt_id != 6)
        throw std::runtime_error("SdmaPtepde: bad pkt_id");

    sp.cache_update = extract_bits(dwords[0], 16, 16) != 0;
    sp.dst_addr = make_u64(dwords[1], dwords[2]);
    sp.flag_mask = make_u64(dwords[3], dwords[4]);
    sp.value_addr = make_u64(dwords[5], dwords[6]);
    sp.incr = dwords[7];
    sp.entry_count = dwords[8];
    sp.completion = Type0CompletionSignal::decode(dwords[14], dwords[15]);
    return sp;
}

std::string Type0SdmaPtepde::to_string() const {
    std::ostringstream oss;
    oss << "SDMA_PTEPDE{ " << header.to_string()
        << " cache_upd=" << cache_update
        << " dst=0x" << std::hex << dst_addr
        << " flag=0x" << flag_mask
        << " val=0x" << value_addr
        << std::dec << " incr=" << incr
        << " entries=" << (entry_count+1)
        << " " << completion.to_string() << " }";
    return oss.str();
}

Type0FlushInvCache Type0FlushInvCache::decode(const uint32_t* dwords) {
    Type0FlushInvCache fic;
    fic.header = Type0Header::decode(static_cast<uint16_t>(dwords[0] & 0xFFFFu));
    if (fic.header.pkt_id != 7)
        throw std::runtime_error("FlushInvCache: bad pkt_id");

    fic.cache_type = static_cast<CacheType>(extract_bits(dwords[0], 17, 16));
    fic.cache_policy = static_cast<CachePolicy>(extract_bits(dwords[0], 19, 18));
    fic.int_type = static_cast<IntType>(extract_bits(dwords[0], 21, 20));
    fic.fence_addr = make_u64(dwords[1], dwords[2]);
    fic.fence_seq = make_u64(dwords[3], dwords[4]);
    fic.completion = Type0CompletionSignal::decode(dwords[14], dwords[15]);
    return fic;
}

std::string Type0FlushInvCache::to_string() const {
    std::ostringstream oss;
    oss << "FLUSH_INV_CACHE{ " << header.to_string()
        << " cache_type=" << (int)static_cast<uint8_t>(cache_type)
        << " policy=" << (int)static_cast<uint8_t>(cache_policy)
        << " int=" << (int)static_cast<uint8_t>(int_type)
        << " f_addr=0x" << std::hex << fence_addr
        << " f_seq=0x" << fence_seq
        << " " << completion.to_string() << " }";
    return oss.str();
}

Type0SdmaCopyLinearRect Type0SdmaCopyLinearRect::decode(const uint32_t* dwords) {
    Type0SdmaCopyLinearRect sclr;
    sclr.header = Type0Header::decode(static_cast<uint16_t>(dwords[0] & 0xFFFFu));
    if (sclr.header.pkt_id != 8)
        throw std::runtime_error("SdmaCopyLinearRect: bad pkt_id");

    sclr.src_addr = make_u64(dwords[1], dwords[2]);
    sclr.src_offset_x = static_cast<uint16_t>(extract_bits(dwords[3], 13, 0));
    sclr.src_offset_y = static_cast<uint16_t>(extract_bits(dwords[3], 29, 16));
    sclr.src_pitch = extract_bits(dwords[4], 31, 13);
    sclr.src_offset_z = static_cast<uint16_t>(extract_bits(dwords[5], 10, 0));
    sclr.src_slice_pitch = extract_bits(dwords[6], 27, 0);
    sclr.dst_addr = make_u64(dwords[7], dwords[8]);
    sclr.dst_offset_x = static_cast<uint16_t>(extract_bits(dwords[9], 13, 0));
    sclr.dst_offset_y = static_cast<uint16_t>(extract_bits(dwords[9], 29, 16));
    sclr.dst_pitch = extract_bits(dwords[10], 31, 13);
    sclr.dst_offset_z = static_cast<uint16_t>(extract_bits(dwords[11], 10, 0));
    sclr.dst_slice_pitch = extract_bits(dwords[12], 27, 0);
    sclr.rect_x = static_cast<uint16_t>(extract_bits(dwords[13], 13, 0));
    sclr.rect_y = static_cast<uint16_t>(extract_bits(dwords[13], 29, 16));
    sclr.rect_z = static_cast<uint16_t>(extract_bits(dwords[14], 10, 0));
    sclr.granularity = static_cast<uint8_t>(extract_bits(dwords[14], 13, 11));
    sclr.src_swap = static_cast<ByteSwap>(extract_bits(dwords[15], 1, 0));
    sclr.dst_swap = static_cast<ByteSwap>(extract_bits(dwords[15], 9, 8));
    return sclr;
}

std::string Type0SdmaCopyLinearRect::to_string() const {
    std::ostringstream oss;
    oss << "SDMA_COPY_LINEAR_RECT{ " << header.to_string()
        << " src=0x" << std::hex << src_addr
        << " soff=(" << std::dec << src_offset_x << "," << src_offset_y << "," << src_offset_z << ")"
        << " spitch=" << (src_pitch+1) << " sslice=" << (src_slice_pitch+1)
        << " dst=0x" << std::hex << dst_addr
        << " doff=(" << std::dec << dst_offset_x << "," << dst_offset_y << "," << dst_offset_z << ")"
        << " dpitch=" << (dst_pitch+1) << " dslice=" << (dst_slice_pitch+1)
        << " rect=(" << (rect_x+1) << "," << (rect_y+1) << "," << (rect_z+1) << ")"
        << " gran=" << (int)granularity
        << " swap_src=" << (int)static_cast<uint8_t>(src_swap)
        << " swap_dst=" << (int)static_cast<uint8_t>(dst_swap) << " }";
    return oss.str();
}

Type0UserTask Type0UserTask::decode(const uint32_t* dwords) {
    Type0UserTask ut;
    ut.header = Type0Header::decode(static_cast<uint16_t>(dwords[0] & 0xFFFFu));
    if (ut.header.pkt_id != 9)
        throw std::runtime_error("UserTask: bad pkt_id");

    ut.scope = static_cast<uint16_t>(extract_bits(dwords[0], 31, 16));
    ut.function = dwords[1];
    ut.cu_mask = dwords[3];
    ut.user_field0 = dwords[4];
    ut.user_field1 = dwords[5];
    ut.src_addr = make_u64(dwords[6], dwords[7]);
    ut.dst_addr = make_u64(dwords[8], dwords[9]);
    ut.completion = Type0CompletionSignal::decode(dwords[14], dwords[15]);
    return ut;
}

std::string Type0UserTask::to_string() const {
    std::ostringstream oss;
    oss << "USER_TASK{ " << header.to_string()
        << " scope=0x" << std::hex << scope
        << " func=0x" << function
        << " cu=0x" << cu_mask
        << " uf0=0x" << user_field0
        << " uf1=0x" << user_field1
        << " src=0x" << src_addr
        << " dst=0x" << dst_addr
        << " " << completion.to_string() << " }";
    return oss.str();
}

} // namespace pg
