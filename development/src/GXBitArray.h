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
#ifndef GX_BIT_ARRAY_H
#define GX_BIT_ARRAY_H
#include "GXByteBuffer.h"
#include "GXCompressionError.h"
#include <string>
/** @brief Owning LSB-first bit buffer; byte padding is always zero. Not thread safe. */
class CGXBitArray
{
    CGXByteBuffer m_Data;
    unsigned long m_Size;
public:
    /** @brief Creates an empty bit buffer. */
    CGXBitArray();
    /** @brief Clears bits and storage. */
    void Clear();
    /** @brief Returns valid bit count excluding byte padding. */
    unsigned long GetSize() const;
    /** @brief Returns underlying bytes; caller must not change them. */
    CGXByteBuffer& GetBytes();
    /** @brief Appends the low count bits of value; count must be 1..32. */
    int Add(unsigned long value, unsigned int count);
    /** @brief Reads count bits at index without a cursor; validates bounds. */
    int Get(unsigned long index, unsigned int count, unsigned long& value);
    /** @brief Returns valid bits in transmission order for diagnostic use. */
    std::string ToString();
};
#endif
