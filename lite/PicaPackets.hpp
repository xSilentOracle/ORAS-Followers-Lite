// SPDX-License-Identifier: MIT
// Portable PICA200 command-list encoder. No hardware access.
#pragma once
#include <cstddef>
#include <cstdint>

namespace PicaPackets {
constexpr uint16_t kFinalize = 0x0010u;
constexpr uint32_t Header(uint16_t reg, uint8_t mask, uint8_t extra,
                          bool incremental) {
    return uint32_t(reg & 0x3FFu) |
           (uint32_t(mask & 0xFu) << 16) |
           (uint32_t(extra) << 20) |
           (incremental ? 0x80000000u : 0u);
}
struct PacketInfo {
    uint16_t reg{};
    uint8_t mask{};
    uint16_t parameters{};
    bool incremental{};
    bool valid{};
};
PacketInfo DecodeHeader(uint32_t word);
// Unlike the libctru global GPUCMD buffer, this writer is an independent,
// bounded object. It rejects all overruns and never calls svcBreak().
class Writer {
public:
    Writer(uint32_t* output, size_t capacity_words);
    bool Write(uint16_t reg, uint32_t value, uint8_t byte_mask=0xFu);
    bool WriteMany(uint16_t reg, const uint32_t* values, size_t count,
                   bool incremental, uint8_t mask=0xFu);
    bool Finalize(); // GPUREG_FINALIZE and 16-byte list alignment
    const uint32_t* Data() const { return data_; }
    size_t SizeWords() const { return size_; }
    size_t SizeBytes() const { return size_ * sizeof(uint32_t); }
    bool Failed() const { return failed_; }
private:
    bool WriteChunk(uint16_t reg,const uint32_t* values,size_t count,
                    bool incremental,uint8_t mask);
    uint32_t* data_{};
    size_t capacity_{};
    size_t size_{};
    bool failed_{};
    bool finalized_{};
};
} // namespace PicaPackets
