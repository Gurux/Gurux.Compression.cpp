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
#ifndef GX_V44_ENCODER_H
#define GX_V44_ENCODER_H
#include "GXV44Options.h"
#include "GXV44Node.h"
#ifdef COMPRESSION_TRACE
#include "GXV44Trace.h"
#endif
#include <vector>
#include "GXByteBuffer.h"
#include <cstdio>
#include <utility>
class CGXBitArray;
/** @brief V.44 encoder compatible with Gurux.Compression.Net.
 * @note Not thread safe. Each call starts a new stream. Trace callbacks are
 * synchronous and must not reenter the instance. No file ownership is taken.
 */
class CGXV44Encoder
{
    CGXV44Options m_Options;
    std::vector<CGXV44Node> m_Nodes;
    unsigned int m_Roots[256];
    unsigned int m_OrdinalSize, m_CodewordSize;
#ifdef COMPRESSION_TRACE
    GX_V44_TRACE_CALLBACK m_Trace;
    void* m_Context;
#endif
    /** @brief Encodes a block with local history positions and optional FLUSH. */
    int EncodeBlock(const unsigned char* input, unsigned long count, bool flush, CGXBitArray& bits);
#ifdef COMPRESSION_TRACE
    /** @brief Publishes a detached event when a callback is installed. */
    int Notify(GX_V44_OPERATION op, unsigned long index, unsigned int value,
        const unsigned char* input, unsigned long count, CGXBitArray& bits);
#endif
    /** @brief Appends a child or root sibling link without retaining vector references. */
    void Link(unsigned int parent, unsigned int root, unsigned int codeword);
public:
    /** @brief Copies options; invalid options are rejected by Compress. */
    explicit CGXV44Encoder(const CGXV44Options& options = CGXV44Options());
    /** @brief Resets dictionary and bit widths, preserving options and callback. */
    void Reset();
#ifdef COMPRESSION_TRACE
    /** @brief Installs an optional callback and caller-owned context. */
    void SetTrace(GX_V44_TRACE_CALLBACK callback, void* context = NULL);
    /** @brief Returns a detached diagnostic tree. */
    std::vector<CGXV44Node> GetNodeTree() const;
    /** @brief Returns root-byte/codeword pairs for linked roots. */
    std::vector<std::pair<unsigned char, unsigned int> > GetRootArray() const;
    /** @brief Releases formatted diagnostics without changing dictionary metadata. */
    void ClearDiagnostics();
#endif
    /** @brief Returns current ordinal width in bits. */
    unsigned int GetOrdinalSize() const;
    /** @brief Returns current codeword width in bits. */
    unsigned int GetCodewordSize() const;
    /** @brief Compresses unread bytes, preserving input position and replacing output.
     * @return GX_COMPRESSION_ERROR; input and output must be different objects.
     * @note Output can be partial on error. Reuse after failure starts cleanly. */
    int Compress(CGXByteBuffer& input, CGXByteBuffer& output);
    /** @brief Compresses binary files using at most min(HistoryLimit,65536) input bytes.
     * @param input Readable binary file, nonnull and distinct from output.
     * @param output Writable binary file. Neither file is closed or flushed.
     * @return GX_COMPRESSION_ERROR; output may be partial on failure.
     * @note Blocking I/O without timeout. Trace Stop cancels at token boundaries. */
    int Compress(FILE* input, FILE* output);
};
#endif
