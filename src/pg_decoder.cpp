#include "pg_decoder/pg_decoder.h"
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <sstream>

namespace pg {

bool is_type0_packet(const uint8_t* data) {
    uint32_t dw0;
    std::memcpy(&dw0, data, sizeof(dw0));
    uint8_t t0_id = static_cast<uint8_t>(dw0 & 0x3Fu);
    return t0_id >= 1 && t0_id <= 9;
}

uint8_t get_type0_id(const uint8_t* data) {
    uint32_t dw0;
    std::memcpy(&dw0, data, sizeof(dw0));
    return static_cast<uint8_t>(dw0 & 0x3Fu);
}

uint8_t get_type1_id(const uint8_t* data) {
    uint32_t dw0;
    std::memcpy(&dw0, data, sizeof(dw0));
    return static_cast<uint8_t>((dw0 >> 24) & 0xFFu);
}

static std::string decode_type0_dispatch(const uint32_t* dwords) {
    uint8_t pkt_id = static_cast<uint8_t>(dwords[0] & 0x3Fu);
    switch (pkt_id) {
        case 1: return Type0KernelDispatch::decode(dwords).to_string();
        case 2: return Type0BarrierAndOr::decode(dwords).to_string();
        case 3: return Type0BindReleaseQueues::decode(dwords).to_string();
        case 4: return Type0Atomic::decode(dwords).to_string();
        case 5: return Type0SdmaCcl::decode(dwords).to_string();
        case 6: return Type0SdmaPtepde::decode(dwords).to_string();
        case 7: return Type0FlushInvCache::decode(dwords).to_string();
        case 8: return Type0SdmaCopyLinearRect::decode(dwords).to_string();
        case 9: return Type0UserTask::decode(dwords).to_string();
        default:
            throw std::runtime_error(std::string("Unknown Type0 PKT_ID: 0x") +
                [](uint8_t id){ char b[4]; snprintf(b,sizeof(b),"%02X",id); return std::string(b); }(pkt_id));
    }
}

static std::string decode_type1_dispatch(const uint32_t* dwords) {
    uint8_t pkt_id = static_cast<uint8_t>((dwords[0] >> 24) & 0xFFu);
    switch (pkt_id) {
        case 0x3F: return Type1Nop::decode(dwords).to_string();
        case 0x10: return Type1IndirectBuffer::decode(dwords).to_string();
        case 0x11: return Type1SetLoadReg::decode(dwords).to_string();
        case 0x12: return Type1CondExec::decode(dwords).to_string();
        case 0x13: return Type1MemSemaphore::decode(dwords).to_string();
        case 0x14: return Type1Sync::decode(dwords).to_string();
        case 0x15: return Type1EventCreate::decode(dwords).to_string();
        case 0x16: return Type1CopyData::decode(dwords).to_string();
        case 0x17: return Type1BarrierObj::decode(dwords).to_string();
        case 0x18: return Type1SdmaCopyLinear::decode(dwords).to_string();
        case 0x19: return Type1Fence::decode(dwords).to_string();
        case 0x1A: return Type1SdmaTrap::decode(dwords).to_string();
        case 0x1B: return Type1SdmaConstFill::decode(dwords).to_string();
        case 0x1C: return Type1Timestamp::decode(dwords).to_string();
        case 0x1D: return Type1RdmaWriteInline::decode(dwords).to_string();
        case 0x1E: return Type1HwCountersSetProfile::decode(dwords).to_string();
        case 0x1F: return Type1InvTlb::decode(dwords).to_string();
        default:
            throw std::runtime_error(std::string("Unknown Type1 PKT_ID: 0x") +
                [](uint8_t id){ char b[4]; snprintf(b,sizeof(b),"%02X",id); return std::string(b); }(pkt_id));
    }
}

std::string decode_type0(const uint8_t* data) {
    uint32_t dwords[16];
    std::memcpy(dwords, data, TYPE0_SIZE);
    return decode_type0_dispatch(dwords);
}

std::string decode_type1(const uint8_t* data) {
    uint32_t dwords[8];
    std::memcpy(dwords, data, TYPE1_SIZE);
    return decode_type1_dispatch(dwords);
}

std::string decode_packet(const uint8_t* data, size_t size) {
    if (is_type0_packet(data)) {
        if (size < TYPE0_SIZE)
            throw std::runtime_error("Buffer too small for Type0 packet (need 64 bytes)");
        return decode_type0(data);
    } else {
        if (size < TYPE1_SIZE)
            throw std::runtime_error("Buffer too small for Type1 packet (need 32 bytes)");
        return decode_type1(data);
    }
}

} // namespace pg
