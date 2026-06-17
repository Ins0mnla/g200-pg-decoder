#pragma once

#include "pg_common.h"
#include <string>
#include <cstdint>

namespace pg {

struct Type0Header {
    uint8_t  pkt_id;                     // [5:0]
    bool     queue_barrier;              // [6]
    Type0FenceScope mem_acquire_fence;   // [8:7]
    Type0FenceScope mem_release_fence;   // [10:9]
    bool     reserved_b11;               // [11]
    uint8_t  reserved;                   // [15:12]

    static Type0Header decode(uint16_t val);
    std::string to_string() const;
};

struct Type0CompletionSignal {
    uint64_t signal_addr;              // [51:0]
    uint16_t barobj_id;                // [61:52]
    BarrierScope barrier_scope;        // [63:62]

    static Type0CompletionSignal decode(uint32_t dw_lo, uint32_t dw_hi);
    std::string to_string() const;
};

// 0x1 — PG_TYPE0_KERNEL_DISPATCH (64 Bytes)
struct Type0KernelDispatch {
    Type0Header header;
    uint16_t workgroup_size_x;              // DW0[31:16]
    uint16_t workgroup_size_y;              // DW1[15:0]
    uint16_t workgroup_size_z;              // DW1[31:16]
    uint32_t cu_mask;                       // DW2
    uint32_t grid_size_x;                   // DW3
    uint32_t grid_size_y;                   // DW4
    uint32_t grid_size_z;                   // DW5
    uint8_t  kernel_dim;                    // DW6[15:14] (enc: 1=1D, 2=2D, 3=3D)
    uint8_t  ker_salu_arg_size;             // DW6[7:0] (unit=4 const-regs)
    uint16_t group_segment_size;            // DW6[31:16] (unit=1KB)
    uint32_t swg_size_x;                    // DW7[27:0] (actual = swg_size_x + 1)
    uint16_t swg_size_y;                    // DW8[15:0] (actual = swg_size_y + 1)
    uint16_t swg_size_z;                    // DW8[31:16] (actual = swg_size_z + 1)
    uint8_t  cwg_x;                         // DW9[3:0] (actual = cwg_x + 1)
    uint8_t  cwg_y;                         // DW9[7:4] (actual = cwg_y + 1)
    uint8_t  cwg_z;                         // DW9[11:8] (actual = cwg_z + 1)
    uint8_t  scx;                           // DW9[15:12]
    uint8_t  scy;                           // DW9[19:16]
    uint8_t  scz;                           // DW9[23:20]
    uint64_t kernel_object;                 // DW10-DW11
    uint64_t kernarg_address;               // DW12-DW13
    Type0CompletionSignal completion;       // DW14-DW15

    static Type0KernelDispatch decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x2 — PG_TYPE0_BARRIER_AND_OR (64 Bytes)
struct Type0BarrierAndOr {
    Type0Header header;
    BarrierType barrier_type;               // DW0[16]
    uint64_t dep_signal[5];                 // DW4-5 to DW12-13
    Type0CompletionSignal completion;       // DW14-DW15

    static Type0BarrierAndOr decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x3 — PG_TYPE0_BIND_RELEASE_QUEUES (64 Bytes)
struct Type0BindReleaseQueues {
    Type0Header header;
    uint8_t  hcqd_id;                       // DW0[23:16]
    bool     active;                        // DW0[24]
    bool     bind_operation;                // DW0[25]
    bool     doorbell_id_mask;              // DW0[26]
    bool     auto_polling;                  // DW0[27]
    uint8_t  vmid;                          // DW1[3:0]
    uint8_t  time_weight;                   // DW1[11:4]
    uint32_t polling_interval;              // DW2
    uint64_t mcqd_addr;                     // DW3-DW4
    uint64_t ring_addr;                     // DW5-DW6
    uint64_t wptr_addr;                     // DW7-DW8
    uint64_t rptr_addr;                     // DW9-DW10
    uint64_t write_pointer;                 // DW11-DW12
    uint32_t ring_len;                      // DW13 (actual = ring_len + 1)
    Type0CompletionSignal completion;       // DW14-DW15

    static Type0BindReleaseQueues decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x4 — PG_TYPE0_ATOMIC (64 Bytes)
struct Type0Atomic {
    Type0Header header;
    AtomicOp   operation;                   // DW1[6:0]
    bool       loop_mode;                   // DW1[7]
    uint16_t   loop_interval;               // DW1[23:8] (actual = loop_interval + 1)
    uint64_t   dst_addr;                    // DW2-DW3
    uint64_t   data;                        // DW4-DW5
    uint64_t   cmp_data;                    // DW6-DW7
    Type0CompletionSignal completion;       // DW14-DW15

