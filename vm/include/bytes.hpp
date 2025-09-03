#pragma once

#include <stdint.h>

inline uint32_t load_int(uint8_t* bytes)
{
    uint32_t value = 0;
    for (int i = 0; i < 4; i++)
    {
        value = (value >> 8) | (static_cast<uint32_t>(*bytes) << 24);
        bytes++;
    }

    return value;
}

inline void write_int(uint8_t* bytes, uint32_t value)
{
    for (int i = 0; i < 4; i++)
    {
        *bytes = static_cast<uint8_t>(value & 0xFF);
        bytes++;
        value = value >> 8;
    }
}