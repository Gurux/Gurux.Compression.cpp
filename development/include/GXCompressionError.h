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
#ifndef GX_COMPRESSION_ERROR_H
#define GX_COMPRESSION_ERROR_H
/** @brief Return codes shared by buffer and codec APIs; zero denotes success. */
enum GX_COMPRESSION_ERROR
{
    GX_COMPRESSION_OK = 0, /**< Operation completed. */
    GX_COMPRESSION_INVALID_ARGUMENT = 1, /**< Invalid options, bounds, pointer or alias. */
    GX_COMPRESSION_OUTOFMEMORY = 2, /**< Allocation failed or size overflowed. */
    GX_COMPRESSION_INVALID_DATA = 3, /**< Malformed, truncated or unbounded wire input. */
    GX_COMPRESSION_IO_ERROR = 4, /**< File read or write failed. */
    GX_COMPRESSION_CANCELLED = 5 /**< Trace callback requested cancellation. */
};
#endif