    static Type0Atomic decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x5 — PG_TYPE0_SDMA_CCL (64 Bytes)
struct Type0SdmaCcl {
    Type0Header header;
    CclDmaType   dma_type;                  // DW0[17:16]
    CclOpDir     op_dir;                    // DW0[18]
    CclDataFormat data_format;              // DW0[21:19]
    CclReduceOp  reduce_op;                 // DW0[23:22]
    CclOperation ccl_operation;             // DW0[26:24]
    uint8_t     rank_id;                    // DW0[31:27]
    uint32_t    data_len;                   // DW1[23:0] (actual = data_len + 1)
    bool        addr_mode;                  // DW1[30]
    bool        header_enable;              // DW1[31]
    uint16_t    unique_id;                  // DW2[31:16]
    uint16_t    gpu_id;                     // DW2[10:0]
    uint32_t    cu_mask;                    // DW3
    uint64_t    src_dst_addr0;              // DW4-DW5
    uint64_t    src_dst_addr1;              // DW6-DW7
    uint64_t    src_dst_addr2;              // DW8-DW9
    uint64_t    dst_src_addr0;              // DW10-DW11
    uint64_t    dst_src_addr1;              // DW12-DW13
    Type0CompletionSignal completion;       // DW14-DW15

    static Type0SdmaCcl decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x6 — PG_TYPE0_SDMA_PTEPDE (64 Bytes)
struct Type0SdmaPtepde {
    Type0Header header;
    bool       cache_update;                // DW0[16]
    uint64_t   dst_addr;                    // DW1-DW2
    uint64_t   flag_mask;                   // DW3-DW4
    uint64_t   value_addr;                  // DW5-DW6
    uint32_t   incr;                        // DW7
    uint32_t   entry_count;                 // DW8 (actual = entry_count + 1)
    Type0CompletionSignal completion;       // DW14-DW15

    static Type0SdmaPtepde decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x7 — PG_TYPE0_FLUSH_INV_CACHE (64 Bytes)
struct Type0FlushInvCache {
    Type0Header header;
    CacheType   cache_type;                 // DW0[17:16]
    CachePolicy cache_policy;               // DW0[19:18]
    IntType     int_type;                   // DW0[21:20]
    uint64_t    fence_addr;                 // DW1-DW2
    uint64_t    fence_seq;                  // DW3-DW4
    Type0CompletionSignal completion;       // DW14-DW15

    static Type0FlushInvCache decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x8 — PG_TYPE0_SDMA_COPY_LINEAR_RECT (64 Bytes, no completion signal)
struct Type0SdmaCopyLinearRect {
    Type0Header header;
    uint64_t   src_addr;                    // DW1-DW2
    uint16_t   src_offset_x;                // DW3[13:0]
    uint16_t   src_offset_y;                // DW3[29:16]
    uint32_t   src_pitch;                   // DW4[31:13] (actual = src_pitch + 1)
    uint16_t   src_offset_z;                // DW5[10:0]
    uint32_t   src_slice_pitch;             // DW6[27:0] (actual = src_slice_pitch + 1)
    uint64_t   dst_addr;                    // DW7-DW8
    uint16_t   dst_offset_x;                // DW9[13:0]
    uint16_t   dst_offset_y;                // DW9[29:16]
    uint32_t   dst_pitch;                   // DW10[31:13] (actual = dst_pitch + 1)
    uint16_t   dst_offset_z;                // DW11[10:0]
    uint32_t   dst_slice_pitch;             // DW12[27:0] (actual = dst_slice_pitch + 1)
    uint16_t   rect_x;                      // DW13[13:0] (actual = rect_x + 1)
    uint16_t   rect_y;                      // DW13[29:16] (actual = rect_y + 1)
    uint16_t   rect_z;                      // DW14[10:0] (actual = rect_z + 1)
    uint8_t    granularity;                 // DW14[13:11]
    ByteSwap   src_swap;                    // DW15[1:0]
    ByteSwap   dst_swap;                    // DW15[9:8]

    static Type0SdmaCopyLinearRect decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x9 — PG_TYPE0_USER_TASK (64 Bytes)
struct Type0UserTask {
    Type0Header header;
    uint16_t   scope;                       // DW0[31:16]
    uint32_t   function;                    // DW1
    uint32_t   cu_mask;                     // DW3
    uint32_t   user_field0;                 // DW4
    uint32_t   user_field1;                 // DW5
    uint64_t   src_addr;                    // DW6-DW7
    uint64_t   dst_addr;                    // DW8-DW9
    Type0CompletionSignal completion;       // DW14-DW15

    static Type0UserTask decode(const uint32_t* dwords);
    std::string to_string() const;
};

} // namespace pg
