#include "pg_decoder/pg_type0.h"
#include "pg_decoder/pg_decoder.h"
#include <cstring>
#include <cstdio>

extern int tests_passed;
extern int tests_failed;
void check(bool condition, const char* name);

static void build_type0(uint8_t buf[64], const uint32_t dwords[16]) {
    std::memcpy(buf, dwords, 64);
}

int test_type0() {
    printf("\n=== Type0 Packet Tests ===\n");

    printf("\n[Type0Header]\n");
    {
        Type0Header h = Type0Header::decode(0x0001);
        check(h.pkt_id == 1, "pkt_id=1");
        check(!h.queue_barrier, "no barrier");
    }
    {
        Type0Header h = Type0Header::decode(0x0042);
        check(h.pkt_id == 2, "pkt_id=2");
        check(h.queue_barrier, "barrier set");
        check(h.mem_acquire_fence == Type0FenceScope::GPU, "acq fence GPU");
    }
    {
        Type0Header h = Type0Header::decode(0x0305);
        check(h.pkt_id == 5, "pkt_id=5");
        check(h.mem_acquire_fence == Type0FenceScope::CrossDevice, "acq CrossDevice");
    }

    printf("\n[Type0CompletionSignal]\n");
    {
        Type0CompletionSignal cs = Type0CompletionSignal::decode(0x12345678, 0x00000000);
        check(cs.signal_addr == 0x12345678, "signal addr lo only");
        check(cs.barobj_id == 0, "barobj_id=0");
        check(cs.barrier_scope == BarrierScope::CP, "scope=CP");
    }
    {
        uint32_t hi = (0xABCDE & 0xFFFFF) | (5 << 20) | (2 << 30);
        Type0CompletionSignal cs = Type0CompletionSignal::decode(0x12345678, hi);
        check(cs.signal_addr == 0xABCDE12345678ULL, "signal addr combined");
        check(cs.barobj_id == 5, "barobj_id=5");
        check(cs.barrier_scope == BarrierScope::CU, "scope=CU");
    }

    printf("\n[0x1 KERNEL_DISPATCH]\n");
    {
        uint32_t dwords[16] = {};
        dwords[0] = (256u << 16) | 1;
        dwords[1] = (1u << 16) | 1;
        dwords[2] = 0xFFFFFFFFu;
        dwords[3] = 1024;
        dwords[4] = 1;
        dwords[5] = 1;
        dwords[6] = (16u << 16) | (1u << 14) | 32;
        dwords[7] = 3;
        dwords[8] = (1u << 16) | 1;
        dwords[9] = (1u << 12);
        dwords[10] = 0xDEAD0000;
        dwords[11] = 0xBEEF0000;
        dwords[12] = 0xC0000000;
        dwords[13] = 0x10000000;
        dwords[14] = 0xAAAAAAAA;

        uint8_t buf[64];
        build_type0(buf, dwords);

        Type0KernelDispatch kd = Type0KernelDispatch::decode(reinterpret_cast<const uint32_t*>(buf));
        check(kd.workgroup_size_x == 256, "wg_x=256");
        check(kd.workgroup_size_y == 1, "wg_y=1");
        check(kd.workgroup_size_z == 1, "wg_z=1");
        check(kd.cu_mask == 0xFFFFFFFFu, "cu_mask all");
        check(kd.grid_size_x == 1024, "grid_x=1024");
        check(kd.kernel_dim == 1, "kernel_dim=Dim1");
        check(kd.ker_salu_arg_size == 32, "salu_arg=32");
        check(kd.group_segment_size == 16, "group_seg=16KB");
        check(kd.swg_size_x == 3, "swg_x raw=3");
        check(kd.cwg_x == 0 && kd.cwg_y == 0 && kd.cwg_z == 0, "cwg all 0");
        check(kd.kernel_object == 0xBEEF0000DEAD0000ULL, "kernel_obj");
        check(kd.kernarg_address == 0x10000000C0000000ULL, "kernarg_addr");
        printf("  decoded: %s\n", kd.to_string().c_str());
    }

    printf("\n[0x2 BARRIER_AND_OR]\n");
    {
        uint32_t dwords[16] = {};
        dwords[0] = (1u << 16) | 2;
        dwords[4] = 0x11110000; dwords[5] = 0x22220000;
        dwords[6] = 0x33330000; dwords[7] = 0x44440000;

        uint8_t buf[64];
        build_type0(buf, dwords);

        Type0BarrierAndOr bao = Type0BarrierAndOr::decode(reinterpret_cast<const uint32_t*>(buf));
        check(bao.barrier_type == BarrierType::OR, "OR barrier");
        check(bao.dep_signal[0] == 0x2222000011110000ULL, "dep0");
        check(bao.dep_signal[1] == 0x4444000033330000ULL, "dep1");
        check(bao.dep_signal[2] == 0, "dep2=0");
        printf("  decoded: %s\n", bao.to_string().c_str());
    }

    printf("\n[0x3 BIND_RELEASE_QUEUES]\n");
    {
        uint32_t dwords[16] = {};
        dwords[0] = (1u << 27) | (1u << 26) | (1u << 25) | (1u << 24) | (3u << 16) | 3;
        dwords[1] = (5u << 4) | 1;
        dwords[2] = 1000;
        dwords[3] = 0x1000; dwords[4] = 0x2000;
        dwords[13] = 15;
        dwords[14] = 0x100; dwords[15] = 1;

        uint8_t buf[64];
        build_type0(buf, dwords);

        Type0BindReleaseQueues brq = Type0BindReleaseQueues::decode(reinterpret_cast<const uint32_t*>(buf));
        check(brq.hcqd_id == 3, "hcqd=3");
        check(brq.active, "active");
        check(brq.bind_operation, "bind op");
        check(brq.doorbell_id_mask, "db_mask");
        check(brq.auto_polling, "auto_poll");
        check(brq.vmid == 1, "vmid=1");
        check(brq.time_weight == 5, "time_weight=5");
        check(brq.polling_interval == 1000, "poll_int=1000");
        check(brq.mcqd_addr == 0x200000001000ULL, "mcqd_addr");
        check(brq.ring_len == 15, "ring_len raw=15");
        printf("  decoded: %s\n", brq.to_string().c_str());
    }

    printf("\n[0x4 ATOMIC]\n");
    {
        uint32_t dwords[16] = {};
        dwords[0] = 4;
        dwords[1] = (5u << 8) | 0x01;
        dwords[2] = 0xA0000000; dwords[3] = 0xB0000000;
        dwords[4] = 0x42; dwords[5] = 0;

        uint8_t buf[64];
        build_type0(buf, dwords);

        Type0Atomic at = Type0Atomic::decode(reinterpret_cast<const uint32_t*>(buf));
        check(at.operation == AtomicOp::SubInt32, "op=SubInt32");
        check(!at.loop_mode, "single pass");
        check(at.loop_interval == 5, "interval raw=5");
        check(at.dst_addr == 0xB0000000A0000000ULL, "dst_addr");
        check(at.data == 0x42, "data=0x42");
        printf("  decoded: %s\n", at.to_string().c_str());
    }

    printf("\n[0x5 SDMA_CCL]\n");
    {
        uint32_t dwords[16] = {};
        dwords[0] = (0u << 27) | (2u << 24) | (0u << 22) | (2u << 19) | (1u << 18) | (0u << 16) | 5;
        dwords[1] = 99;
        dwords[3] = 0xFF;

        uint8_t buf[64];
        build_type0(buf, dwords);

        Type0SdmaCcl sc = Type0SdmaCcl::decode(reinterpret_cast<const uint32_t*>(buf));
        check(sc.ccl_operation == CclOperation::Broadcast, "op=Broadcast");
        check(sc.data_format == CclDataFormat::FP32, "fmt=FP32");
        check(sc.op_dir == CclOpDir::SEND, "dir=SEND");
        check(sc.dma_type == CclDmaType::SDMA, "type=SDMA");
        check(sc.data_len == 99, "data_len raw=99");
        printf("  decoded: %s\n", sc.to_string().c_str());
    }

    printf("\n[0x9 USER_TASK]\n");
    {
        uint32_t dwords[16] = {};
        dwords[0] = (0xABCDu << 16) | 9;
        dwords[1] = 0xCAFE0001;
        dwords[4] = 0xDEAD;
        dwords[5] = 0xBEEF;

        uint8_t buf[64];
        build_type0(buf, dwords);

        Type0UserTask ut = Type0UserTask::decode(reinterpret_cast<const uint32_t*>(buf));
        check(ut.scope == 0xABCD, "scope=0xABCD");
        check(ut.function == 0xCAFE0001, "function");
        check(ut.user_field0 == 0xDEAD, "uf0");
        check(ut.user_field1 == 0xBEEF, "uf1");
        printf("  decoded: %s\n", ut.to_string().c_str());
    }

    printf("\n[Decoder dispatch]\n");
    {
        uint32_t dwords[16] = {};
        dwords[0] = (64u << 16) | 1;
        uint8_t buf[64];
        build_type0(buf, dwords);

        check(is_type0_packet(buf), "is_type0");
        check(get_type0_id(buf) == 1, "get_type0_id=1");
        std::string result = decode_packet(buf, 64);
        check(!result.empty(), "decode_packet returns string");
        printf("  decoded: %s\n", result.c_str());
    }

    return 0;
}
