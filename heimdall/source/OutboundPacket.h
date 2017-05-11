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

#ifndef OUTBOUNDPACKET_H
#define OUTBOUNDPACKET_H

// Heimdall
#include "Packet.h"

namespace Heimdall
{
	class OutboundPacket : public Packet
	{
		protected:

			void PackInteger(unsigned int offset, unsigned int value)
			{
#ifdef WORDS_BIGENDIAN
				data[offset] = (value & 0xFF000000) >> 24;
				data[offset + 1] = (value & 0x00FF0000) >> 16;
				data[offset + 2] = (value & 0x0000FF00) >> 8;
				data[offset + 3] = value & 0x000000FF;
#else
				// Flip endianness
				data[offset] = value & 0x000000FF;
				data[offset + 1] = (value & 0x0000FF00) >> 8;
				data[offset + 2] = (value & 0x00FF0000) >> 16;
				data[offset + 3] = (value & 0xFF000000) >> 24;
#endif
			}

			void PackShort(unsigned int offset, unsigned short value)
			{
#ifdef WORDS_BIGENDIAN
				data[offset] = (value & 0xFF00) >> 8;
				data[offset + 1] = value & 0x00FF;
#else
				// Flip endianness
				data[offset] = value & 0x00FF;
				data[offset + 1] = (value & 0xFF00) >> 8;
#endif
			}

		public:

			OutboundPacket(unsigned int size) : Packet(size)
			{
			}

			virtual void Pack(void) = 0;
	};
}

#endif
