/* Copyright (c) 2010-2014 Benjamin Dobell, Glass Echidna
 
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

// libpit
#include "libpit.h"

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
				kSupportedDeviceCount = 3
			};

			enum
			{
				kInitialiseSucceeded = 0,
				kInitialiseFailed,
				kInitialiseDeviceNotDetected
			};

			enum
			{
				kVidSamsung	= 0x04E8
			};

			enum
			{
				kPidGalaxyS = 0x6601,
				kPidGalaxyS2 = 0x685D,
				kPidDroidCharge = 0x68C3
			};

			enum
			{
				kDefaultTimeoutSend = 6000,
				kDefaultTimeoutReceive = 6000,
				kDefaultTimeoutEmptyTransfer = 100
			};

			enum class UsbLogLevel
			{
				None = 0,
				Error,
				Warning,
				Info,
				Debug,

				Default = Error
			};

			enum
			{
				kEmptyTransferNone = 0,
				kEmptyTransferBefore = 1,
				kEmptyTransferAfter = 1 << 1,
				kEmptyTransferBeforeAndAfter = kEmptyTransferBefore | kEmptyTransferAfter
			};

		private:

			static const DeviceIdentifier supportedDevices[kSupportedDeviceCount];

			bool verbose;

			libusb_context *libusbContext;
			libusb_device_handle *deviceHandle;
			libusb_device *heimdallDevice;

			int interfaceIndex;
			int altSettingIndex;
			int inEndpoint;
			int outEndpoint;

			bool interfaceClaimed;

#ifdef OS_LINUX

			bool detachedDriver;

#endif

			unsigned int fileTransferSequenceMaxLength;
			unsigned int fileTransferPacketSize;
			unsigned int fileTransferSequenceTimeout;

			UsbLogLevel usbLogLevel;

			int FindDeviceInterface(void);
			bool ClaimDeviceInterface(void);
			bool SetupDeviceInterface(void);
			void ReleaseDeviceInterface(void);

			bool InitialiseProtocol(void);

			bool SendBulkTransfer(unsigned char *data, int length, int timeout, bool retry = true) const;
			int ReceiveBulkTransfer(unsigned char *data, int length, int timeout, bool retry = true) const;

		public:

			BridgeManager(bool verbose);
			~BridgeManager();

			bool DetectDevice(void);
			int Initialise(bool resume);

			bool BeginSession(void);
			bool EndSession(bool reboot) const;

			bool SendPacket(OutboundPacket *packet, int timeout = kDefaultTimeoutSend, int emptyTransferFlags = kEmptyTransferAfter) const;
			bool ReceivePacket(InboundPacket *packet, int timeout = kDefaultTimeoutReceive, int emptyTransferFlags = kEmptyTransferNone) const;

			bool RequestDeviceType(unsigned int request, int *result) const;

			bool SendPitData(const libpit::PitData *pitData) const;
			int ReceivePitFile(unsigned char **pitBuffer) const;
			int DownloadPitFile(unsigned char **pitBuffer) const; // Thin wrapper around ReceivePitFile() with additional logging.

			bool SendFile(FILE *file, unsigned int destination, unsigned int deviceType, unsigned int fileIdentifier = 0xFFFFFFFF) const;

			void SetUsbLogLevel(UsbLogLevel usbLogLevel);

			UsbLogLevel GetUsbLogLevel(void) const
			{
				return usbLogLevel;
			}

			bool IsVerbose(void) const
			{
				return (verbose);
			}
	};
}

#endif
