#include "pg_decoder/pg_type1.h"
#include "pg_decoder/pg_decoder.h"
#include <cstring>
#include <cstdio>

extern int tests_passed;
extern int tests_failed;
void check(bool condition, const char* name);

static void build_type1(uint8_t buf[32], const uint32_t dwords[8]) {
    std::memcpy(buf, dwords, 32);
}

int test_type1() {
    printf("\n=== Type1 Packet Tests ===\n");

    printf("\n[Type1Header]\n");
    {
        uint32_t dw0 = (0x3Fu << 24) | (1u << 22) | (1u << 21);
        Type1Header h = Type1Header::decode(dw0);
        check(h.pkt_id == 0x3F, "pkt_id=0x3F");
        check(h.fence_scope == FenceScope::BPC, "fence=BPC");
        check(h.barrier, "barrier set");
    }
    {
        uint32_t dw0 = (0x10u << 24) | (2u << 22);
        Type1Header h = Type1Header::decode(dw0);
        check(h.pkt_id == 0x10, "pkt_id=0x10");
        check(h.fence_scope == FenceScope::CU, "fence=CU");
        check(!h.barrier, "no barrier");
    }

    printf("\n[0x3F NOP]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x3F000000u;
        Type1Nop nop = Type1Nop::decode(dwords);
        check(nop.header.pkt_id == 0x3F, "NOP pkt_id");

        uint8_t buf[32];
        build_type1(buf, dwords);
        check(!is_type0_packet(buf), "NOP is not Type0");
        check(get_type1_id(buf) == 0x3F, "get_type1_id=0x3F");
        printf("  decoded: %s\n", nop.to_string().c_str());
    }

    printf("\n[0x10 INDIRECT_BUFFER]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x10000000u;
        dwords[1] = 0x1000;
        dwords[2] = 0x2000;
        dwords[3] = 256;
        Type1IndirectBuffer ib = Type1IndirectBuffer::decode(dwords);
        check(ib.ib_addr_lo == 0x1000, "ib_addr_lo");
        check(ib.ib_addr_hi == 0x2000, "ib_addr_hi");
        check(ib.ib_size == 256, "ib_size=256");
        printf("  decoded: %s\n", ib.to_string().c_str());
    }

    printf("\n[0x11 SET_LOAD_REG]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x11000000u;
        dwords[1] = (4u << 28) | (1u << 27);
        dwords[2] = 0x0000FFFFu;
        dwords[3] = 0x100;
        dwords[4] = (0x10u << 16) | 8;
        Type1SetLoadReg slr = Type1SetLoadReg::decode(dwords);
        check(slr.reg_block_id == RegBlockId::CIM_DIE_REGS, "block=CIM_DIE_REGS");
        check(slr.function, "function=Set");
        check(slr.cim_mask == 0x0000FFFFu, "cim_mask");
        check(slr.base_addr == 0x100, "base_addr=0x100");
        check(slr.reg_offset == 0x10, "reg_offset=0x10");
        check(slr.num_dwords == 8, "num_dwords=8");
        printf("  decoded: %s\n", slr.to_string().c_str());
    }

    printf("\n[0x12 COND_EXEC]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x12000000u;
        dwords[1] = 0;
        dwords[2] = 0x5000; dwords[3] = 0x6000;
        dwords[4] = 4;
        Type1CondExec ce = Type1CondExec::decode(dwords);
        check(!ce.operation, "op=Discard");
        check(ce.cond_exec_addr == 0x600000005000ULL, "cond_addr");
        check(ce.exec_pkt_count == 4, "exec_pkt_count raw=4");
        printf("  decoded: %s\n", ce.to_string().c_str());
    }

    printf("\n[0x13 MEM_SEMAPHORE]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x13000000u;
        dwords[1] = (1u << 1);
        dwords[2] = 0xAAAA0000; dwords[3] = 0xBBBB0000;
        Type1MemSemaphore ms = Type1MemSemaphore::decode(dwords);
        check(ms.sem_type == SemType::Signal, "sem=Signal");
        check(ms.use_mailbox, "mailbox");
        check(ms.address == 0xBBBB0000AAAA0000ULL, "addr");
        printf("  decoded: %s\n", ms.to_string().c_str());
    }

    printf("\n[0x14 SYNC]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x14000000u;
        dwords[1] = (1u << 3) | 3;
        dwords[2] = 0x1000; dwords[3] = 0x2000;
        dwords[4] = 0x42;
        dwords[5] = 0xFFFFFFFFu;
        dwords[6] = 0x10;
        Type1Sync sync = Type1Sync::decode(dwords);
        check(sync.function == SyncFunction::GTE, "fn=GTE");
        check(sync.mem_location, "mem_loc=Host");
        check(sync.ref_value == 0x42, "ref=0x42");
        check(sync.mask == 0xFFFFFFFFu, "mask=all");
        check(sync.poll_interval == 0x10, "interval=0x10");
        printf("  decoded: %s\n", sync.to_string().c_str());
    }

    printf("\n[0x15 EVENT_CREATE]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x15000000u;
        dwords[1] = (1u << 2) | 3;
        dwords[2] = 0x3000; dwords[3] = 0x4000;
        dwords[4] = 0x9999;
        dwords[5] = 0x0000000Fu;
        Type1EventCreate ec = Type1EventCreate::decode(dwords);
        check(ec.int_sel == IntSel::Fence_Interrupt, "int=Fence+Int");
        check(ec.event_type == EventType::EOS, "type=EOS");
        check(ec.fence_addr == 0x400000003000ULL, "fence_addr");
        check(ec.fence_seq == 0x9999, "fence_seq=0x9999");
        printf("  decoded: %s\n", ec.to_string().c_str());
    }

    printf("\n[0x16 COPY_DATA]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x16000000u;
        dwords[1] = (1u << 15) | (2u << 13) | (1u << 12) | 16;
        dwords[2] = 0xDEADBEEF;
        dwords[3] = 0x1000;
        dwords[4] = 0x1;
        Type1CopyData cd = Type1CopyData::decode(dwords);
        check(cd.dst_type == CopyDstType::Mem, "dst=Mem");
        check(cd.src_type == CopySrcType::Value, "src=Value");
        check(cd.context_state == ContextState::Contextual, "ctx=Contextual");
        check(cd.dword_count == 16, "count=16");
        check(cd.src_addr == 0xDEADBEEF, "src=0xDEADBEEF");
        printf("  decoded: %s\n", cd.to_string().c_str());
    }

    printf("\n[0x17 BARRIER_OBJ]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x17000000u;
        dwords[1] = (8u << 2) | 2;
        dwords[2] = (2u << 30) | 0x42;
        dwords[3] = 0xFF;
        Type1BarrierObj bo = Type1BarrierObj::decode(dwords);
        check(bo.barrier_op == BarrierOp::Bar_Init, "op=Bar_Init");
        check(bo.bar_init_value == 8, "init_val=8");
        check(bo.barobj_scope == BarrierScope::CU, "scope=CU");
        check(bo.barobj_id_value == 0x42, "id=0x42");
        printf("  decoded: %s\n", bo.to_string().c_str());
    }

    printf("\n[0x18 SDMA_COPY_LINEAR]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x18000000u;
        dwords[1] = 1023;
        dwords[2] = 0xA000; dwords[3] = 0xB000;
        dwords[4] = 0xC000; dwords[5] = 0xD000;
        Type1SdmaCopyLinear scl = Type1SdmaCopyLinear::decode(dwords);
        check(scl.count == 1023, "count raw=1023");
        check(scl.src_addr == 0xB0000000A000ULL, "src_addr");
        check(scl.dst_addr == 0xD0000000C000ULL, "dst_addr");
        printf("  decoded: %s\n", scl.to_string().c_str());
    }

    printf("\n[0x19 FENCE]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x19000000u;
        dwords[1] = 2;
        dwords[2] = 0x1000; dwords[3] = 0x2000;
        dwords[4] = 0xCAFECAFE;
        dwords[5] = 0x3;
        Type1Fence f = Type1Fence::decode(dwords);
        check(f.mtype == MemType::WT, "mtype=WT");
        check(f.data == 0xCAFECAFE, "data=0xCAFECAFE");
        printf("  decoded: %s\n", f.to_string().c_str());
    }

    printf("\n[0x1A SDMA_TRAP]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x1A000000u;
        dwords[1] = 0xABCDEF0u;
        Type1SdmaTrap trap = Type1SdmaTrap::decode(dwords);
        check(trap.int_context == 0xABCDEF0u, "context");
        printf("  decoded: %s\n", trap.to_string().c_str());
    }

    printf("\n[0x1B SDMA_CONST_FILL]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x1B000000u;
        dwords[1] = (2u << 30) | (10u << 8);
        dwords[2] = 0x1000; dwords[3] = 0x2000;
        dwords[4] = 0xDEADBEEF;
        Type1SdmaConstFill cf = Type1SdmaConstFill::decode(dwords);
        check(cf.fill_size == FillSize::Fill_32bit, "size=32bit");
        check(cf.fill_count == 10, "count raw=10");
        check(cf.src_data_lo == 0xDEADBEEF, "data_lo");
        printf("  decoded: %s\n", cf.to_string().c_str());
    }

    printf("\n[0x1C TIMESTAMP]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x1C000000u;
        dwords[1] = 0x88880000; dwords[2] = 0x99990000;
        Type1Timestamp ts = Type1Timestamp::decode(dwords);
        check(ts.addr == 0x9999000088880000ULL, "addr");
        printf("  decoded: %s\n", ts.to_string().c_str());
    }

    printf("\n[0x1D RDMA_WRITE_INLINE]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x1D000000u;
        dwords[1] = 0x1000; dwords[2] = 0x2000;
        dwords[3] = 63;
        dwords[4] = 0x3000; dwords[5] = 0x4000;
        Type1RdmaWriteInline rdma = Type1RdmaWriteInline::decode(dwords);
        check(rdma.length == 63, "length raw=63");
        check(rdma.dst_addr == 0x200000001000ULL, "dst_addr");
        check(rdma.data_addr == 0x400000003000ULL, "data_addr");
        printf("  decoded: %s\n", rdma.to_string().c_str());
    }

    printf("\n[0x1E HWCOUNTERS_SET_PROFILE]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x1E000000u;
        dwords[1] = (3u << 23) | (1u << 22) | (1u << 20) | (2u << 18) | (5u << 8) | 10;
        dwords[2] = 0x1000; dwords[3] = 0x2000;
        dwords[4] = 7;
        Type1HwCountersSetProfile hp = Type1HwCountersSetProfile::decode(dwords);
        check(hp.op_mode == OpMode::ENABLE_PROFILING, "op_mode=ENABLE_PROFILING");
        check(hp.profiling_mode == ProfilingMode::PACKET_DRIVEN, "mode=PACKET_DRIVEN");
        check(hp.cp_io_die_cg_sel == CounterGroup::GROUP_1, "cp_cg=1");
        check(hp.cim_die_cg_sel == CounterGroup::GROUP_2, "cim_cg=2");
        check(hp.threshold_size == 5, "thresh=5");
        check(hp.profiling_interval == 10, "interval=10");
        check(hp.memory_size == 7, "mem_size raw=7");
        printf("  decoded: %s\n", hp.to_string().c_str());
    }

    printf("\n[0x1F INV_TLB]\n");
    {
        uint32_t dwords[8] = {};
        dwords[0] = 0x1F000000u;
        dwords[1] = (3u << 20) | (2u << 10) | 1;
        dwords[2] = (1u << 19) | 0x55555;
        dwords[3] = (7u << 20) | (6u << 10) | 5;
        dwords[4] = 0x33333;
        dwords[5] = (1u << 4) | 3;
        Type1InvTlb tlb = Type1InvTlb::decode(dwords);
        check(tlb.vpn0 == 1, "vpn0=1");
        check(tlb.vpn1 == 2, "vpn1=2");
        check(tlb.vpn2 == 3, "vpn2=3");
        check(tlb.vpn3 == 0x55555, "vpn3");
        check(tlb.vpn_vld, "vpn_vld");
        check(tlb.mask_vpn0 == 5, "mask0=5");
        check(tlb.mask_vpn1 == 6, "mask1=6");
        check(tlb.mask_vpn2 == 7, "mask2=7");
        check(tlb.mask_vpn3 == 0x33333, "mask3");
        check(tlb.vmid == 3, "vmid=3");
        check(tlb.vmid_vld, "vmid_vld");
        printf("  decoded: %s\n", tlb.to_string().c_str());
    }

    return 0;
}
