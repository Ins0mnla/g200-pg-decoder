#include "pg_decoder/pg_common.h"
#include <sstream>

namespace pg {

void CompletionSignal::decode(uint32_t dw_lo, uint32_t dw_hi) {
    uint64_t raw = make_u64(dw_lo, dw_hi);
    addr = extract_bits64(raw, 51, 0);
    barobj_id = static_cast<uint16_t>(extract_bits64(raw, 61, 52));
    barrier_scope = static_cast<BarrierScope>(extract_bits64(raw, 63, 62));
}

std::string CompletionSignal::to_string() const {
    std::ostringstream oss;
    oss << "CompletionSignal{addr=0x" << std::hex << addr << std::dec
        << ", barobj_id=" << barobj_id
        << ", barrier_scope=" << barrier_scope_name(barrier_scope) << "}";
    return oss.str();
}

const char* fence_scope_name(FenceScope s) {
    switch (s) {
        case FenceScope::CP: return "CP";
        case FenceScope::BPC: return "BPC";
        case FenceScope::CU: return "CU";
        default: return "Reserved";
    }
}

const char* barrier_scope_name(BarrierScope s) {
    switch (s) {
        case BarrierScope::CP: return "CP";
        case BarrierScope::BPC: return "BPC";
        case BarrierScope::CU: return "CU";
        default: return "Reserved";
    }
}

const char* sync_function_name(SyncFunction f) {
    switch (f) {
        case SyncFunction::EQ: return "EQ";
        case SyncFunction::NE: return "NE";
        case SyncFunction::LT: return "LT";
        case SyncFunction::GTE: return "GTE";
        case SyncFunction::LTE: return "LTE";
        case SyncFunction::GT: return "GT";
        case SyncFunction::NoComp: return "NoComp";
        default: return "Reserved";
    }
}

const char* sem_type_name(SemType t) {
    switch (t) {
        case SemType::Signal: return "Signal";
        case SemType::Wait: return "Wait";
    }
    return "?";
}

const char* barrier_op_name(BarrierOp op) {
    switch (op) {
        case BarrierOp::Bar_Wait: return "Bar_Wait";
        case BarrierOp::Bar_Arrival: return "Bar_Arrival";
        case BarrierOp::Bar_Init: return "Bar_Init";
        default: return "Reserved";
    }
}

const char* mem_type_name(MemType t) {
    switch (t) {
        case MemType::Default: return "Default";
        case MemType::WB: return "WB";
        case MemType::WT: return "WT";
        case MemType::UC: return "UC";
        case MemType::WC: return "WC";
    }
    return "Reserved";
}

const char* reg_block_id_name(RegBlockId id) {
    switch (id) {
        case RegBlockId::CP_REGS: return "CP_REGS";
        case RegBlockId::CIM_DIE_REGS: return "CIM_DIE_REGS";
        case RegBlockId::CLUSTER_REGS: return "CLUSTER_REGS";
        case RegBlockId::HW_COUNTERS_CONFIG: return "HW_COUNTERS_CONFIG";
        case RegBlockId::KERNEL_DISPATCH_COMM: return "KERNEL_DISPATCH_COMM";
        case RegBlockId::DUPLICATE_REGS: return "DUPLICATE_REGS";
    }
    return "Reserved";
}

const char* int_sel_name(IntSel s) {
    switch (s) {
        case IntSel::None: return "None";
        case IntSel::Interrupt: return "Interrupt";
        case IntSel::Fence: return "Fence";
        case IntSel::Fence_Interrupt: return "Fence+Interrupt";
    }
    return "?";
}

const char* event_type_name(EventType t) {
    switch (t) {
        case EventType::EOP: return "EOP";
        case EventType::EOS: return "EOS";
    }
    return "?";
}

const char* copy_src_type_name(CopySrcType t) {
    switch (t) {
        case CopySrcType::Reg: return "Reg";
        case CopySrcType::Mem: return "Mem";
        case CopySrcType::Value: return "Value";
        case CopySrcType::NOP: return "NOP";
    }
    return "?";
}

const char* copy_dst_type_name(CopyDstType t) {
    switch (t) {
        case CopyDstType::Reg: return "Reg";
        case CopyDstType::Mem: return "Mem";
    }
    return "?";
}

const char* fill_size_name(FillSize s) {
    switch (s) {
        case FillSize::Fill_8bit: return "Fill_8bit";
        case FillSize::Fill_16bit: return "Fill_16bit";
        case FillSize::Fill_32bit: return "Fill_32bit";
        case FillSize::Fill_64bit: return "Fill_64bit";
    }
    return "?";
}

const char* profiling_mode_name(ProfilingMode m) {
    switch (m) {
        case ProfilingMode::AUTO: return "AUTO";
        case ProfilingMode::PACKET_DRIVEN: return "PACKET_DRIVEN";
    }
    return "?";
}

const char* op_mode_name(OpMode m) {
    switch (m) {
        case OpMode::DISABLE: return "DISABLE";
        case OpMode::CLEAR: return "CLEAR";
        case OpMode::ENABLE_SET: return "ENABLE_SET";
        case OpMode::ENABLE_PROFILING: return "ENABLE_PROFILING";
    }
    return "?";
}

const char* counter_group_name(CounterGroup g) {
    switch (g) {
        case CounterGroup::GROUP_0: return "GROUP_0";
        case CounterGroup::GROUP_1: return "GROUP_1";
        case CounterGroup::GROUP_2: return "GROUP_2";
        case CounterGroup::GROUP_3: return "GROUP_3";
    }
    return "?";
}

} // namespace pg
