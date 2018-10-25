#pragma once

#include <cstdint>

namespace efc {
namespace naf {
namespace Http {

/**
 * Convert an int based enum to an int.
 */
template <typename T> uint32_t enumToInt(T val) { return static_cast<uint32_t>(val); }

} // namespace Http
} // namespace naf
} // namespace efc
