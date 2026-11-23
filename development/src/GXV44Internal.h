// --------------------------------------------------------------------------
// Gurux Ltd
//
// Copyright (c) Gurux Ltd.
//
// This file is part of Gurux Device Framework.
//
// Commit: $Format:%H$
// Date:   $Format:%cI$
//
// This software is available under either of the following licenses:
//
// 1. GNU General Public License, version 2 only (GPL-2.0-only).
//    See the LICENSE file included with this distribution.
//
// 2. A commercial license granted by Gurux Ltd under a separate written
//    agreement. Use under this option requires a valid commercial license
//    and is governed by the terms of that agreement.
//
// If you have not obtained a commercial license from Gurux Ltd,
// use of this software is governed by GPL-2.0-only.
//
// Commercial licensing: https://www.gurux.fi/
//
// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-Gurux-Commercial
// --------------------------------------------------------------------------
#ifndef GX_V44_INTERNAL_H
#define GX_V44_INTERNAL_H
#include "GXBitArray.h"
#include "GXV44Options.h"
#include <cstdio>
#include <string>
/** @brief Internal LSB-first reader with one-byte lookahead and sticky errors. */
class CGXV44Reader
{
    CGXByteBuffer* m_Input;
    FILE* m_File;
    unsigned long m_Offset;
    unsigned long m_Bits;
    unsigned int m_Available;
public:
    unsigned long Position; /**< Consumed bits; wraps only on extremely large files. */
    int Error; /**< First I/O or malformed-input error. */
    /** @brief Uses exactly one source; caller retains ownership. */
    CGXV44Reader(CGXByteBuffer* input, FILE* file) : m_Input(input), m_File(file),
        m_Offset(input == NULL ? 0 : input->GetPosition()), m_Bits(0), m_Available(0), Position(0), Error(0) {}
    /** @brief Peeks one bit; false denotes EOF or a sticky read error. */
    bool Peek(unsigned int& bit)
    {
        if (m_Available == 0)
        {
            int next;
            if (m_Input != NULL)
            {
                if (m_Offset == m_Input->GetSize()) { return false; }
                next = m_Input->GetData()[m_Offset++];
            }
            else
            {
                next = std::fgetc(m_File);
                if (next == EOF)
                {
                    if (std::ferror(m_File)) { Error = GX_COMPRESSION_IO_ERROR; }
                    return false;
                }
            }
            m_Bits = static_cast<unsigned int>(next); m_Available = 8;
        }
        bit = static_cast<unsigned int>(m_Bits & 1UL); return true;
    }
    /** @brief Reads up to sixteen bits; truncated tokens set InvalidData. */
    unsigned int Read(unsigned int count)
    {
        unsigned int result = 0, bit = 0;
        for (unsigned int i = 0; i < count; ++i)
        {
            if (!Peek(bit))
            {
                if (Error == 0) { Error = GX_COMPRESSION_INVALID_DATA; }
                return 0;
            }
            result |= bit << i; m_Bits >>= 1; --m_Available; ++Position;
        }
        return result;
    }
    /** @brief Validates zero padding through the current byte boundary. */
    void Align()
    {
        unsigned int padding = static_cast<unsigned int>((8 - Position % 8) % 8);
        if (padding != 0 && Read(padding) != 0) { Error = GX_COMPRESSION_INVALID_DATA; }
    }
};
/** @brief Returns the negotiated long-extension bit width. */
inline unsigned int GXExtensionWidth(GX_V44_MAXIMUM_STRING_LENGTH maximum)
{
    if (maximum == GX_V44_MAXIMUM_STRING_LENGTH_46) { return 5; }
    if (maximum == GX_V44_MAXIMUM_STRING_LENGTH_78) { return 6; }
    if (maximum == GX_V44_MAXIMUM_STRING_LENGTH_142) { return 7; }
    return 8;
}
/** @brief Encodes an extension length using Table 6/V.44. */
inline int GXWriteExtension(CGXBitArray& bits, unsigned int value, GX_V44_MAXIMUM_STRING_LENGTH maximum)
{
    if (value == 1) { return bits.Add(1, 1); }
    if (value <= 4) { return bits.Add((value - 1) << 1, 3); }
    if (value <= 12) { return bits.Add((value - 5) << 4, 7); }
    int ret = bits.Add(8, 4);
    if (ret != 0) { return ret; }
    return bits.Add(value - 13, GXExtensionWidth(maximum));
}
/** @brief Decodes a bounded extension length, rejecting reserved values. */
inline unsigned int GXReadExtension(CGXV44Reader& reader, GX_V44_MAXIMUM_STRING_LENGTH maximum)
{
    if (reader.Read(1) == 1) { return 1; }
    unsigned int value = reader.Read(2);
    if (value != 0) { return value + 1; }
    if (reader.Read(1) == 0) { return reader.Read(3) + 5; }
    value = reader.Read(GXExtensionWidth(maximum)) + 13;
    if (value > static_cast<unsigned int>(maximum) - 2U) { reader.Error = GX_COMPRESSION_INVALID_DATA; }
    return value;
}
#ifdef COMPRESSION_TRACE
/** @brief Formats bytes without retaining references to source storage. */
inline std::string GXDiagnostic(const unsigned char* data, unsigned long count, bool hex)
{
    std::string value;
    const char* digits = "0123456789ABCDEF";
    for (unsigned long i = 0; i < count; ++i)
    {
        if (hex || data[i] < 32 || data[i] > 126)
        { value += digits[data[i] >> 4]; value += digits[data[i] & 15]; }
        else { value += static_cast<char>(data[i]); }
    }
    return value;
}
#endif // COMPRESSION_TRACE
#endif
