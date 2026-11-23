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
#include "GXV44Options.h"
CGXV44Options::CGXV44Options() : DefaultCodewordSize(GX_V44_CODEWORD_SIZE_6), DefaultOrdinalSize(GX_V44_ORDINAL_SIZE_7), MaxCodewords(1024), MaximumStringLength(GX_V44_MAXIMUM_STRING_LENGTH_255), HistoryLimit(3072)
#ifdef COMPRESSION_TRACE
    , Hex(false)
#endif
{}
int CGXV44Options::Validate() const
{
    if (DefaultCodewordSize < GX_V44_CODEWORD_SIZE_6 || DefaultCodewordSize > GX_V44_CODEWORD_SIZE_16 || DefaultOrdinalSize < GX_V44_ORDINAL_SIZE_7 ||
        DefaultOrdinalSize > GX_V44_ORDINAL_SIZE_8 || MaxCodewords < 256 || MaxCodewords > 65535 ||
        HistoryLimit < 512 || HistoryLimit > 2147483647UL ||
        (MaximumStringLength != GX_V44_MAXIMUM_STRING_LENGTH_46 && MaximumStringLength != GX_V44_MAXIMUM_STRING_LENGTH_78 && MaximumStringLength != GX_V44_MAXIMUM_STRING_LENGTH_142 && MaximumStringLength != GX_V44_MAXIMUM_STRING_LENGTH_255))
    { return GX_COMPRESSION_INVALID_ARGUMENT; }
    return 0;
}
