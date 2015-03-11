/* Copyright (c) 2010-2014 Benjamin Dobell, Glass Echidna

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

// Heimdall Frontend
#import "GZipFile.h"

using namespace HeimdallFrontend;

GZipFile::GZipFile(const QString& path) :
	file(path),
	gzFile(nullptr),
	temporary(false)
{
}

GZipFile::~GZipFile()
{
	Close();

	if (temporary)
	{
		file.remove();
	}
}

bool GZipFile::Open(Mode mode)
{
	if (!file.isOpen() && !file.open(mode == GZipFile::ReadOnly ? QFile::ReadOnly : QFile::WriteOnly))
	{
		return (false);
	}

	gzFile = gzdopen(file.handle(), mode == GZipFile::ReadOnly ? "rb" : "wb");
	return (gzFile != nullptr);
}

void GZipFile::Close()
{
	file.close();
	gzclose(gzFile);
}

int GZipFile::Read(void *buffer, int length)
{
	return (length >= 0 && !file.isWritable() ? gzread(gzFile, buffer, length) : -1);
}

bool GZipFile::Write(void *buffer, int length)
{
	return (length >= 0 && gzwrite(gzFile, buffer, length) == length);
}

qint64 GZipFile::Offset() const
{
	return gzoffset(gzFile);
}
