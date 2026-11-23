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
#include "GXV44Decoder.h"
#include "GXV44Internal.h"
#include <algorithm>
#include <new>
#include <stdexcept>
/** @brief Propagates decoder failures without throwing protocol exceptions. */
#define GX_TRY(expression) do { int gxRet = (expression); if (gxRet != 0) { return gxRet; } } while (0)

CGXV44Decoder::CGXV44Decoder(const CGXV44Options& options) : m_Options(options),
    m_PreviousType(GX_V44_NONE), m_PreviousCodeword(0), m_PreviousOrdinal(0),
    m_OrdinalSize(options.DefaultOrdinalSize), m_CodewordSize(options.DefaultCodewordSize)
#ifdef COMPRESSION_TRACE
    , m_Trace(NULL), m_Context(NULL)
#endif
{}
void CGXV44Decoder::Reset()
{
    m_Strings.clear(); m_History.Clear(); m_PreviousType = GX_V44_NONE;
    m_PreviousCodeword = m_PreviousOrdinal = 0;
    m_OrdinalSize = m_Options.DefaultOrdinalSize; m_CodewordSize = m_Options.DefaultCodewordSize;
}
#ifdef COMPRESSION_TRACE
void CGXV44Decoder::SetTrace(GX_V44_TRACE_CALLBACK callback, void* context) { m_Trace = callback; m_Context = context; }
std::vector<CGXV44Node> CGXV44Decoder::GetStringCollection() const { return m_Strings; }
void CGXV44Decoder::ClearDiagnostics()
{
    for (size_t i = 0; i < m_Strings.size(); ++i) { std::string().swap(m_Strings[i].Value); }
}
#endif
unsigned int CGXV44Decoder::GetOrdinalSize() const { return m_OrdinalSize; }
unsigned int CGXV44Decoder::GetCodewordSize() const { return m_CodewordSize; }
#ifdef COMPRESSION_TRACE
int CGXV44Decoder::Notify(GX_V44_OPERATION operation, unsigned long index, unsigned int value, const std::string& bits)
{
    if (m_Trace == NULL) { return 0; }
    CGXV44TraceArgs args;
    args.Operation = operation; args.Index = index; args.Value = value; args.Compressed = bits;
    args.Dictionary = m_Strings;
    if (m_History.GetSize() != 0)
    { args.Uncompressed.assign(m_History.GetData(), m_History.GetData() + m_History.GetSize()); }
    m_Trace(args, m_Context);
    return args.Stop ? GX_COMPRESSION_CANCELLED : 0;
}
#endif
int CGXV44Decoder::Append(unsigned char value)
{
    if (m_History.GetSize() >= m_Options.HistoryLimit) { return GX_COMPRESSION_INVALID_DATA; }
    return m_History.SetUInt8(value) == 0 ? 0 : GX_COMPRESSION_OUTOFMEMORY;
}
void CGXV44Decoder::AddString(unsigned int length, unsigned long position)
{
    if (length > static_cast<unsigned int>(m_Options.MaximumStringLength) || 4 + m_Strings.size() >= m_Options.MaxCodewords) { return; }
    CGXV44Node node;
    node.Codeword = static_cast<unsigned int>(4 + m_Strings.size());
    node.Position = position; node.Length = length; node.Type = m_PreviousType;
#ifdef COMPRESSION_TRACE
    if (position < m_History.GetSize() && position + 1 >= length)
    { node.Value = GXDiagnostic(m_History.GetData() + position + 1 - length, length, m_Options.Hex); }
#endif
    m_Strings.push_back(node);
}
int CGXV44Decoder::Ordinal(unsigned int value)
{
    GX_TRY(Append(static_cast<unsigned char>(value)));
    if (m_PreviousType == GX_V44_ORDINAL) { AddString(2, m_History.GetSize() - 1); }
    else if (m_PreviousType == GX_V44_CODEWORD)
    { AddString(m_Strings[m_PreviousCodeword - 4].Length + 1, m_History.GetSize() - 1); }
    m_PreviousType = GX_V44_ORDINAL; m_PreviousOrdinal = value; return 0;
}
int CGXV44Decoder::Codeword(unsigned int value)
{
    unsigned int next = static_cast<unsigned int>(4 + m_Strings.size());
    if (value < 4 || value >= m_Options.MaxCodewords || value > next) { return GX_COMPRESSION_INVALID_DATA; }
    if (value == next && (m_PreviousType != GX_V44_ORDINAL && m_PreviousType != GX_V44_CODEWORD))
    { return GX_COMPRESSION_INVALID_DATA; }
    unsigned int previousLength = 0;
    if (m_PreviousType == GX_V44_CODEWORD)
    {
        previousLength = m_Strings[m_PreviousCodeword - 4].Length;
        if (value == next && previousLength >= static_cast<unsigned int>(m_Options.MaximumStringLength)) { return GX_COMPRESSION_INVALID_DATA; }
    }
    unsigned long outputStart = m_History.GetSize();
    if (value == next && m_PreviousType == GX_V44_ORDINAL)
    { GX_TRY(Append(static_cast<unsigned char>(m_PreviousOrdinal))); GX_TRY(Append(static_cast<unsigned char>(m_PreviousOrdinal))); }
    else
    {
        const CGXV44Node entry = m_Strings[(value == next ? m_PreviousCodeword : value) - 4];
        if (entry.Position + 1 < entry.Length) { return GX_COMPRESSION_INVALID_DATA; }
        unsigned long start = entry.Position + 1 - entry.Length;
        unsigned int length = entry.Length;
        const unsigned char first = m_History.GetData()[start];
        for (unsigned int i = 0; i < length; ++i)
        {
            if (start + i >= m_History.GetSize()) { return GX_COMPRESSION_INVALID_DATA; }
            GX_TRY(Append(m_History.GetData()[start + i]));
        }
        // The special next entry is previous-string + its first byte, not
        // the byte following the old string in history (which may differ).
        if (value == next) { GX_TRY(Append(first)); }
    }
    if (m_PreviousType == GX_V44_ORDINAL) { AddString(2, outputStart); }
    else if (m_PreviousType == GX_V44_CODEWORD) { AddString(previousLength + 1, outputStart); }
    m_PreviousType = GX_V44_CODEWORD; m_PreviousCodeword = value; return 0;
}
int CGXV44Decoder::Extension(unsigned int length)
{
    if (m_PreviousType != GX_V44_CODEWORD || m_PreviousCodeword < 4 ||
        m_PreviousCodeword - 4 >= m_Strings.size()) { return GX_COMPRESSION_INVALID_DATA; }
    const CGXV44Node entry = m_Strings[m_PreviousCodeword - 4];
    if (entry.Length + length > static_cast<unsigned int>(m_Options.MaximumStringLength)) { return GX_COMPRESSION_INVALID_DATA; }
    for (unsigned int i = 0; i < length; ++i)
    {
        unsigned long source = entry.Position + 1 + i;
        if (source >= m_History.GetSize()) { return GX_COMPRESSION_INVALID_DATA; }
        GX_TRY(Append(m_History.GetData()[source]));
    }
    AddString(entry.Length + length, m_History.GetSize() - 1);
    m_PreviousType = GX_V44_STRING_EXTENSION; return 0;
}

