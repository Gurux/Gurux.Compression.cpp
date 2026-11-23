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
#ifndef GX_COMPRESSION_BYTE_BUFFER_H
#define GX_COMPRESSION_BYTE_BUFFER_H
#ifdef GX_COMPRESSION_EXTERNAL_BUFFER_HEADER
#include GX_COMPRESSION_EXTERNAL_BUFFER_HEADER
#else
#include "GXCompressionError.h"
/**
 * @brief Portable byte-storage subset of the supplied Gurux DLMS CGXByteBuffer.
 * @note Owns storage; not thread safe. Copies unread bytes, matching DLMS.
 * Numeric/protocol conversion methods are intentionally outside this subset.
 * Define GX_COMPRESSION_EXTERNAL_BUFFER_HEADER to use an application's full
 * DLMS class instead; compile without this project's GXByteBuffer.cpp.
 */
class CGXByteBuffer
{
    unsigned char* m_Data;
    unsigned long m_Capacity;
    unsigned long m_Size;
    unsigned long m_Position;
public:
    /** @brief Creates an empty buffer. */
    CGXByteBuffer();
    /** @brief Copies unread bytes; throws std::bad_alloc on allocation failure. */
    CGXByteBuffer(const CGXByteBuffer& value);
    /** @brief Releases owned storage. */
    ~CGXByteBuffer();
    /** @brief Copies unread bytes with strong allocation-failure safety. */
    CGXByteBuffer& operator=(const CGXByteBuffer& value);
    /** @brief Returns the readable byte count. */
    unsigned long Available();
    /** @brief Returns stored bytes, including consumed bytes. */
    unsigned long GetSize();
    /** @brief Sets size within capacity; invalid sizes leave the buffer unchanged. */
    void SetSize(unsigned long value);
    /** @brief Returns the read cursor in bytes. */
    unsigned long GetPosition();
    /** @brief Sets read cursor within size; invalid positions are ignored. */
    void SetPosition(unsigned long value);
    /** @brief Resizes storage; zero releases it. Returns a GX_COMPRESSION_ERROR. */
    int Capacity(unsigned long capacity);
    /** @brief Returns allocated bytes. */
    unsigned long Capacity();
    /** @brief Appends a byte; returns zero or an allocation error. */
    int SetUInt8(unsigned char value);
    /** @brief Writes an existing byte without advancing size; validates index. */
    int SetUInt8(unsigned long index, unsigned char value);
    /** @brief Appends count bytes; null is accepted only for count zero.
     * @note Source must not overlap this buffer's allocation. */
    int Set(const void* source, unsigned long count);
    /** @brief Reads one byte and advances the cursor only on success. */
    int GetUInt8(unsigned char* value);
    /** @brief Reads an indexed byte without advancing the cursor. */
    int GetUInt8(unsigned long index, unsigned char* value);
    /** @brief Returns storage, valid until the next mutation; null when empty. */
    unsigned char* GetData();
    /** @brief Releases storage and resets the cursor and size. */
    void Clear();
};
#endif
#endif
