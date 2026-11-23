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
#ifndef GX_V44_DECODER_H
#define GX_V44_DECODER_H
#include "GXV44Options.h"
#include "GXV44Node.h"
#ifdef COMPRESSION_TRACE
#include "GXV44Trace.h"
#endif
#include <vector>
#include "GXByteBuffer.h"
#include <cstdio>
/** @brief Strict V.44 decoder with history bounded by negotiated options.
 * @note Instances are not thread safe. Calls start new streams. No ownership
 * of files or callback context is taken. Callbacks must not reenter the codec. */
class CGXV44Decoder
{
    CGXV44Options m_Options;
    std::vector<CGXV44Node> m_Strings;
    CGXByteBuffer m_History;
    GX_V44_OPERATION m_PreviousType;
    unsigned int m_PreviousCodeword, m_PreviousOrdinal;
    unsigned int m_OrdinalSize, m_CodewordSize;
#ifdef COMPRESSION_TRACE
    GX_V44_TRACE_CALLBACK m_Trace;
    void* m_Context;
#endif
    /** @brief Clears the dictionary and current history window. */
    void Reset();
    /** @brief Adds a history byte, checking the negotiated limit. */
    int Append(unsigned char value);
    /** @brief Creates a dictionary entry ending at position when capacity permits. */
    void AddString(unsigned int length, unsigned long position);
    /** @brief Handles an ordinal and pending dictionary creation. */
    int Ordinal(unsigned int value);
    /** @brief Handles an existing or special next dictionary codeword. */
    int Codeword(unsigned int value);
    /** @brief Copies a string extension from its preceding history location. */
    int Extension(unsigned int length);
#ifdef COMPRESSION_TRACE
    /** @brief Publishes detached history and dictionary snapshots. */
    int Notify(GX_V44_OPERATION operation, unsigned long index, unsigned int value, const std::string& bits);
#endif
    /** @brief Shared buffer/file parser. Exactly one input and one output are active. */
    int Decode(CGXByteBuffer* input, FILE* inputFile, CGXByteBuffer* output, FILE* outputFile);
public:
    /** @brief Copies options; invalid options are rejected by Decompress. */
    explicit CGXV44Decoder(const CGXV44Options& options = CGXV44Options());
#ifdef COMPRESSION_TRACE
    /** @brief Installs an optional synchronous callback and caller-owned context. */
    void SetTrace(GX_V44_TRACE_CALLBACK callback, void* context = NULL);
    /** @brief Returns a detached decoder dictionary. */
    std::vector<CGXV44Node> GetStringCollection() const;
    /** @brief Releases formatted values, retaining dictionary metadata. */
    void ClearDiagnostics();
#endif
    /** @brief Returns current ordinal width in bits. */
    unsigned int GetOrdinalSize() const;
    /** @brief Returns current codeword width in bits. */
    unsigned int GetCodewordSize() const;
    /** @brief Decompresses unread input, preserving its position and replacing output.
     * @return GX_COMPRESSION_ERROR. Aliasing is rejected; output may be partial.
     * @note Requires terminal FLUSH and zero byte padding. */
    int Decompress(CGXByteBuffer& input, CGXByteBuffer& output);
    /** @brief Decompresses blocking binary files, leaving both files open.
     * @param input Readable nonnull file distinct from output.
     * @param output Writable nonnull file; completed history blocks may be written
     * before malformed input is detected. The file is not flushed or closed.
     * @return GX_COMPRESSION_ERROR, including cancellation and I/O failure.
     * @note Memory is bounded by HistoryLimit and MaxCodewords; no timeout. */
    int Decompress(FILE* input, FILE* output);
};
#endif
