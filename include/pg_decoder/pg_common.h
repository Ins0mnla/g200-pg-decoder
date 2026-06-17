#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>

namespace pg {

// ============================================================
// 通用枚举定义
// ============================================================

// FENCE_SCOPE (Table H-1, Table CS-1)
enum class FenceScope : uint8_t {
    CP   = 0,
    BPC  = 1,
    CU   = 2,
    Reserved = 3,
};

// BARRIER_SCOPE (Appendix A.2)
enum class BarrierScope : uint8_t {
    CP  = 0,
    BPC = 1,
    CU  = 2,
    Reserved = 3,
};

// SYNC FUNCTION (Table 14-1)
enum class SyncFunction : uint8_t {
    EQ     = 0,
    NE     = 1,
    LT     = 2,
    GTE    = 3,
    LTE    = 4,
    GT     = 5,
    NoComp = 6,
    Reserved = 7,
};

// SEM_TYPE (Table 13-1)
enum class SemType : uint8_t {
    Signal = 0,
    Wait   = 1,
};

// BARRIER_OP (Table 17-1)
enum class BarrierOp : uint8_t {
    Bar_Wait    = 0,
    Bar_Arrival = 1,
    Bar_Init    = 2,
    Reserved    = 3,
};

// MTYPE (Table 19-1)
enum class MemType : uint8_t {
    Default = 0,
    WB      = 1,
    WT      = 2,
    UC      = 3,
    WC      = 4,
};

// REG_BLOCK_ID (Table 11-1)
enum class RegBlockId : uint8_t {
    CP_REGS               = 0,
    CIM_DIE_REGS          = 4,
    CLUSTER_REGS          = 5,
    HW_COUNTERS_CONFIG    = 6,
    KERNEL_DISPATCH_COMM  = 7,
    DUPLICATE_REGS        = 8,
};

// INT_SEL (Table 15-1)
enum class IntSel : uint8_t {
    None            = 0,
    Interrupt       = 1,
    Fence           = 2,
    Fence_Interrupt = 3,
};

// EVENT_TYPE (Table 15-2)
enum class EventType : uint8_t {
    EOP = 0,
    EOS = 1,
};

// SRC_TYPE / DST_TYPE (Table 16-1, 16-2)
enum class CopySrcType : uint8_t {
    Reg   = 0,
    Mem   = 1,
    Value = 2,
    NOP   = 3,
};

enum class CopyDstType : uint8_t {
    Reg = 0,
    Mem = 1,
};

enum class ContextState : uint8_t {
    Global     = 0,
    Contextual = 1,
};

// FILL_SIZE (Table 1B-1)
enum class FillSize : uint8_t {
    Fill_8bit  = 0,
    Fill_16bit = 1,
    Fill_32bit = 2,
    Fill_64bit = 3,
};

// PROFILING_MODE (Table 1E-2)
enum class ProfilingMode : uint8_t {
    AUTO          = 0,
    PACKET_DRIVEN = 1,
};

// OP_MODE (Table 1E-3)
enum class OpMode : uint8_t {
    DISABLE          = 0,
    CLEAR            = 1,
    ENABLE_SET       = 2,
    ENABLE_PROFILING = 3,
};

// CG_SEL (Table 1E-1)
enum class CounterGroup : uint8_t {
    GROUP_0 = 0,
    GROUP_1 = 1,
    GROUP_2 = 2,
    GROUP_3 = 3,
};

// ============================================================
// Type0 专用枚举
// ============================================================

enum class Type0FenceScope : uint8_t {
    None        = 0,
    GPU         = 1,
    CrossDevice = 2,
    Reserved    = 3,
};

enum class KernelDim : uint8_t {
    Dim1 = 1,
    Dim2 = 2,
    Dim3 = 3,
};

enum class BarrierType : uint8_t {
    AND = 0,
    OR  = 1,
};

enum class AtomicOp : uint8_t {
    AddInt32  = 0x00, SubInt32  = 0x01, MinUint32 = 0x02, MaxUint32 = 0x03,
    MinSint32 = 0x04, MaxSint32 = 0x05, AndInt32  = 0x06, OrInt32   = 0x07,
    XorInt32  = 0x08, IncUint32 = 0x09, DecUint32 = 0x0A, AddInt64  = 0x0B,
    SubInt64  = 0x0C, MinUint64 = 0x0D, MaxUint64 = 0x0E, MinSint64 = 0x0F,
    MaxSint64 = 0x10, AndInt64  = 0x11, OrInt64   = 0x12, XorInt64  = 0x13,
    IncUint64 = 0x14, DecUint64 = 0x15,
};

enum class CclOperation : uint8_t {
    SendWrite = 0, Gather = 1, Broadcast = 2, RecvRead = 3,
    Reduce = 4, ReduceScatter = 5, AllReduce = 6, AllGather = 7,
};

enum class CclReduceOp : uint8_t { NOP = 0, ADD = 1, AMAX = 2, MAX = 3 };

enum class CclDataFormat : uint8_t {
    FP16 = 0, BF16 = 1, FP32 = 2, INT32 = 4, UINT32 = 5,
};

enum class CclDmaType : uint8_t { SDMA = 0, RDMA = 1, CDMA = 2, PDMA = 3 };
enum class CclOpDir : uint8_t { READ = 0, SEND = 1 };

enum class CachePolicy : uint8_t { WRITE_BACK = 0, INVALIDATE = 1, FLUSH = 2 };
enum class CacheType : uint8_t { L1 = 1, L2 = 2, L1_L2 = 3 };
enum class IntType : uint8_t { None = 0, OnWrite = 1, OnConfirm = 2 };
enum class ByteSwap : uint8_t { None = 0, Swap16 = 1, Swap32 = 2, Swap64 = 3 };

// ============================================================
// 通用结构体
// ============================================================

struct CompletionSignal {
    uint64_t addr;
    uint16_t barobj_id;
    BarrierScope barrier_scope;

    void decode(uint32_t dw_lo, uint32_t dw_hi);
    std::string to_string() const;
};

// ============================================================
// 位操作工具函数
// ============================================================

inline uint32_t extract_bits(uint32_t value, uint8_t high, uint8_t low) {
    uint32_t mask = ((1u << (high - low + 1)) - 1) << low;
    return (value & mask) >> low;
}

inline uint64_t extract_bits64(uint64_t value, uint8_t high, uint8_t low) {
    uint64_t mask = ((1ull << (high - low + 1)) - 1) << low;
    return (value & mask) >> low;
}

inline uint64_t make_u64(uint32_t lo, uint32_t hi) {
    return (static_cast<uint64_t>(hi) << 32) | lo;
}

const char* fence_scope_name(FenceScope s);
const char* barrier_scope_name(BarrierScope s);
const char* sync_function_name(SyncFunction f);
const char* sem_type_name(SemType t);
const char* barrier_op_name(BarrierOp op);
const char* mem_type_name(MemType t);
const char* reg_block_id_name(RegBlockId id);
const char* int_sel_name(IntSel s);
const char* event_type_name(EventType t);
const char* copy_src_type_name(CopySrcType t);
const char* copy_dst_type_name(CopyDstType t);
const char* fill_size_name(FillSize s);
const char* profiling_mode_name(ProfilingMode m);
const char* op_mode_name(OpMode m);
const char* counter_group_name(CounterGroup g);

} // namespace pg
