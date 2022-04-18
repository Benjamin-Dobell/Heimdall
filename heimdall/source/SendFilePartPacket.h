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

#ifndef SENDFILEPARTPACKET_H
#define SENDFILEPARTPACKET_H

// C Standard Library
#include <stdio.h>
#include <string.h>

// Heimdall
#include "Packet.h"

namespace Heimdall
{
	class SendFilePartPacket : public OutboundPacket
	{
		public:

			SendFilePartPacket(FILE *file, unsigned long size) : OutboundPacket(size)
			{
				memset(data, 0, size);

				unsigned long position = (unsigned long)FileTell(file);

				FileSeek(file, 0, SEEK_END);
				unsigned long fileSize = (unsigned long)FileTell(file);
				FileSeek(file, position, SEEK_SET);

				// min(fileSize, size)
				unsigned long bytesToRead = (fileSize < size) ? fileSize - position : size;
				(void)fread(data, 1, bytesToRead, file);
			}

			SendFilePartPacket(unsigned char *buffer, unsigned int size) : OutboundPacket(size)
			{
				memcpy(data, buffer, size);
			}

			void Pack(void)
			{
			}
	};
}

#endif
