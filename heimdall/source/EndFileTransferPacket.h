/* Copyright (c) 2010-2011 Benjamin Dobell, Glass Echidna
 
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

			unsigned int destination;			// Chip identifier perhaps
			unsigned short partialPacketLength;	// Length or (length - 65536) if lastFullPacket is odd.
			unsigned int lastFullPacketIndex;	
			unsigned short unknown1;
			unsigned int partitionType;

		protected:

			EndFileTransferPacket(unsigned int destination, unsigned int partialPacketLength, unsigned int lastFullPacketIndex,
				unsigned short unknown1, unsigned int partitionType)
				: FileTransferPacket(FileTransferPacket::kRequestEnd)
			{
				this->destination = destination;

				if (partialPacketLength > 65535)
				{
					this->partialPacketLength = (partialPacketLength - 65536);
					this->lastFullPacketIndex = lastFullPacketIndex + 1;
				}
				else
				{
					this->partialPacketLength = partialPacketLength;
					this->lastFullPacketIndex = lastFullPacketIndex;
				}
				
				this->unknown1 = unknown1;
				this->partitionType = partitionType;
			}

		public:

			unsigned int GetDestination(void) const
			{
				return (destination);
			}

			unsigned int GetPartialPacketLength(void) const
			{
				if (lastFullPacketIndex % 2 == 0)
					return (partialPacketLength);
				else
					return (partialPacketLength + 65536);
			}

			unsigned int GetLastFullPacketIndex(void) const
			{
				return (lastFullPacketIndex - lastFullPacketIndex % 2);
			}

			unsigned short GetUnknown1(void) const
			{
				return (unknown1);
			}

			unsigned int GetPartitionType(void) const
			{
				return (partitionType);
			}

			virtual void Pack(void)
			{
				FileTransferPacket::Pack();

				PackInteger(FileTransferPacket::kDataSize, destination);
				PackShort(FileTransferPacket::kDataSize + 4, partialPacketLength);
				PackInteger(FileTransferPacket::kDataSize + 6, lastFullPacketIndex);
				PackShort(FileTransferPacket::kDataSize + 10, unknown1);
				PackInteger(FileTransferPacket::kDataSize + 12, partitionType);
			}
	};
}

#endif
