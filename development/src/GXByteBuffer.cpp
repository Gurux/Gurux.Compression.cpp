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
#include "GXByteBuffer.h"
#include <cstdlib>
#include <cstring>
#include <climits>
#include <new>
#include <cstddef>
#include <limits>
CGXByteBuffer::CGXByteBuffer() : m_Data(NULL), m_Capacity(0), m_Size(0), m_Position(0) {}
CGXByteBuffer::CGXByteBuffer(const CGXByteBuffer& value) : m_Data(NULL), m_Capacity(0), m_Size(0), m_Position(0)
{
    if (value.m_Size != value.m_Position && Set(value.m_Data + value.m_Position, value.m_Size - value.m_Position) != 0)
    {
        throw std::bad_alloc();
    }
}
CGXByteBuffer::~CGXByteBuffer() { Clear(); }
CGXByteBuffer& CGXByteBuffer::operator=(const CGXByteBuffer& value)
{
    if (this != &value)
    {
        CGXByteBuffer copy(value);
        unsigned char* data = m_Data; m_Data = copy.m_Data; copy.m_Data = data;
        unsigned long size = m_Size; m_Size = copy.m_Size; copy.m_Size = size;
        size = m_Capacity; m_Capacity = copy.m_Capacity; copy.m_Capacity = size;
        m_Position = 0;
    }
    return *this;
}
unsigned long CGXByteBuffer::Available() { return m_Size - m_Position; }
unsigned long CGXByteBuffer::GetSize() { return m_Size; }
void CGXByteBuffer::SetSize(unsigned long value)
{
    if (value <= m_Capacity)
    {
        m_Size = value;
        if (m_Position > value) { m_Position = value; }
    }
}
unsigned long CGXByteBuffer::GetPosition() { return m_Position; }
void CGXByteBuffer::SetPosition(unsigned long value)
{
    if (value <= m_Size) { m_Position = value; }
}
int CGXByteBuffer::Capacity(unsigned long capacity)
{
    if (capacity == 0) { Clear(); return 0; }
    if (capacity > ULONG_MAX / 2 || capacity > static_cast<unsigned long>((std::numeric_limits<std::ptrdiff_t>::max)())) { return GX_COMPRESSION_OUTOFMEMORY; }
    unsigned char* data = static_cast<unsigned char*>(std::realloc(m_Data, static_cast<size_t>(capacity)));
    if (data == NULL) { return GX_COMPRESSION_OUTOFMEMORY; }
    m_Data = data; m_Capacity = capacity;
    if (m_Size > capacity) { m_Size = capacity; }
    if (m_Position > m_Size) { m_Position = m_Size; }
    return 0;
}
unsigned long CGXByteBuffer::Capacity() { return m_Capacity; }
int CGXByteBuffer::SetUInt8(unsigned char value) { return Set(&value, 1); }
int CGXByteBuffer::SetUInt8(unsigned long index, unsigned char value)
{
    if (index >= m_Size) { return GX_COMPRESSION_INVALID_ARGUMENT; }
    m_Data[index] = value; return 0;
}
int CGXByteBuffer::Set(const void* source, unsigned long count)
{
    if (count == 0) { return 0; }
    if (source == NULL) { return GX_COMPRESSION_INVALID_ARGUMENT; }
    if (count > ULONG_MAX - m_Size) { return GX_COMPRESSION_OUTOFMEMORY; }
    unsigned long required = m_Size + count;
    if (required > m_Capacity)
    {
        unsigned long capacity = m_Capacity <= ULONG_MAX / 2 ? m_Capacity * 2 : ULONG_MAX;
        if (capacity < 64) { capacity = 64; }
        if (capacity < required) { capacity = required; }
        int ret = Capacity(capacity);
        if (ret != 0) { return ret; }
    }
    std::memcpy(m_Data + m_Size, source, static_cast<size_t>(count));
    m_Size = required; return 0;
}
int CGXByteBuffer::GetUInt8(unsigned char* value)
{
    int ret = GetUInt8(m_Position, value);
    if (ret == 0) { ++m_Position; }
    return ret;
}
int CGXByteBuffer::GetUInt8(unsigned long index, unsigned char* value)
{
    if (value == NULL || index >= m_Size) { return GX_COMPRESSION_INVALID_ARGUMENT; }
    *value = m_Data[index]; return 0;
}
unsigned char* CGXByteBuffer::GetData() { return m_Data; }
void CGXByteBuffer::Clear()
{
    std::free(m_Data); m_Data = NULL; m_Capacity = m_Size = m_Position = 0;
}
