#pragma once

#include "pg_common.h"
#include <string>
#include <cstdint>

namespace pg {

struct Type1Header {
    uint8_t   pkt_id;           // [31:24]
    FenceScope fence_scope;     // [23:22]
    bool      barrier;          // [21]
    uint32_t  reserved;         // [20:0]

    static Type1Header decode(uint32_t dw0);
    std::string to_string() const;
};

#define DECLARE_TYPE1(name, id) \
    struct Type1##name { \
        Type1Header header; \
        static constexpr uint8_t PKT_ID = id; \
        static Type1##name decode(const uint32_t* dwords); \
        std::string to_string() const; \
    }

// 0x3F — NOP
struct Type1Nop {
    Type1Header header;
    static constexpr uint8_t PKT_ID = 0x3F;
    static Type1Nop decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x10 — INDIRECT_BUFFER
struct Type1IndirectBuffer {
    Type1Header header;
    uint32_t ib_addr_lo, ib_addr_hi;
    uint32_t ib_size;               // DW3[21:0]
    uint32_t reserved4;
    CompletionSignal completion;    // DW5-DW6
    static constexpr uint8_t PKT_ID = 0x10;
    static Type1IndirectBuffer decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x11 — SET_LOAD_REG
struct Type1SetLoadReg {
    Type1Header header;
    RegBlockId reg_block_id;
    bool       function;            // 0=Load, 1=Set
    uint32_t   cim_mask;
    uint32_t   base_addr;
    uint16_t   reg_offset, num_dwords;
    CompletionSignal completion;
    static constexpr uint8_t PKT_ID = 0x11;
    static Type1SetLoadReg decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x12 — COND_EXEC
struct Type1CondExec {
    Type1Header header;
    bool       operation;           // 0=Discard
    uint64_t   cond_exec_addr;
    uint8_t    exec_pkt_count;      // actual = +1
    CompletionSignal completion;
    static constexpr uint8_t PKT_ID = 0x12;
    static Type1CondExec decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x13 — MEM_SEMAPHORE
struct Type1MemSemaphore {
    Type1Header header;
    SemType    sem_type;
    bool       use_mailbox;
    uint64_t   address;
    CompletionSignal completion;
    static constexpr uint8_t PKT_ID = 0x13;
    static Type1MemSemaphore decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x14 — SYNC
struct Type1Sync {
    Type1Header  header;
    SyncFunction function;
    bool         mem_location;      // 0=NonHost, 1=Host
    uint64_t     poll_addr;
    uint32_t     ref_value, mask, poll_interval;
    static constexpr uint8_t PKT_ID = 0x14;
    static Type1Sync decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x15 — EVENT_CREATE
struct Type1EventCreate {
    Type1Header header;
    IntSel     int_sel;
    EventType  event_type;
    uint64_t   fence_addr;
    uint32_t   fence_seq, cu_mask;
    CompletionSignal completion;
    static constexpr uint8_t PKT_ID = 0x15;
    static Type1EventCreate decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x16 — COPY_DATA
struct Type1CopyData {
    Type1Header  header;
    uint16_t     dword_count;
    ContextState context_state;
    CopySrcType  src_type;
    CopyDstType  dst_type;
    uint32_t     src_addr, dst_addr, cu_mask;
    CompletionSignal completion;
    static constexpr uint8_t PKT_ID = 0x16;
    static Type1CopyData decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x17 — BARRIER_OBJ
struct Type1BarrierObj {
    Type1Header header;
    BarrierOp   barrier_op;
    uint8_t     bar_init_value;
    BarrierScope barobj_scope;
    uint32_t    barobj_id_value, cu_mask;
    CompletionSignal completion;
    static constexpr uint8_t PKT_ID = 0x17;
    static Type1BarrierObj decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x18 — SDMA_COPY_LINEAR
struct Type1SdmaCopyLinear {
    Type1Header header;
    uint32_t   count;              // actual bytes = +1
    uint64_t   src_addr, dst_addr;
    static constexpr uint8_t PKT_ID = 0x18;
    static Type1SdmaCopyLinear decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x19 — FENCE
struct Type1Fence {
    Type1Header header;
    MemType    mtype;
    uint64_t   addr;
    uint32_t   data, cu_mask;
    CompletionSignal completion;
    static constexpr uint8_t PKT_ID = 0x19;
    static Type1Fence decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x1A — SDMA_TRAP
struct Type1SdmaTrap {
    Type1Header header;
    uint32_t   int_context;        // [27:0]
    static constexpr uint8_t PKT_ID = 0x1A;
    static Type1SdmaTrap decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x1B — SDMA_CONST_FILL
struct Type1SdmaConstFill {
    Type1Header header;
    FillSize   fill_size;
    uint32_t   fill_count;         // actual = +1
    uint64_t   dst_addr;
    uint32_t   src_data_lo, src_data_hi;
    static constexpr uint8_t PKT_ID = 0x1B;
    static Type1SdmaConstFill decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x1C — TIMESTAMP
struct Type1Timestamp {
    Type1Header header;
    uint64_t   addr;
    CompletionSignal completion;
    static constexpr uint8_t PKT_ID = 0x1C;
    static Type1Timestamp decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x1D — RDMA_WRITE_INLINE
struct Type1RdmaWriteInline {
    Type1Header header;
    uint64_t   dst_addr, data_addr;
    uint32_t   length;             // actual = +1, max 128
    static constexpr uint8_t PKT_ID = 0x1D;
    static Type1RdmaWriteInline decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x1E — HWCOUNTERS_SET_PROFILE
struct Type1HwCountersSetProfile {
    Type1Header    header;
    uint8_t        profiling_interval;
    uint16_t       threshold_size;     // unit=1MB
    CounterGroup   cim_die_cg_sel, cp_io_die_cg_sel;
    ProfilingMode  profiling_mode;
    OpMode         op_mode;
    uint64_t       base_address;
    uint32_t       memory_size;        // actual MB = +1
    CompletionSignal completion;
    static constexpr uint8_t PKT_ID = 0x1E;
    static Type1HwCountersSetProfile decode(const uint32_t* dwords);
    std::string to_string() const;
};

// 0x1F — INV_TLB
struct Type1InvTlb {
    Type1Header header;
    uint32_t   vpn0, vpn1, vpn2, vpn3;
    bool       vpn_vld, vmid_vld;
    uint16_t   mask_vpn0, mask_vpn1, mask_vpn2;
    uint32_t   mask_vpn3;
    uint8_t    vmid;
    CompletionSignal completion;
    static constexpr uint8_t PKT_ID = 0x1F;
    static Type1InvTlb decode(const uint32_t* dwords);
    std::string to_string() const;
};

} // namespace pg
