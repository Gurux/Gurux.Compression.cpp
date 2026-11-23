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
#include "GXBitArray.h"
#include <climits>
CGXBitArray::CGXBitArray() : m_Size(0) {}
void CGXBitArray::Clear() { m_Data.Clear(); m_Size = 0; }
unsigned long CGXBitArray::GetSize() const { return m_Size; }
CGXByteBuffer& CGXBitArray::GetBytes() { return m_Data; }
int CGXBitArray::Add(unsigned long value, unsigned int count)
{
    if (count == 0 || count > 32) { return GX_COMPRESSION_INVALID_ARGUMENT; }
    if (m_Size > ULONG_MAX - count) { return GX_COMPRESSION_OUTOFMEMORY; }
    for (unsigned int i = 0; i < count; ++i)
    {
        if ((m_Size & 7) == 0)
        {
            int ret = m_Data.SetUInt8(0);
            if (ret != 0)
            {
                Clear();
                return GX_COMPRESSION_OUTOFMEMORY;
            }
        }
        m_Data.GetData()[m_Size / 8] |= static_cast<unsigned char>(((value >> i) & 1UL) << (m_Size & 7));
        ++m_Size;
    }
    return 0;
}
int CGXBitArray::Get(unsigned long index, unsigned int count, unsigned long& value)
{
    if (count == 0 || count > 32 || index > m_Size || count > m_Size - index)
    { return GX_COMPRESSION_INVALID_ARGUMENT; }
    value = 0;
    for (unsigned int i = 0; i < count; ++i)
    {
        unsigned long pos = index + i;
        value |= static_cast<unsigned long>((m_Data.GetData()[pos / 8] >> (pos & 7)) & 1) << i;
    }
    return 0;
}
std::string CGXBitArray::ToString()
{
    std::string result;
    for (unsigned long i = 0; i < m_Size; ++i)
    { result += ((m_Data.GetData()[i / 8] >> (i & 7)) & 1) ? '1' : '0'; }
    return result;
}
