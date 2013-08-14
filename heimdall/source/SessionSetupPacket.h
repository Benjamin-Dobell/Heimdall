/* Copyright (c) 2010-2013 Benjamin Dobell, Glass Echidna
 
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

#ifndef SESSIONSETUPPACKET_H
#define SESSIONSETUPPACKET_H

// Heimdall
#include "ControlPacket.h"

namespace Heimdall
{
	class SessionSetupPacket : public ControlPacket
	{
		public:

			enum
			{
				kBeginSession = 0,
				kDeviceType = 1, // ?
				kTotalBytes = 2,
				//kEnableSomeSortOfFlag = 3,
				kFilePartSize = 5
			};

		private:

			unsigned int request;

		protected:

			enum
			{
				kDataSize = ControlPacket::kDataSize + 4
			};

		public:

			SessionSetupPacket(unsigned int request) : ControlPacket(ControlPacket::kControlTypeSession)
			{
				this->request = request;
			}

			unsigned int GetRequest(void) const
			{
				return (request);
			}

			void Pack(void)
			{
				ControlPacket::Pack();

				PackInteger(ControlPacket::kDataSize, request);
			}
	};
}

#endif
