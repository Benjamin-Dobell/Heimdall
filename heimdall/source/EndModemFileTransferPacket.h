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

#ifndef ENDMODEMFILETRANSFERPACKET_H
#define ENDMODEMFILETRANSFERPACKET_H

// Heimdall
#include "EndFileTransferPacket.h"

namespace Heimdall
{
	class EndModemFileTransferPacket : public EndFileTransferPacket
	{
		private:

			unsigned int endOfFile;

		public:

			EndModemFileTransferPacket(unsigned int sequenceByteCount, unsigned int unknown1, unsigned int chipIdentifier, bool endOfFile)
				: EndFileTransferPacket(EndFileTransferPacket::kDestinationModem, sequenceByteCount, unknown1, chipIdentifier)
			{
				this->endOfFile = (endOfFile) ? 1 : 0;
			}

			bool IsEndOfFile(void) const
			{
				return (endOfFile == 1);
			}

			void Pack(void)
			{
				EndFileTransferPacket::Pack();

				PackInteger(EndFileTransferPacket::kDataSize, endOfFile);
			}
	};
}

#endif
