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
#include "GXV44Encoder.h"
#include "GXV44Internal.h"
#include <algorithm>
#include <cstring>
#include <new>
#include <stdexcept>

/** @brief Propagates the first error from bounded bit and buffer operations. */
#define GX_TRY(expression) do { int gxRet = (expression); if (gxRet != 0) { return gxRet; } } while (0)
/** @brief Removes trace expressions entirely from builds without tracing. */
#ifdef COMPRESSION_TRACE
#define GX_TRACE(expression) GX_TRY(expression)
#else
#define GX_TRACE(expression) do {} while (0)
#endif

CGXV44Encoder::CGXV44Encoder(const CGXV44Options& options) : m_Options(options),
    m_OrdinalSize(options.DefaultOrdinalSize), m_CodewordSize(options.DefaultCodewordSize)
#ifdef COMPRESSION_TRACE
    , m_Trace(NULL), m_Context(NULL)
#endif
{ Reset(); }
void CGXV44Encoder::Reset()
{
    std::memset(m_Roots, 0, sizeof(m_Roots)); m_Nodes.clear();
    m_OrdinalSize = m_Options.DefaultOrdinalSize; m_CodewordSize = m_Options.DefaultCodewordSize;
}
#ifdef COMPRESSION_TRACE
void CGXV44Encoder::SetTrace(GX_V44_TRACE_CALLBACK callback, void* context) { m_Trace = callback; m_Context = context; }
std::vector<CGXV44Node> CGXV44Encoder::GetNodeTree() const { return m_Nodes; }
std::vector<std::pair<unsigned char, unsigned int> > CGXV44Encoder::GetRootArray() const
{
    std::vector<std::pair<unsigned char, unsigned int> > result;
    for (unsigned int i = 0; i < 256; ++i)
    {
        if (m_Roots[i] != 0) { result.push_back(std::make_pair(static_cast<unsigned char>(i), m_Roots[i])); }
    }
    return result;
}
void CGXV44Encoder::ClearDiagnostics()
{
    for (size_t i = 0; i < m_Nodes.size(); ++i) { std::string().swap(m_Nodes[i].Value); }
}
#endif
unsigned int CGXV44Encoder::GetOrdinalSize() const { return m_OrdinalSize; }
unsigned int CGXV44Encoder::GetCodewordSize() const { return m_CodewordSize; }
void CGXV44Encoder::Link(unsigned int parent, unsigned int root, unsigned int codeword)
{
    unsigned int* link = parent == 0 ? &m_Roots[root] : &m_Nodes[parent - 4].DownIndex;
    while (*link != 0) { link = &m_Nodes[*link - 4].SideIndex; }
    *link = codeword;
}
#ifdef COMPRESSION_TRACE
int CGXV44Encoder::Notify(GX_V44_OPERATION op, unsigned long index, unsigned int value,
    const unsigned char* input, unsigned long count, CGXBitArray& bits)
{
    if (m_Trace == NULL) { return 0; }
    CGXV44TraceArgs args;
    args.Compress = true; args.Operation = op; args.Index = index; args.Value = value;
    args.Compressed = bits.ToString(); args.Dictionary = m_Nodes;
    if (count != 0) { args.Uncompressed.assign(input, input + count); }
    m_Trace(args, m_Context);
    return args.Stop ? GX_COMPRESSION_CANCELLED : 0;
}
#endif
int CGXV44Encoder::EncodeBlock(const unsigned char* input, unsigned long count, bool flush, CGXBitArray& bits)
{
    bits.Clear(); Reset();
    unsigned long history = 0, end = std::min(count, m_Options.HistoryLimit);
    GX_V44_OPERATION previous = GX_V44_ORDINAL;
    GX_TRACE(Notify(GX_V44_START, 0, 0, input, count, bits));
    unsigned long i = 0;
    while (i < count)
    {
        if (i == end || 4 + m_Nodes.size() >= m_Options.MaxCodewords)
        {
            GX_TRY(bits.Add(1, 1)); GX_TRY(bits.Add(3, m_CodewordSize));
            Reset(); history = i; end = i + std::min(m_Options.HistoryLimit, count - i);
            previous = GX_V44_NONE;
            GX_TRACE(Notify(GX_V44_REINIT, i, 0, input, count, bits));
        }
        unsigned char ch = input[i]; ++history;
        int best = -1;
        unsigned int matchedLength = 0, tail = 0;
        unsigned int code = m_Roots[ch];
        unsigned long cursor = i + 1;
        while (code != 0)
        {
            bool found = false;
            while (code != 0)
            {
                const CGXV44Node& node = m_Nodes[code - 4];
                if (1 + tail + node.Length <= static_cast<unsigned int>(m_Options.MaximumStringLength) &&
                    node.Length <= end - cursor && node.Position <= history && node.Length <= history - node.Position &&
                    std::memcmp(input + cursor, input + node.Position, node.Length) == 0)
                { found = true; break; }
                code = node.SideIndex;
            }
            if (!found) { break; }
            best = static_cast<int>(code - 4);
            tail += m_Nodes[best].Length; matchedLength = 1 + tail;
            cursor += m_Nodes[best].Length; code = m_Nodes[best].DownIndex;
        }
        CGXV44Node node;
        node.Codeword = static_cast<unsigned int>(4 + m_Nodes.size());
        if (best < 0)
        {
            if (ch > 127 && m_OrdinalSize == GX_V44_ORDINAL_SIZE_7)
            {
                GX_TRY(bits.Add(1, 1)); GX_TRY(bits.Add(2, m_CodewordSize)); ++m_OrdinalSize;
                GX_TRACE(Notify(GX_V44_STEPUP_ORDINAL, i, 0, input, count, bits));
            }
            else if (previous == GX_V44_CODEWORD) { GX_TRY(bits.Add(0, 1)); }
            GX_TRY(bits.Add(0, 1)); GX_TRY(bits.Add(ch, m_OrdinalSize));
            node.Position = i + 1; node.Length = 1;
#ifdef COMPRESSION_TRACE
            node.Value = GXDiagnostic(input + i, 1, m_Options.Hex);
#endif
            if (i + 1 < count) { Link(0, ch, node.Codeword); }
            m_Nodes.push_back(node); previous = GX_V44_ORDINAL; ++i;
            GX_TRACE(Notify(GX_V44_ORDINAL, i, ch, input, count, bits));
            continue;
        }
        history += matchedLength - 1;
        const CGXV44Node matched = m_Nodes[best];
        unsigned int extension = 0;
        unsigned long extensionSource = matched.Position + matched.Length;
        while (extension < static_cast<unsigned int>(m_Options.MaximumStringLength) - matchedLength &&
            i + matchedLength + extension < end && extensionSource + extension < history &&
            input[i + matchedLength + extension] == input[extensionSource + extension])
        { ++history; ++extension; }
        GX_TRY(bits.Add(1, 1));
        while (matched.Codeword >= (1UL << m_CodewordSize))
        {
            GX_TRY(bits.Add(2, m_CodewordSize)); ++m_CodewordSize; GX_TRY(bits.Add(1, 1));
            GX_TRACE(Notify(GX_V44_STEPUP_CODEWORD, i, 0, input, count, bits));
        }
        GX_TRY(bits.Add(matched.Codeword, m_CodewordSize));
        node.Position = i + matchedLength;
        node.Length = extension == 0 ? 1 : extension;
        node.Type = extension == 0 ? GX_V44_CODEWORD : GX_V44_STRING_EXTENSION;
#ifdef COMPRESSION_TRACE
        node.Value = GXDiagnostic(input + i, matchedLength + extension, m_Options.Hex);
#endif
        if (extension != 0 || matchedLength < static_cast<unsigned int>(m_Options.MaximumStringLength))
        {
            Link(matched.Codeword, ch, node.Codeword); m_Nodes.push_back(node);
        }
        if (extension != 0)
        {
            GX_TRY(bits.Add(2, 2)); GX_TRY(GXWriteExtension(bits, extension, m_Options.MaximumStringLength));
        }
        previous = node.Type; i += matchedLength + extension;
        GX_TRACE(Notify(previous, i, matched.Codeword, input, count, bits));
        GX_TRACE(Notify(GX_V44_PROCESSING, i, matched.Codeword, input, count, bits));
    }
    if (flush)
    {
        GX_TRY(bits.Add(1, 1)); GX_TRY(bits.Add(1, m_CodewordSize));
        GX_TRACE(Notify(GX_V44_FLUSH, count, 0, input, count, bits));
    }
    GX_TRACE(Notify(GX_V44_END, count, 0, input, count, bits));
    return 0;
}
int CGXV44Encoder::Compress(CGXByteBuffer& input, CGXByteBuffer& output)
{
    if (&input == &output || m_Options.Validate() != 0) { return GX_COMPRESSION_INVALID_ARGUMENT; }
    output.Clear();
    try
    {
        CGXBitArray bits;
        const unsigned char* data = input.Available() == 0 ? NULL : input.GetData() + input.GetPosition();
        GX_TRY(EncodeBlock(data, input.Available(), true, bits));
        return output.Set(bits.GetBytes().GetData(), bits.GetBytes().GetSize()) == 0 ? 0 : GX_COMPRESSION_OUTOFMEMORY;
    }
    catch (const std::bad_alloc&) { return GX_COMPRESSION_OUTOFMEMORY; }
    catch (const std::length_error&) { return GX_COMPRESSION_OUTOFMEMORY; }
}
int CGXV44Encoder::Compress(FILE* input, FILE* output)
{
    if (input == NULL || output == NULL || input == output || m_Options.Validate() != 0)
    { return GX_COMPRESSION_INVALID_ARGUMENT; }
    try
    {
        const unsigned long capacity = std::min(m_Options.HistoryLimit, 65536UL);
        CGXByteBuffer block;
        if (block.Capacity(capacity) != 0) { return GX_COMPRESSION_OUTOFMEMORY; }
        unsigned int partial = 0, available = 0;
        int pending = EOF;
        do
        {
            unsigned long count = 0;
            if (pending != EOF) { block.GetData()[count++] = static_cast<unsigned char>(pending); }
            while (count < capacity)
            {
                size_t read = std::fread(block.GetData() + count, 1, static_cast<size_t>(capacity - count), input);
                if (std::ferror(input)) { return GX_COMPRESSION_IO_ERROR; }
                count += static_cast<unsigned long>(read);
                if (read == 0) { break; }
            }
            pending = std::fgetc(input);
            if (pending == EOF && std::ferror(input)) { return GX_COMPRESSION_IO_ERROR; }
            CGXBitArray bits;
            GX_TRY(EncodeBlock(block.GetData(), count, pending == EOF, bits));
            if (pending != EOF) { GX_TRY(bits.Add(1, 1)); GX_TRY(bits.Add(3, m_CodewordSize)); }
            for (unsigned long i = 0; i < bits.GetSize(); ++i)
            {
                unsigned long bit = 0; GX_TRY(bits.Get(i, 1, bit));
                partial |= static_cast<unsigned int>(bit) << available;
                if (++available == 8)
                {
                    if (std::fputc(static_cast<int>(partial), output) == EOF) { return GX_COMPRESSION_IO_ERROR; }
                    partial = available = 0;
                }
            }
        } while (pending != EOF);
        if (available != 0 && std::fputc(static_cast<int>(partial), output) == EOF) { return GX_COMPRESSION_IO_ERROR; }
        return 0;
    }
    catch (const std::bad_alloc&) { return GX_COMPRESSION_OUTOFMEMORY; }
    catch (const std::length_error&) { return GX_COMPRESSION_OUTOFMEMORY; }
}
