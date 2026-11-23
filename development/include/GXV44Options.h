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
#ifndef GX_V44_OPTIONS_H
#define GX_V44_OPTIONS_H
#include "GXCompressionError.h"
#include "GXCompressionConfig.h"
/** @brief Negotiated maximum V.44 string length, in bytes. */
enum GX_V44_MAXIMUM_STRING_LENGTH
{
    GX_V44_MAXIMUM_STRING_LENGTH_46 = 46, /**< Maximum 46 bytes. */
    GX_V44_MAXIMUM_STRING_LENGTH_78 = 78, /**< Maximum 78 bytes. */
    GX_V44_MAXIMUM_STRING_LENGTH_142 = 142, /**< Maximum 142 bytes. */
    GX_V44_MAXIMUM_STRING_LENGTH_255 = 255 /**< Maximum 255 bytes; default. */
};

/** @brief Initial codeword width, in bits, before STEPUP controls. */
enum GX_V44_CODEWORD_SIZE
{
    GX_V44_CODEWORD_SIZE_6 = 6, /**< Six bits; default. */
    GX_V44_CODEWORD_SIZE_7 = 7, /**< Seven bits. */
    GX_V44_CODEWORD_SIZE_8 = 8, /**< Eight bits. */
    GX_V44_CODEWORD_SIZE_9 = 9, /**< Nine bits. */
    GX_V44_CODEWORD_SIZE_10 = 10, /**< Ten bits. */
    GX_V44_CODEWORD_SIZE_11 = 11, /**< Eleven bits. */
    GX_V44_CODEWORD_SIZE_12 = 12, /**< Twelve bits. */
    GX_V44_CODEWORD_SIZE_13 = 13, /**< Thirteen bits. */
    GX_V44_CODEWORD_SIZE_14 = 14, /**< Fourteen bits. */
    GX_V44_CODEWORD_SIZE_15 = 15, /**< Fifteen bits. */
    GX_V44_CODEWORD_SIZE_16 = 16 /**< Sixteen bits. */
};

/** @brief Initial ordinal width, in bits, before STEPUP controls. */
enum GX_V44_ORDINAL_SIZE
{
    GX_V44_ORDINAL_SIZE_7 = 7, /**< Seven-bit ordinals; default. */
    GX_V44_ORDINAL_SIZE_8 = 8 /**< Eight-bit binary ordinals. */
};

/** @brief Negotiated V.44 limits; codecs retain an independent value copy. */
struct CGXV44Options
{
    GX_V44_CODEWORD_SIZE DefaultCodewordSize; /**< Initial codeword width, 6..16. */
    GX_V44_ORDINAL_SIZE DefaultOrdinalSize; /**< Initial ordinal width, 7..8. */
    unsigned int MaxCodewords; /**< Dictionary limit including four controls, 256..65535. */
    GX_V44_MAXIMUM_STRING_LENGTH MaximumStringLength; /**< Supported values: 46, 78, 142, 255 bytes. */
    unsigned long HistoryLimit; /**< Maximum bytes between REINIT controls, 512..2147483647. */
#ifdef COMPRESSION_TRACE
    bool Hex; /**< Format diagnostic values as hexadecimal when true. */
#endif
    /** @brief Initializes the same defaults as Gurux.Compression.Net. */
    CGXV44Options();
    /** @brief Returns zero or GX_COMPRESSION_INVALID_ARGUMENT. */
    int Validate() const;
};
#endif
