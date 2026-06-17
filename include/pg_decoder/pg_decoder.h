#pragma once

#include "pg_decoder/pg_type0.h"
#include "pg_decoder/pg_type1.h"
#include <cstdint>
#include <string>

namespace pg {

constexpr size_t TYPE0_SIZE = 64;
constexpr size_t TYPE1_SIZE = 32;

bool is_type0_packet(const uint8_t* data);
uint8_t get_type0_id(const uint8_t* data);
uint8_t get_type1_id(const uint8_t* data);

std::string decode_type0(const uint8_t* data);
std::string decode_type1(const uint8_t* data);
std::string decode_packet(const uint8_t* data, size_t size);

} // namespace pg
