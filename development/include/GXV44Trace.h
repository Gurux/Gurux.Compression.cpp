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
#ifndef GX_V44_TRACE_H
#define GX_V44_TRACE_H
#include "GXCompressionConfig.h"
#ifdef COMPRESSION_TRACE
#include "GXV44Node.h"
#include <string>
#include <vector>
/** @brief Detached event data. Callbacks may copy it, but must not reenter the codec. */
struct CGXV44TraceArgs
{
    bool Compress; /**< True for encoder events. */
    GX_V44_OPERATION Operation; /**< Current operation. */
    unsigned long Index; /**< Input byte index or consumed compressed-bit index. */
    unsigned int Value; /**< Ordinal, codeword, extension length or error code. */
    std::string Compressed; /**< LSB-first bits; empty for streaming decoders. */
    std::vector<unsigned char> Uncompressed; /**< Detached current input/history. */
    std::vector<CGXV44Node> Dictionary; /**< Detached current dictionary. */
    bool Stop; /**< Set true in callback to return GX_COMPRESSION_CANCELLED. */
    /** @brief Initializes a non-cancelling event. */
    CGXV44TraceArgs() : Compress(false), Operation(GX_V44_NONE), Index(0), Value(0), Stop(false) {}
};
/** @brief Callback valid for the operation; context remains caller-owned.
 * @note Callbacks must not throw exceptions or call the same codec recursively. */
typedef void (*GX_V44_TRACE_CALLBACK)(CGXV44TraceArgs& args, void* context);
#endif // COMPRESSION_TRACE
#endif
