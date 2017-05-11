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

#ifndef ENDFILETRANSFERPACKET_H
#define ENDFILETRANSFERPACKET_H

// Heimdall
#include "FileTransferPacket.h"

namespace Heimdall
{
	class EndFileTransferPacket : public FileTransferPacket
	{
		public:

			enum
			{
				kDestinationPhone	= 0x00,
				kDestinationModem	= 0x01
			};

		protected:

			enum
			{
				kDataSize = FileTransferPacket::kDataSize + 16
			};

		private:

			unsigned int destination;			// PDA / Modem
			unsigned int sequenceByteCount;
			unsigned int unknown1; // EFS?
			unsigned int deviceType;

		protected:

			EndFileTransferPacket(unsigned int destination, unsigned int sequenceByteCount, unsigned int unknown1, unsigned int deviceType)
				: FileTransferPacket(FileTransferPacket::kRequestEnd)
			{
				this->destination = destination;
				this->sequenceByteCount = sequenceByteCount;
				this->unknown1 = unknown1;
				this->deviceType = deviceType;
			}

		public:

			unsigned int GetDestination(void) const
			{
				return (destination);
			}

			unsigned int GetSequenceByteCount(void) const
			{
				return (sequenceByteCount);
			}

			unsigned int GetUnknown1(void) const
			{
				return (unknown1);
			}

			unsigned int GetDeviceType(void) const
			{
				return (deviceType);
			}

			virtual void Pack(void)
			{
				FileTransferPacket::Pack();

				PackInteger(FileTransferPacket::kDataSize, destination);
				PackInteger(FileTransferPacket::kDataSize + 4, sequenceByteCount);
				PackInteger(FileTransferPacket::kDataSize + 8, unknown1);
				PackInteger(FileTransferPacket::kDataSize + 12, deviceType);
			}
	};
}

#endif
