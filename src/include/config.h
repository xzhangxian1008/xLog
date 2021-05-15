#pragma once

#include <stdint.h>

namespace xLog {

constexpr uint32_t BUFFER_SIZE = 10 * 1024 * 1024; // 10MB every buffer
constexpr uint32_t LOCAL_MEM_SIZE = 500;

// Log format example: [DEBUG] /home/xzx/main.cpp: 345: ...content...
// ' ' and ':' are extra content.
constexpr uint32_t EXTRA_SIZE = 5;

} // namespace xLog
