/* Copyright (c) 2010 Benjamin Dobell, Glass Echidna
 
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

#ifndef BRIDGEMANAGER_H
#define BRIDGEMANAGER_H

// Heimdall
#include "Heimdall.h"

struct libusb_context;
struct libusb_device;
struct libusb_device_handle;

namespace Heimdall
{
	class InboundPacket;
	class OutboundPacket;

	class DeviceIdentifier
	{
		public:

			const int vendorId;
			const int productId;

			DeviceIdentifier(int vid, int pid) :
				vendorId(vid),
				productId(pid)
			{
			}
	};

	class BridgeManager
	{
		public:

			enum
			{
				kSupportedDeviceCount		= 3,

				kCommunicationDelayDefault	= 0,
				kDumpBufferSize				= 4096
			};

			enum
			{
				kVidSamsung	= 0x04E8
			};

			enum
			{
				kPidGalaxyS		    = 0x6601,
				kPidGalaxyS2        = 0x685D,
				kPidDroidCharge     = 0x68C3
			};

		private:

			static const DeviceIdentifier supportedDevices[kSupportedDeviceCount];

			bool verbose;

			libusb_context *libusbContext;
			libusb_device_handle *deviceHandle;
			libusb_device *heimdallDevice;
			int interfaceIndex;
			int inEndpoint;
			int outEndpoint;

			int communicationDelay;

#ifdef OS_LINUX

			bool detachedDriver;

#endif

		public:

			BridgeManager(bool verbose, int communicationDelay);
			~BridgeManager();

			bool Initialise(void);

			bool BeginSession(void) const;
			bool EndSession(void) const;

			bool SendPacket(OutboundPacket *packet, int timeout = 3000) const;
			bool ReceivePacket(InboundPacket *packet, int timeout = 3000) const;

			bool SendPitFile(FILE *file) const;
			int ReceivePitFile(unsigned char **pitBuffer) const;

			bool SendFile(FILE *file, int destination, int fileIdentifier = -1) const;
			bool ReceiveDump(int chipType, int chipId, FILE *file) const;

			bool RebootDevice(void) const;

			bool IsVerbose(void) const
			{
				return (verbose);
			}
	};
}

#endif
