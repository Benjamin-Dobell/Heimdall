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

#ifndef ENDPHONEFILETRANSFERPACKET_H
#define ENDPHONEFILETRANSFERPACKET_H

// Heimdall
#include "EndFileTransferPacket.h"

namespace Heimdall
{
	class EndPhoneFileTransferPacket : public EndFileTransferPacket
	{
		public:

			/*enum
			{
				kFilePrimaryBootloader			= 0x00,
				kFilePit						= 0x01, // New 1.1 - Don't flash the pit this way!
				kFileSecondaryBootloader		= 0x03,
				kFileSecondaryBootloaderBackup	= 0x04,	// New 1.1
				kFileKernel						= 0x06,
				kFileRecovery					= 0x07,	// New 1.1

				kFileTabletModem				= 0x08, // New 1.2

				kFileEfs						= 0x14, // New 1.1
				kFileParamLfs					= 0x15,
				kFileFactoryFilesystem			= 0x16,
				kFileDatabaseData				= 0x17,
				kFileCache						= 0x18,

				kFileModem						= 0x0B	// New 1.1 - Kies flashes the modem this way rather than using the EndModemFileTransferPacket.
			};*/

		private:

			unsigned int fileIdentifier;
			unsigned int endOfFile;

		public:

			EndPhoneFileTransferPacket(unsigned int sequenceByteCount, unsigned int unknown1, unsigned int chipIdentifier,
				unsigned int fileIdentifier, bool endOfFile)
				: EndFileTransferPacket(EndFileTransferPacket::kDestinationPhone, sequenceByteCount, unknown1, chipIdentifier)
			{
				this->fileIdentifier = fileIdentifier;
				this->endOfFile = (endOfFile) ? 1 : 0;
			}

			unsigned int GetFileIdentifier(void)
			{
				return (fileIdentifier);
			}

			bool IsEndOfFile(void) const
			{
				return (endOfFile == 1);
			}

			void Pack(void)
			{
				EndFileTransferPacket::Pack();

				PackInteger(EndFileTransferPacket::kDataSize, fileIdentifier);
				PackInteger(EndFileTransferPacket::kDataSize + 4, endOfFile);
			}
	};
}

#endif
