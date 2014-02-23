/*  tripping-cyril
 *  Copyright (C) 2014  Toon Schoenmakers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * In this file you can disable certain features of the framework, simply uncomment
 * the define statements. Note that if these depend on a certain library you will
 * still have to modify the Makefile to not link against these libraries!
 */

#ifndef _DEFINES_H
#define _DEFINES_H

/**
 * Disables the GZipCompressor
 * @note You may get rid of the link against zlib
 * TODO Disable gzip decompression in the http client as well
 */
//#define _NO_GZIP

/**
 * Disables the LZMACompressor
 * @note You may get rid of the link against liblzma
 */
//#define _NO_LZMA

#endif //_DEFINES_H