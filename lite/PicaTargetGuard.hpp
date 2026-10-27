// SPDX-License-Identifier: MIT
#pragma once
#include "GxQueuePlan.hpp"
#include "PicaStateAudit.hpp"
#include <cstdint>
namespace PicaTargetGuard {
enum class Failure : uint8_t {
 None, NotTopTransfer, MissingRegisters, MismatchedColor,
 MismatchedDepth, BadDimensions, BadColorFormat, BadColorMask
};
struct Result {
 bool matches{};
 Failure failure{Failure::NotTopTransfer};
 uint32_t color_physical{};
 uint32_t depth_physical{};
};
Result MatchTop(const GxQueuePlan::Decision& pending,
                const PicaStateAudit::Shadow& gpu);
// This guard NEVER submits graphics and is insufficient to prove live GPU
// injection safe. It only asserts that the original game target is correct.
} // namespace PicaTargetGuard
