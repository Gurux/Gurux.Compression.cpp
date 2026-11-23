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
#ifndef GX_V44_NODE_H
#define GX_V44_NODE_H
#include "GXCompressionConfig.h"
#ifdef COMPRESSION_TRACE
#include <string>
#endif
/** @brief V.44 token state and optional diagnostic event identifiers. */
enum GX_V44_OPERATION
{
    GX_V44_NONE = -3, GX_V44_START = -2, GX_V44_END = -1,
    GX_V44_ORDINAL = 0, GX_V44_CODEWORD = 1, GX_V44_STRING_EXTENSION = 2,
    GX_V44_STEPUP_ORDINAL = 3, GX_V44_STEPUP_CODEWORD = 4,
    GX_V44_FLUSH = 5, GX_V44_REINIT = 6, GX_V44_ERROR = 7, GX_V44_PROCESSING = 8
};
/** @brief Detached dictionary entry; encoder Position is a segment start,
 * decoder Position is an inclusive string end. Links are codewords or zero. */
struct CGXV44Node
{
    unsigned int Codeword; /**< Dictionary codeword, starting at four. */
    unsigned long Position; /**< History position in bytes, as described above. */
    unsigned int Length; /**< Segment (encoder) or full string (decoder) length. */
    unsigned int DownIndex; /**< First child codeword, encoder only. */
    unsigned int SideIndex; /**< Next sibling codeword, encoder only. */
    GX_V44_OPERATION Type; /**< Token that created this node. */
#ifdef COMPRESSION_TRACE
    std::string Value; /**< Detached formatted bytes, empty after ClearDiagnostics. */
#endif
    /** @brief Initializes an unlinked dictionary entry. */
    CGXV44Node() : Codeword(0), Position(0), Length(0), DownIndex(0), SideIndex(0), Type(GX_V44_ORDINAL) {}
};
#endif
