/* Copyright (c) 2010-2017 Benjamin Dobell, Glass Echidna
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.*/

#ifndef HEIMDALL_H
#define HEIMDALL_H

#ifdef _MSC_VER // Microsoft Visual C Standard Library

#include <Windows.h>
#undef GetBinaryType

#ifndef va_copy
#define va_copy(d, s) ((d) = (s))
#endif

#define FileOpen(FILE, MODE) fopen(FILE, MODE)
#define FileClose(FILE) fclose(FILE)
#define FileSeek(FILE, OFFSET, ORIGIN) _fseeki64(FILE, OFFSET, ORIGIN)
#define FileTell(FILE) _ftelli64(FILE)
#define FileRewind(FILE) rewind(FILE)

#else // POSIX Standard Library

#ifdef AUTOCONF
#include "../config.h"
#endif

#include <unistd.h>

#define Sleep(t) usleep(1000*t)

#define FileOpen(FILE, MODE) fopen(FILE, MODE)
#define FileClose(FILE) fclose(FILE)
#define FileSeek(FILE, OFFSET, ORIGIN) fseeko(FILE, OFFSET, ORIGIN)
#define FileTell(FILE) ftello(FILE)
#define FileRewind(FILE) rewind(FILE)

#endif

#if (!(defined _MSC_VER) || (_MSC_VER < 1700))

#ifndef nullptr
#define nullptr 0
#endif

#endif

#endif
