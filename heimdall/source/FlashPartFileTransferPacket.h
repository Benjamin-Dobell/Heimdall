/* Copyright (c) 2010-2012 Benjamin Dobell, Glass Echidna
 
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

#ifndef FLASHPARTFILETRANSFERPACKET_H
#define FLASHPARTFILETRANSFERPACKET_H

// Heimdall
#include "FileTransferPacket.h"

namespace Heimdall
{
	class FlashPartFileTransferPacket : public FileTransferPacket
	{
		private:

			unsigned short unknown;
			unsigned int transferCount;

		public:

			FlashPartFileTransferPacket(unsigned short unknown, unsigned int transferCount)
				: FileTransferPacket(FileTransferPacket::kRequestPart)
			{
				this->unknown = unknown;
				this->transferCount = transferCount;
			}

			unsigned short GetUnknown(void) const
			{
				return (unknown);
			}

			unsigned int GetTransferCount(void) const
			{
				return (transferCount);
			}

			void Pack(void)
			{
				FileTransferPacket::Pack();

				PackShort(FileTransferPacket::kDataSize, unknown);
				PackInteger(FileTransferPacket::kDataSize + 2, transferCount);
			}
	};
}

#endif
