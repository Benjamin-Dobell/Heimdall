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

#ifndef SETUPSESSIONPACKET_H
#define SETUPSESSIONPACKET_H

// Heimdall
#include "ControlPacket.h"

namespace Heimdall
{
	class SetupSessionPacket : public ControlPacket
	{
		public:

			enum
			{
				kBeginSession		= 0,
				kDeviceInfo			= 1,
				kTotalBytes			= 2
			};

		private:

			unsigned int request;
			unsigned int unknown3Parameter;

		public:

			SetupSessionPacket(unsigned int request, unsigned int unknown3Parameter = 0)
				: ControlPacket(ControlPacket::kControlTypeSetupSession)
			{
				this->request = request;
				this->unknown3Parameter = unknown3Parameter;
			}

			unsigned int GetRequest(void) const
			{
				return (request);
			}

			unsigned int GetUnknown3Parameter(void) const
			{
				return (unknown3Parameter);
			}

			void Pack(void)
			{
				ControlPacket::Pack();

				PackInteger(ControlPacket::kDataSize, request);
				PackInteger(ControlPacket::kDataSize + 4, unknown3Parameter);
			}
	};
}

#endif
