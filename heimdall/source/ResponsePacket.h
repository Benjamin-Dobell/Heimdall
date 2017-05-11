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

#ifndef RESPONSEPACKET_H
#define RESPONSEPACKET_H

// Heimdall
#include "InboundPacket.h"

namespace Heimdall
{
	class ResponsePacket : public InboundPacket
	{
		public:

			enum
			{
				kResponseTypeSendFilePart = 0x00,
				kResponseTypeSessionSetup = 0x64,
				kResponseTypePitFile = 0x65,
				kResponseTypeFileTransfer = 0x66,
				kResponseTypeEndSession = 0x67
			};

		private:

			unsigned int responseType;

		protected:

			enum
			{
				kDataSize = 4
			};

		public:

			ResponsePacket(int responseType) : InboundPacket(8)
			{
				this->responseType = responseType;
			}

			unsigned int GetResponseType(void) const
			{
				return (responseType);
			}

			virtual bool Unpack(void)
			{
				unsigned int receivedResponseType = UnpackInteger(0);
				if (receivedResponseType != responseType)
				{
					responseType = receivedResponseType;
					return (false);
				}

				return (true);
			}
	};
}

#endif