/** @brief Writes a completed history slice without closing the destination. */
static int GXWriteHistory(CGXByteBuffer& history, unsigned long start, CGXByteBuffer* output, FILE* file)
{
    unsigned long count = history.GetSize() - start;
    if (count == 0) { return 0; }
    if (output != NULL)
    { return output->Set(history.GetData() + start, count) == 0 ? 0 : GX_COMPRESSION_OUTOFMEMORY; }
    return std::fwrite(history.GetData() + start, 1, static_cast<size_t>(count), file) == count ? 0 : GX_COMPRESSION_IO_ERROR;
}
int CGXV44Decoder::Decode(CGXByteBuffer* input, FILE* inputFile, CGXByteBuffer* output, FILE* outputFile)
{
    Reset();
    CGXV44Reader reader(input, inputFile);
#ifdef COMPRESSION_TRACE
    std::string traceBits;
    if (m_Trace != NULL && input != NULL)
    {
        for (unsigned long i = input->GetPosition(); i < input->GetSize(); ++i)
        {
            for (unsigned int b = 0; b < 8; ++b) { traceBits += ((input->GetData()[i] >> b) & 1) ? '1' : '0'; }
        }
    }
    GX_TRY(Notify(GX_V44_START, 0, 0, traceBits));
#endif
    bool afterCodeword = false, flushed = false;
    unsigned long written = 0;
    unsigned int peek = 0;
    int ret = 0;
    while (reader.Peek(peek))
    {
        flushed = false;
        unsigned int prefix = reader.Read(1);
        if (afterCodeword && prefix == 0)
        {
            unsigned int second = reader.Read(1);
            if (reader.Error != 0) { break; }
            if (second == 1)
            {
                unsigned int extension = GXReadExtension(reader, m_Options.MaximumStringLength);
                if (reader.Error != 0) { break; }
                ret = Extension(extension);
                if (ret != 0) { break; }
#ifdef COMPRESSION_TRACE
                ret = Notify(GX_V44_STRING_EXTENSION, reader.Position, extension, traceBits);
                if (ret != 0) { break; }
#endif
                afterCodeword = false; continue;
            }
        }
        afterCodeword = false;
        if (prefix == 0)
        {
            unsigned int value = reader.Read(m_OrdinalSize);
            if (reader.Error != 0) { break; }
            ret = Ordinal(value);
            if (ret != 0) { break; }
#ifdef COMPRESSION_TRACE
            ret = Notify(GX_V44_ORDINAL, reader.Position, value, traceBits);
            if (ret != 0) { break; }
#endif
            continue;
        }
        unsigned int code = reader.Read(m_CodewordSize);
        if (reader.Error != 0) { break; }
#ifdef COMPRESSION_TRACE
        GX_V44_OPERATION operation = GX_V44_CODEWORD;
#endif
        if (code == 0) { ret = GX_COMPRESSION_INVALID_DATA; break; }
        if (code == 1)
        {
            reader.Align();
            if (reader.Error != 0) { break; }
            ret = GXWriteHistory(m_History, written, output, outputFile);
            if (ret != 0) { break; }
            written = m_History.GetSize(); flushed = true;
#ifdef COMPRESSION_TRACE
            operation = GX_V44_FLUSH;
#endif
        }
        else if (code == 2)
        {
            if (!reader.Peek(peek)) { ret = reader.Error != 0 ? reader.Error : GX_COMPRESSION_INVALID_DATA; break; }
            if (peek == 0)
            {
                if (m_OrdinalSize == GX_V44_ORDINAL_SIZE_8) { ret = GX_COMPRESSION_INVALID_DATA; break; }
                ++m_OrdinalSize;
#ifdef COMPRESSION_TRACE
                operation = GX_V44_STEPUP_ORDINAL;
#endif
            }
            else
            {
                unsigned int maxBits = 0;
                for (unsigned int n = m_Options.MaxCodewords - 1; n != 0; n >>= 1) { ++maxBits; }
                if (m_CodewordSize >= std::max(static_cast<unsigned int>(m_Options.DefaultCodewordSize), maxBits))
                { ret = GX_COMPRESSION_INVALID_DATA; break; }
                ++m_CodewordSize;
#ifdef COMPRESSION_TRACE
                operation = GX_V44_STEPUP_CODEWORD;
#endif
            }
        }
        else if (code == 3)
        {
            ret = GXWriteHistory(m_History, written, output, outputFile);
            if (ret != 0) { break; }
            written = 0; Reset();
#ifdef COMPRESSION_TRACE
            operation = GX_V44_REINIT;
#endif
        }
        else
        {
            ret = Codeword(code);
            if (ret != 0) { break; }
            afterCodeword = true;
        }
#ifdef COMPRESSION_TRACE
        ret = Notify(operation, reader.Position, code, traceBits);
        if (ret != 0) { break; }
#endif
    }
    if (ret == 0 && reader.Error != 0) { ret = reader.Error; }
    if (ret == 0 && !flushed) { ret = GX_COMPRESSION_INVALID_DATA; }
#ifdef COMPRESSION_TRACE
    if (ret == GX_COMPRESSION_CANCELLED) { return ret; }
    if (ret != 0)
    {
        int traceRet = Notify(GX_V44_ERROR, reader.Position, static_cast<unsigned int>(ret), traceBits);
        return traceRet != 0 ? traceRet : ret;
    }
    return Notify(GX_V44_END, reader.Position, 0, traceBits);
#else
    return ret;
#endif
}
int CGXV44Decoder::Decompress(CGXByteBuffer& input, CGXByteBuffer& output)
{
    if (&input == &output || m_Options.Validate() != 0) { return GX_COMPRESSION_INVALID_ARGUMENT; }
    output.Clear();
    try { return Decode(&input, NULL, &output, NULL); }
    catch (const std::bad_alloc&) { return GX_COMPRESSION_OUTOFMEMORY; }
    catch (const std::length_error&) { return GX_COMPRESSION_OUTOFMEMORY; }
}
int CGXV44Decoder::Decompress(FILE* input, FILE* output)
{
    if (input == NULL || output == NULL || input == output || m_Options.Validate() != 0)
    { return GX_COMPRESSION_INVALID_ARGUMENT; }
    try { return Decode(NULL, input, NULL, output); }
    catch (const std::bad_alloc&) { return GX_COMPRESSION_OUTOFMEMORY; }
    catch (const std::length_error&) { return GX_COMPRESSION_OUTOFMEMORY; }
}
