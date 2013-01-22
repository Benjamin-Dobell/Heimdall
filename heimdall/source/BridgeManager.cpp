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

// C Standard Library
#include <stdio.h>

// libusb
#include <libusb.h>

// Heimdall
#include "BeginDumpPacket.h"
#include "BeginSessionPacket.h"
#include "BridgeManager.h"
#include "DeviceTypePacket.h"
#include "DumpPartFileTransferPacket.h"
#include "DumpPartPitFilePacket.h"
#include "DumpResponse.h"
#include "EndModemFileTransferPacket.h"
#include "EndPhoneFileTransferPacket.h"
#include "EndPitFileTransferPacket.h"
#include "EndSessionPacket.h"
#include "FilePartSizePacket.h"
#include "FileTransferPacket.h"
#include "FlashPartFileTransferPacket.h"
#include "FlashPartPitFilePacket.h"
#include "InboundPacket.h"
#include "Interface.h"
#include "OutboundPacket.h"
#include "PitFilePacket.h"
#include "PitFileResponse.h"
#include "ReceiveFilePartPacket.h"
#include "ResponsePacket.h"
#include "SendFilePartPacket.h"
#include "SendFilePartResponse.h"
#include "SessionSetupPacket.h"
#include "SessionSetupResponse.h"

// Future versions of libusb will use usb_interface instead of interface.
#define usb_interface interface

#define USB_CLASS_CDC_DATA 0x0A

using namespace Heimdall;

const DeviceIdentifier BridgeManager::supportedDevices[BridgeManager::kSupportedDeviceCount] = {
	DeviceIdentifier(BridgeManager::kVidSamsung, BridgeManager::kPidGalaxyS),
	DeviceIdentifier(BridgeManager::kVidSamsung, BridgeManager::kPidGalaxyS2),
	DeviceIdentifier(BridgeManager::kVidSamsung, BridgeManager::kPidDroidCharge)
};

enum
{
	kDumpBufferSize = 4096
};

enum
{
	kFileTransferSequenceMaxLengthDefault = 800,
	kFileTransferPacketSizeDefault = 131072,
	kFileTransferSequenceTimeoutDefault = 30000 // 30 seconds
};

enum
{
	kHandshakeMaxAttempts = 5,
	kReceivePacketMaxAttempts = 5
};

int BridgeManager::FindDeviceInterface(void)
{
	Interface::Print("Detecting device...\n");

	struct libusb_device **devices;
	int deviceCount = libusb_get_device_list(libusbContext, &devices);

	for (int deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++)
	{
		libusb_device_descriptor descriptor;
		libusb_get_device_descriptor(devices[deviceIndex], &descriptor);

		for (int i = 0; i < BridgeManager::kSupportedDeviceCount; i++)
		{
			if (descriptor.idVendor == supportedDevices[i].vendorId && descriptor.idProduct == supportedDevices[i].productId)
			{
				heimdallDevice = devices[deviceIndex];
				libusb_ref_device(heimdallDevice);
				break;
			}
		}

		if (heimdallDevice)
			break;
	}

	libusb_free_device_list(devices, deviceCount);

	if (!heimdallDevice)
	{
		Interface::PrintDeviceDetectionFailed();
		return (BridgeManager::kInitialiseDeviceNotDetected);
	}

	int result = libusb_open(heimdallDevice, &deviceHandle);
	if (result != LIBUSB_SUCCESS)
	{
		Interface::PrintError("Failed to access device. libusb error: %d\n", result);
		return (BridgeManager::kInitialiseFailed);
	}

	libusb_device_descriptor deviceDescriptor;
	result = libusb_get_device_descriptor(heimdallDevice, &deviceDescriptor);
	if (result != LIBUSB_SUCCESS)
	{
		Interface::PrintError("Failed to retrieve device description\n");
		return (BridgeManager::kInitialiseFailed);
	}

	if (verbose)
	{
		unsigned char stringBuffer[128];

		if (libusb_get_string_descriptor_ascii(deviceHandle, deviceDescriptor.iManufacturer,
			stringBuffer, 128) >= 0)
		{
			Interface::Print("      Manufacturer: \"%s\"\n", stringBuffer);
		}

		if (libusb_get_string_descriptor_ascii(deviceHandle, deviceDescriptor.iProduct,
			stringBuffer, 128) >= 0)
		{
			Interface::Print("           Product: \"%s\"\n", stringBuffer);
		}

		if (libusb_get_string_descriptor_ascii(deviceHandle, deviceDescriptor.iSerialNumber,
			stringBuffer, 128) >= 0)
		{
			Interface::Print("         Serial No: \"%s\"\n", stringBuffer);
		}

		Interface::Print("\n            length: %d\n", deviceDescriptor.bLength);
		Interface::Print("      device class: %d\n", deviceDescriptor.bDeviceClass);
		Interface::Print("               S/N: %d\n", deviceDescriptor.iSerialNumber);
		Interface::Print("           VID:PID: %04X:%04X\n", deviceDescriptor.idVendor, deviceDescriptor.idProduct);
		Interface::Print("         bcdDevice: %04X\n", deviceDescriptor.bcdDevice);
		Interface::Print("   iMan:iProd:iSer: %d:%d:%d\n", deviceDescriptor.iManufacturer, deviceDescriptor.iProduct,
			deviceDescriptor.iSerialNumber);
		Interface::Print("          nb confs: %d\n", deviceDescriptor.bNumConfigurations);
	}

	libusb_config_descriptor *configDescriptor;
	result = libusb_get_config_descriptor(heimdallDevice, 0, &configDescriptor);

	if (result != LIBUSB_SUCCESS || !configDescriptor)
	{
		Interface::PrintError("Failed to retrieve config descriptor\n");
		return (BridgeManager::kInitialiseFailed);
	}

	interfaceIndex = -1;
	altSettingIndex = -1;

	for (int i = 0; i < configDescriptor->bNumInterfaces; i++)
	{
		for (int j = 0 ; j < configDescriptor->usb_interface[i].num_altsetting; j++)
		{
			if (verbose)
			{
				Interface::Print("\ninterface[%d].altsetting[%d]: num endpoints = %d\n",
					i, j, configDescriptor->usb_interface[i].altsetting[j].bNumEndpoints);
				Interface::Print("   Class.SubClass.Protocol: %02X.%02X.%02X\n",
					configDescriptor->usb_interface[i].altsetting[j].bInterfaceClass,
					configDescriptor->usb_interface[i].altsetting[j].bInterfaceSubClass,
					configDescriptor->usb_interface[i].altsetting[j].bInterfaceProtocol);
			}

			int inEndpointAddress = -1;
			int outEndpointAddress = -1;

			for (int k = 0; k < configDescriptor->usb_interface[i].altsetting[j].bNumEndpoints; k++)
			{
				const libusb_endpoint_descriptor *endpoint = &configDescriptor->usb_interface[i].altsetting[j].endpoint[k];

				if (verbose)
				{
					Interface::Print("       endpoint[%d].address: %02X\n", k, endpoint->bEndpointAddress);
					Interface::Print("           max packet size: %04X\n", endpoint->wMaxPacketSize);
					Interface::Print("          polling interval: %02X\n", endpoint->bInterval);
				}

				if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN)
					inEndpointAddress = endpoint->bEndpointAddress;
				else
					outEndpointAddress = endpoint->bEndpointAddress;
			}

			if (interfaceIndex < 0
				&& configDescriptor->usb_interface[i].altsetting[j].bNumEndpoints == 2
				&& configDescriptor->usb_interface[i].altsetting[j].bInterfaceClass == USB_CLASS_CDC_DATA
				&& inEndpointAddress != -1
				&& outEndpointAddress != -1)
			{
				interfaceIndex = i;
				altSettingIndex = j;
				inEndpoint = inEndpointAddress;
				outEndpoint = outEndpointAddress;
			}
		}
	}

	libusb_free_config_descriptor(configDescriptor);

	if (result != LIBUSB_SUCCESS)
	{
		Interface::PrintError("Failed to find correct interface configuration\n");
		return (BridgeManager::kInitialiseFailed);
	}

	return (BridgeManager::kInitialiseSucceeded);
}

bool BridgeManager::ClaimDeviceInterface(void)
{
	Interface::Print("Claiming interface...\n");

	int result = libusb_claim_interface(deviceHandle, interfaceIndex);

#ifdef OS_LINUX

	if (result != LIBUSB_SUCCESS)
	{
		detachedDriver = true;
		Interface::Print("Attempt failed. Detaching driver...\n");
		libusb_detach_kernel_driver(deviceHandle, interfaceIndex);
		Interface::Print("Claiming interface again...\n");
		result = libusb_claim_interface(deviceHandle, interfaceIndex);
	}

#endif

	if (result != LIBUSB_SUCCESS)
	{
		Interface::PrintError("Claiming interface failed!\n");
		return (false);
	}

	interfaceClaimed = true;

	return (true);
}

bool BridgeManager::SetupDeviceInterface(void)
{
	Interface::Print("Setting up interface...\n");

	int result = libusb_set_interface_alt_setting(deviceHandle, interfaceIndex, altSettingIndex);

	if (result != LIBUSB_SUCCESS)
	{
		Interface::PrintError("Setting up interface failed!\n");
		return (false);
	}

	Interface::Print("\n");
	return (true);
}

void BridgeManager::ReleaseDeviceInterface(void)
{
	Interface::Print("Releasing device interface...\n");

	libusb_release_interface(deviceHandle, interfaceIndex);

#ifdef OS_LINUX

	if (detachedDriver)
	{
		Interface::Print("Re-attaching kernel driver...\n");
		libusb_attach_kernel_driver(deviceHandle, interfaceIndex);
	}

#endif

	interfaceClaimed = false;
	Interface::Print("\n");
}

bool BridgeManager::CheckProtocol(void) const
{
	Interface::Print("Checking if protocol is initialised...\n");

	DeviceTypePacket deviceTypePacket;

	if (!SendPacket(&deviceTypePacket, 150, false))
	{
		Interface::Print("Protocol is not initialised.\n");
		return (false);
	}

	unsigned char buffer[1024];
	memset(buffer, 0, sizeof(buffer));

	SessionSetupResponse deviceTypeResponse;

	if (!ReceivePacket(&deviceTypeResponse, 150, false, buffer, sizeof(buffer)))
	{
		Interface::Print("Protocol is not initialised.\n\n");
		return (false);
	}

	Interface::Print("Protocol is initialised.\n\n");
	return (true);
}

bool BridgeManager::InitialiseProtocol(void) const
{
	Interface::Print("Initialising protocol...\n");

	unsigned char dataBuffer[7];

	int result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x22, 0x3, 0, nullptr, 0, 1000);

	if (result < 0 && verbose)
		Interface::PrintWarning("Control transfer #1 failed. Result: %d\n", result);

	memset(dataBuffer, 0, 7);
	dataBuffer[1] = 0xC2;
	dataBuffer[2] = 0x01;
	dataBuffer[6] = 0x07;

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x20, 0x0, 0, dataBuffer, 7, 1000);

	if (result < 0 && verbose)
		Interface::PrintWarning("Control transfer #2 failed. Result: %d\n", result);

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x22, 0x3, 0, nullptr, 0, 1000);

	if (result < 0 && verbose)
		Interface::PrintWarning("Control transfer #3 failed. Result: %d\n", result);

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x22, 0x2, 0, nullptr, 0, 1000);

	if (result < 0 && verbose)
		Interface::PrintWarning("Control transfer #4 failed. Result: %d\n", result);

	memset(dataBuffer, 0, 7);
	dataBuffer[1] = 0xC2;
	dataBuffer[2] = 0x01;
	dataBuffer[6] = 0x08;

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x20, 0x0, 0, dataBuffer, 7, 1000);

	if (result < 0 && verbose)
		Interface::PrintWarning("Control transfer #5 failed. Result: %d\n", result);

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x22, 0x2, 0, nullptr, 0, 1000);

	if (result < 0 && verbose)
		Interface::PrintWarning("Control transfer #6 failed. Result: %d\n", result);

	unsigned int attempt = 0;

	// max(250, communicationDelay)
	int retryDelay = (communicationDelay > 250) ? communicationDelay : 250;

	for (; attempt < kHandshakeMaxAttempts; attempt++)
	{
		if (attempt > 0)
		{
			if (verbose)
				Interface::PrintErrorSameLine(" Retrying...\n");
			
			// Wait longer each retry
			Sleep(retryDelay * (attempt + 1));
		}

		int dataTransferred = 0;

		// Send "ODIN"
		memcpy(dataBuffer, "ODIN", 4);
		memset(dataBuffer + 4, 0, 1);

		result = libusb_bulk_transfer(deviceHandle, outEndpoint, dataBuffer, 4, &dataTransferred, 1000);
		if (result < 0)
		{
			if (verbose)
				Interface::PrintError("Failed to send data: \"%s\"\n", dataBuffer);
			else
				Interface::PrintError("Failed to send data!");

			return (false);
		}

		if (dataTransferred != 4)
		{
			if (verbose)
				Interface::PrintError("Failed to complete sending of data: \"%s\"\n", dataBuffer);
			else
				Interface::PrintError("Failed to complete sending of data!");

			return (false);
		}

		// Expect "LOKE"
		memset(dataBuffer, 0, 7);

		int retry = 0;
		dataTransferred = 0;

		result = libusb_bulk_transfer(deviceHandle, inEndpoint, dataBuffer, 7, &dataTransferred, 1000);

		if (result < 0)
		{
			if (verbose)
				Interface::PrintError("Failed to receive handshake response.");
		}
		else
		{
			if (dataTransferred == 4 && memcmp(dataBuffer, "LOKE", 4) == 0)
			{
				// Successfully received "LOKE"
				break;
			}
			else
			{
				if (verbose)
					Interface::PrintError("Expected: \"%s\"\nReceived: \"%s\"\n", "LOKE", dataBuffer);

				Interface::PrintError("Unexpected handshake response!");
			}
		}
	}

	if (attempt == kHandshakeMaxAttempts)
	{
		if (verbose)
			Interface::PrintErrorSameLine("\n");

		Interface::PrintError("Protocol initialisation failed!\n\n");
		return (false);
	}
	else
	{
		Interface::Print("Protocol initialisation successful.\n\n");
		return (true);
	}
}

BridgeManager::BridgeManager(bool verbose, int communicationDelay)
{
	this->verbose = verbose;
	this->communicationDelay = communicationDelay;

	libusbContext = nullptr;
	deviceHandle = nullptr;
	heimdallDevice = nullptr;

	inEndpoint = -1;
	outEndpoint = -1;
	interfaceIndex = -1;
	altSettingIndex = -1;

	interfaceClaimed = false;

#ifdef OS_LINUX

	detachedDriver = false;

#endif

	fileTransferSequenceMaxLength = kFileTransferSequenceMaxLengthDefault;
	fileTransferPacketSize = kFileTransferPacketSizeDefault;
	fileTransferSequenceTimeout = kFileTransferSequenceTimeoutDefault;
}

BridgeManager::~BridgeManager()
{
	if (interfaceClaimed)
		ReleaseDeviceInterface();

	if (deviceHandle)
		libusb_close(deviceHandle);

	if (heimdallDevice)
		libusb_unref_device(heimdallDevice);

	if (libusbContext)
		libusb_exit(libusbContext);
}

bool BridgeManager::DetectDevice(void)
{
	// Initialise libusb-1.0
	int result = libusb_init(&libusbContext);
	if (result != LIBUSB_SUCCESS)
	{
		Interface::PrintError("Failed to initialise libusb. libusb error: %d\n", result);
		return (false);
	}

	// Get handle to Galaxy S device
	struct libusb_device **devices;
	int deviceCount = libusb_get_device_list(libusbContext, &devices);

	for (int deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++)
	{
		libusb_device_descriptor descriptor;
		libusb_get_device_descriptor(devices[deviceIndex], &descriptor);

		for (int i = 0; i < BridgeManager::kSupportedDeviceCount; i++)
		{
			if (descriptor.idVendor == supportedDevices[i].vendorId && descriptor.idProduct == supportedDevices[i].productId)
			{
				libusb_free_device_list(devices, deviceCount);

				Interface::Print("Device detected\n");
				return (true);
			}
		}
	}

	libusb_free_device_list(devices, deviceCount);

	Interface::PrintDeviceDetectionFailed();
	return (false);
}

int BridgeManager::Initialise()
{
	Interface::Print("Initialising connection...\n");

	// Initialise libusb-1.0
	int result = libusb_init(&libusbContext);

	if (result != LIBUSB_SUCCESS)
	{
		Interface::PrintError("Failed to initialise libusb. libusb error: %d\n", result);
		Interface::Print("Failed to connect to device!");
		return (BridgeManager::kInitialiseFailed);
	}
	
	result = FindDeviceInterface();

	if (result != BridgeManager::kInitialiseSucceeded)
		return (result);

	if (!ClaimDeviceInterface())
		return (BridgeManager::kInitialiseFailed);

	if (!SetupDeviceInterface())
		return (BridgeManager::kInitialiseFailed);

	if (!CheckProtocol())
	{
		if (!InitialiseProtocol())
			return (BridgeManager::kInitialiseFailed);
	}

	return (BridgeManager::kInitialiseSucceeded);
}

bool BridgeManager::BeginSession(void)
{
	Interface::Print("Beginning session...\n");

	BeginSessionPacket beginSessionPacket;

	if (!SendPacket(&beginSessionPacket))
	{
		Interface::PrintError("Failed to begin session!\n");
		return (false);
	}

	SessionSetupResponse beginSessionResponse;
	if (!ReceivePacket(&beginSessionResponse))
		return (false);

	unsigned int result = beginSessionResponse.GetResult();

	if (result != 0) // Assume 0 means don't care, otherwise use the response as the fileTransferPacketSize.
		fileTransferPacketSize = result;

	// -------------------- KIES DOESN'T DO THIS --------------------

	DeviceTypePacket deviceTypePacket;

	if (!SendPacket(&deviceTypePacket))
	{
		Interface::PrintError("Failed to request device type!\n");
		return (false);
	}

	SessionSetupResponse deviceTypeResponse;

	if (!ReceivePacket(&deviceTypeResponse))
		return (false);

	unsigned int deviceType = deviceTypeResponse.GetResult();

	switch (deviceType)
	{
		// NOTE: If you add a new device type don't forget to update the error message below!

		case 0: // Galaxy S etc.
		case 3: // Galaxy Tab
		case 30: // Galaxy S 2 Skyrocket
		case 180: // Galaxy S etc.
		case 190: // M110S Galaxy S

			if (verbose)
				Interface::Print("Session begun with device of type: %d.\n\n", deviceType);
			else
				Interface::Print("Session begun.\n\n", deviceType);

			if (deviceType == 30)
			{
				Interface::Print("In certain situations this device may take up to 2 minutes to respond.\nPlease be patient!\n\n", deviceType);
				Sleep(2000); // Give the user time to read the message.

				// The SGH-I727 is very unstable/unreliable using the default settings. Flashing
				// seems to be much more reliable using the following setup.
				
				fileTransferSequenceMaxLength = 30;
				fileTransferSequenceTimeout = 120000; // 2 minutes!
#if 0
// This may be correct for the SGH-I727, but it appears to not be correct
// for the SGH-T989.
				fileTransferPacketSize = 1048576; // 1 MiB

				FilePartSizePacket filePartSizePacket(fileTransferPacketSize); // 1 MiB

				if (!SendPacket(&filePartSizePacket))
				{
					Interface::PrintError("Failed to send file part size packet!\n");
					return (false);
				}

				SessionSetupResponse filePartSizeResponse;

				if (!ReceivePacket(&filePartSizeResponse))
					return (false);

				if (filePartSizeResponse.GetResult() != 0)
				{
					Interface::PrintError("Unexpected file part size response!\nExpected: 0\nReceived: %d\n", filePartSizeResponse.GetResult());
					return (false);
				}
#endif
			}

			return (true);

		default:

			Interface::PrintError("Unexpected device info response!\nExpected: 0, 3, 30, 180 or 190\nReceived:%d\n", deviceType);
			return (false);
	}
}

bool BridgeManager::EndSession(bool reboot) const
{
	Interface::Print("Ending session...\n");

	EndSessionPacket *endSessionPacket = new EndSessionPacket(EndSessionPacket::kRequestEndSession);
	bool success = SendPacket(endSessionPacket);
	delete endSessionPacket;

	if (!success)
	{
		Interface::PrintError("Failed to send end session packet!\n");

		return (false);
	}

	ResponsePacket *endSessionResponse = new ResponsePacket(ResponsePacket::kResponseTypeEndSession);
	success = ReceivePacket(endSessionResponse);
	delete endSessionResponse;

	if (!success)
	{
		Interface::PrintError("Failed to receive session end confirmation!\n");

		return (false);
	}

	if (reboot)
	{
		Interface::Print("Rebooting device...\n");

		EndSessionPacket *rebootDevicePacket = new EndSessionPacket(EndSessionPacket::kRequestRebootDevice);
		bool success = SendPacket(rebootDevicePacket);
		delete rebootDevicePacket;

		if (!success)
		{
			Interface::PrintError("Failed to send reboot device packet!\n");

			return (false);
		}

		ResponsePacket *rebootDeviceResponse = new ResponsePacket(ResponsePacket::kResponseTypeEndSession);
		success = ReceivePacket(rebootDeviceResponse);
		delete rebootDeviceResponse;

		if (!success)
		{
			Interface::PrintError("Failed to receive reboot confirmation!\n");

			return (false);
		}
	}

	return (true);
}

bool BridgeManager::SendPacket(OutboundPacket *packet, int timeout, bool retry) const
{
	packet->Pack();

	int dataTransferred;
	int result = libusb_bulk_transfer(deviceHandle, outEndpoint, packet->GetData(), packet->GetSize(),
		&dataTransferred, timeout);

	if (result < 0 && retry)
	{
		// max(250, communicationDelay)
		int retryDelay = (communicationDelay > 250) ? communicationDelay : 250;

		if (verbose)
			Interface::PrintError("libusb error %d whilst sending packet.", result);

		// Retry
		for (int i = 0; i < 5; i++)
		{
			if (verbose)
				Interface::PrintErrorSameLine(" Retrying...\n");

			// Wait longer each retry
			Sleep(retryDelay * (i + 1));

			result = libusb_bulk_transfer(deviceHandle, outEndpoint, packet->GetData(), packet->GetSize(),
				&dataTransferred, timeout);

			if (result >= 0)
				break;

			if (verbose)
				Interface::PrintError("libusb error %d whilst sending packet.", result);
		}

		if (verbose)
			Interface::PrintErrorSameLine("\n");
	}

	if (communicationDelay != 0)
		Sleep(communicationDelay);

	if (result < 0 || dataTransferred != packet->GetSize())
		return (false);

	return (true);
}

bool BridgeManager::ReceivePacket(InboundPacket *packet, int timeout, bool retry, unsigned char *buffer, unsigned int bufferSize) const
{
	bool bufferProvided = buffer != nullptr && bufferSize >= packet->GetSize();

	if (!bufferProvided)
	{
		buffer = packet->GetData();
		bufferSize = packet->GetSize();
	}

	int dataTransferred;
	int result;

	unsigned int attempt = 0;
	unsigned int maxAttempts = (retry) ? kReceivePacketMaxAttempts : 1;
	
	// max(250, communicationDelay)
	int retryDelay = (communicationDelay > 250) ? communicationDelay : 250;

	for (; attempt < maxAttempts; attempt++)
	{
		if (attempt > 0)
		{
			if (verbose)
				Interface::PrintErrorSameLine(" Retrying...\n");
			
			// Wait longer each retry
			Sleep(retryDelay * (attempt + 1));
		}

		result = libusb_bulk_transfer(deviceHandle, inEndpoint, buffer, bufferSize, &dataTransferred, timeout);

		if (result >= 0)
			break;

		if (verbose)
			Interface::PrintError("libusb error %d whilst receiving packet.", result);
	}

	if (verbose && attempt > 0)
		Interface::PrintErrorSameLine("\n");

	if (attempt == maxAttempts)
		return (false);

	if (communicationDelay != 0)
		Sleep(communicationDelay);

	if (dataTransferred != packet->GetSize() && !packet->IsSizeVariable())
	{
		if (verbose)
			Interface::PrintError("Incorrect packet size received - expected size = %d, received size = %d.\n", packet->GetSize(), dataTransferred);

		return (false);
	}

	if (bufferProvided)
		memcpy(packet->GetData(), buffer, dataTransferred);

	packet->SetReceivedSize(dataTransferred);

	bool unpacked = packet->Unpack();

	if (!unpacked && verbose)
		Interface::PrintError("Failed to unpack received packet.\n");

	return (unpacked);
}

bool BridgeManager::RequestDeviceType(unsigned int request, int *result) const
{
	DeviceTypePacket deviceTypePacket;
	bool success = SendPacket(&deviceTypePacket);

	if (!success)
	{
		Interface::PrintError("Failed to request device info packet!\n");

		if (verbose)
			Interface::PrintError("Failed request: %u\n", request);

		return (false);
	}

	SessionSetupResponse deviceTypeResponse;

	if (!ReceivePacket(&deviceTypeResponse))
		return (false);

	*result = deviceTypeResponse.GetResult();

	return (true);
}

bool BridgeManager::SendPitFile(FILE *file) const
{
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	// Start file transfer
	PitFilePacket *pitFilePacket = new PitFilePacket(PitFilePacket::kRequestFlash);
	bool success = SendPacket(pitFilePacket);
	delete pitFilePacket;

	if (!success)
	{
		Interface::PrintError("Failed to initialise PIT file transfer!\n");
		return (false);
	}

	PitFileResponse *pitFileResponse = new PitFileResponse();
	success = ReceivePacket(pitFileResponse);
	delete pitFileResponse;

	if (!success)
	{
		Interface::PrintError("Failed to confirm transfer initialisation!\n");
		return (false);
	}

	// Transfer file size
	FlashPartPitFilePacket *flashPartPitFilePacket = new FlashPartPitFilePacket(fileSize);
	success = SendPacket(flashPartPitFilePacket);
	delete flashPartPitFilePacket;

	if (!success)
	{
		Interface::PrintError("Failed to send PIT file part information!\n");
		return (false);
	}

	pitFileResponse = new PitFileResponse();
	success = ReceivePacket(pitFileResponse);
	delete pitFileResponse;

	if (!success)
	{
		Interface::PrintError("Failed to confirm sending of PIT file part information!\n");
		return (false);
	}

	// Flash pit file
	SendFilePartPacket *sendFilePartPacket = new SendFilePartPacket(file, fileSize);
	success = SendPacket(sendFilePartPacket);
	delete sendFilePartPacket;

	if (!success)
	{
		Interface::PrintError("Failed to send file part packet!\n");
		return (false);
	}

	pitFileResponse = new PitFileResponse();
	success = ReceivePacket(pitFileResponse);
	delete pitFileResponse;

	if (!success)
	{
		Interface::PrintError("Failed to receive PIT file part response!\n");
		return (false);
	}

	// End pit file transfer
	EndPitFileTransferPacket *endPitFileTransferPacket = new EndPitFileTransferPacket(fileSize);
	success = SendPacket(endPitFileTransferPacket);
	delete endPitFileTransferPacket;

	if (!success)
	{
		Interface::PrintError("Failed to send end PIT file transfer packet!\n");
		return (false);
	}

	pitFileResponse = new PitFileResponse();
	success = ReceivePacket(pitFileResponse);
	delete pitFileResponse;

	if (!success)
	{
		Interface::PrintError("Failed to confirm end of PIT file transfer!\n");
		return (false);
	}

	return (true);
}

int BridgeManager::ReceivePitFile(unsigned char **pitBuffer) const
{
	*pitBuffer = nullptr;

	bool success;

	// Start file transfer
	PitFilePacket *pitFilePacket = new PitFilePacket(PitFilePacket::kRequestDump);
	success = SendPacket(pitFilePacket);
	delete pitFilePacket;

	if (!success)
	{
		Interface::PrintError("Failed to request receival of PIT file!\n");
		return (0);
	}

	PitFileResponse *pitFileResponse = new PitFileResponse();
	success = ReceivePacket(pitFileResponse);
	unsigned int fileSize = pitFileResponse->GetFileSize();
	delete pitFileResponse;

	if (!success)
	{
		Interface::PrintError("Failed to receive PIT file size!\n");
		return (0);
	}

	unsigned int transferCount = fileSize / ReceiveFilePartPacket::kDataSize;
	if (fileSize % ReceiveFilePartPacket::kDataSize != 0)
		transferCount++;

	unsigned char *buffer = new unsigned char[fileSize];
	int offset = 0;

	// NOTE: The PIT file appears to always be padded out to exactly 4 kilobytes.
	for (unsigned int i = 0; i < transferCount; i++)
	{
		DumpPartPitFilePacket *requestPacket = new DumpPartPitFilePacket(i);
		success = SendPacket(requestPacket);
		delete requestPacket;

		if (!success)
		{
			Interface::PrintError("Failed to request PIT file part #%d!\n", i);
			delete [] buffer;
			return (0);
		}
		
		ReceiveFilePartPacket *receiveFilePartPacket = new ReceiveFilePartPacket();
		success = ReceivePacket(receiveFilePartPacket);
		
		if (!success)
		{
			Interface::PrintError("Failed to receive PIT file part #%d!\n", i);
			delete receiveFilePartPacket;
			delete [] buffer;
			return (0);
		}

		// Copy the whole packet data into the buffer.
		memcpy(buffer + offset, receiveFilePartPacket->GetData(), receiveFilePartPacket->GetReceivedSize());
		offset += receiveFilePartPacket->GetReceivedSize();

		delete receiveFilePartPacket;
	}

	// End file transfer
	pitFilePacket = new PitFilePacket(PitFilePacket::kRequestEndTransfer);
	success = SendPacket(pitFilePacket);
	delete pitFilePacket;

	if (!success)
	{
		Interface::PrintError("Failed to send request to end PIT file transfer!\n");
		delete [] buffer;
		return (0);
	}

	pitFileResponse = new PitFileResponse();
	success = ReceivePacket(pitFileResponse);
	delete pitFileResponse;

	if (!success)
	{
		Interface::PrintError("Failed to receive end PIT file transfer verification!\n");
		delete [] buffer;
		return (0);
	}

	*pitBuffer = buffer;
	return (fileSize);
}

int BridgeManager::DownloadPitFile(unsigned char **pitBuffer) const
{
	Interface::Print("Downloading device's PIT file...\n");

	int devicePitFileSize = ReceivePitFile(pitBuffer);

	if (!*pitBuffer)
	{
		Interface::PrintError("Failed to download PIT file!\n");
		return (0);
	}

	Interface::Print("PIT file download successful.\n\n");
	return (devicePitFileSize);
}

bool BridgeManager::SendFile(FILE *file, unsigned int destination, unsigned int deviceType, unsigned int fileIdentifier) const
{
	if (destination != EndFileTransferPacket::kDestinationModem && destination != EndFileTransferPacket::kDestinationPhone)
	{
		Interface::PrintError("Attempted to send file to unknown destination!\n");
		return (false);
	}

	if (destination == EndFileTransferPacket::kDestinationModem && fileIdentifier != 0xFFFFFFFF)
	{
		Interface::PrintError("The modem file does not have an identifier!\n");
		return (false);
	}

	FileTransferPacket *flashFileTransferPacket = new FileTransferPacket(FileTransferPacket::kRequestFlash);
	bool success = SendPacket(flashFileTransferPacket);
	delete flashFileTransferPacket;

	if (!success)
	{
		Interface::PrintError("Failed to initialise file transfer!\n");
		return (false);
	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	ResponsePacket *fileTransferResponse = new ResponsePacket(ResponsePacket::kResponseTypeFileTransfer);
	success = ReceivePacket(fileTransferResponse);
	delete fileTransferResponse;

	if (!success)
	{
		Interface::PrintError("Failed to confirm transfer initialisation!\n");
		return (false);
	}

	unsigned int sequenceCount = fileSize / (fileTransferSequenceMaxLength * fileTransferPacketSize);
	unsigned int lastSequenceSize = fileTransferSequenceMaxLength;
	unsigned int partialPacketByteCount = fileSize % fileTransferPacketSize;

	if (fileSize % (fileTransferSequenceMaxLength * fileTransferPacketSize) != 0)
	{
		sequenceCount++;

		int lastSequenceBytes = fileSize % (fileTransferSequenceMaxLength * fileTransferPacketSize);
		lastSequenceSize = lastSequenceBytes / fileTransferPacketSize;

		if (partialPacketByteCount != 0)
			lastSequenceSize++;
	}

	long bytesTransferred = 0;
	unsigned int currentPercent;
	unsigned int previousPercent = 0;
	Interface::Print("0%%");

	for (unsigned int sequenceIndex = 0; sequenceIndex < sequenceCount; sequenceIndex++)
	{
		bool isLastSequence = (sequenceIndex == sequenceCount - 1);
		unsigned int sequenceSize = (isLastSequence) ? lastSequenceSize : fileTransferSequenceMaxLength;
		unsigned int sequenceByteCount;

		if (isLastSequence)
			sequenceByteCount = ((partialPacketByteCount) ? lastSequenceSize - 1 : lastSequenceSize) * fileTransferPacketSize + partialPacketByteCount;
		else
			sequenceByteCount = fileTransferSequenceMaxLength * fileTransferPacketSize;

		FlashPartFileTransferPacket *beginFileTransferPacket = new FlashPartFileTransferPacket(sequenceByteCount);
		success = SendPacket(beginFileTransferPacket);
		delete beginFileTransferPacket;

		if (!success)
		{
			Interface::PrintErrorSameLine("\n");
			Interface::PrintError("Failed to begin file transfer sequence!\n");
			return (false);
		}

		fileTransferResponse = new ResponsePacket(ResponsePacket::kResponseTypeFileTransfer);
		bool success = ReceivePacket(fileTransferResponse);
		delete fileTransferResponse;

		if (!success)
		{
			Interface::PrintErrorSameLine("\n");
			Interface::PrintError("Failed to confirm beginning of file transfer sequence!\n");
			return (false);
		}

		SendFilePartPacket *sendFilePartPacket;
		SendFilePartResponse *sendFilePartResponse;

		for (unsigned int filePartIndex = 0; filePartIndex < sequenceSize; filePartIndex++)
		{
			// Send
			sendFilePartPacket = new SendFilePartPacket(file, fileTransferPacketSize);
			success = SendPacket(sendFilePartPacket);
			delete sendFilePartPacket;

			if (!success)
			{
				Interface::PrintErrorSameLine("\n");
				Interface::PrintError("Failed to send file part packet!\n");
				return (false);
			}

			// Response
			sendFilePartResponse = new SendFilePartResponse();
			success = ReceivePacket(sendFilePartResponse);
			int receivedPartIndex = sendFilePartResponse->GetPartIndex();

			if (verbose)
			{
				const unsigned char *data = sendFilePartResponse->GetData();
				Interface::Print("File Part #%d... Response: %X  %X  %X  %X  %X  %X  %X  %X \n", filePartIndex,
					data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
			}

			delete sendFilePartResponse;

			if (!success)
			{
				Interface::PrintErrorSameLine("\n");
				Interface::PrintError("Failed to receive file part response!\n");

				for (int retry = 0; retry < 4; retry++)
				{
					Interface::PrintErrorSameLine("\n");
					Interface::PrintError("Retrying...");

					// Send
					sendFilePartPacket = new SendFilePartPacket(file, fileTransferPacketSize);
					success = SendPacket(sendFilePartPacket);
					delete sendFilePartPacket;

					if (!success)
					{
						Interface::PrintErrorSameLine("\n");
						Interface::PrintError("Failed to send file part packet!\n");
						return (false);
					}

					// Response
					sendFilePartResponse = new SendFilePartResponse();
					success = ReceivePacket(sendFilePartResponse);
					unsigned int receivedPartIndex = sendFilePartResponse->GetPartIndex();

					if (verbose)
					{
						const unsigned char *data = sendFilePartResponse->GetData();
						Interface::Print("File Part #%d... Response: %X  %X  %X  %X  %X  %X  %X  %X \n", filePartIndex,
							data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
					}

					delete sendFilePartResponse;

					if (receivedPartIndex != filePartIndex)
					{
						Interface::PrintErrorSameLine("\n");
						Interface::PrintError("Expected file part index: %d Received: %d\n", filePartIndex, receivedPartIndex);
						return (false);
					}

					if (success)
						break;
				}

				if (!success)
					return (false);
			}

			if (receivedPartIndex != filePartIndex)
			{
				Interface::PrintErrorSameLine("\n");
				Interface::PrintError("Expected file part index: %d Received: %d\n", filePartIndex, receivedPartIndex);
				return (false);
			}

			bytesTransferred += fileTransferPacketSize;

			if (bytesTransferred > fileSize)
				bytesTransferred = fileSize;

			currentPercent = (int)(100.0f * ((float)bytesTransferred / (float)fileSize));

			if (currentPercent != previousPercent)
			{
				if (!verbose)
				{
					if (previousPercent < 10)
						Interface::Print("\b\b%d%%", currentPercent);
					else
						Interface::Print("\b\b\b%d%%", currentPercent);
				}
				else
				{
					Interface::Print("\n%d%%\n", currentPercent);
				}
			}

			previousPercent = currentPercent;
		}

		if (destination == EndFileTransferPacket::kDestinationPhone)
		{
			EndPhoneFileTransferPacket *endPhoneFileTransferPacket = new EndPhoneFileTransferPacket(sequenceByteCount, 0, deviceType, fileIdentifier, isLastSequence);

			success = SendPacket(endPhoneFileTransferPacket, 3000);
			delete endPhoneFileTransferPacket;

			if (!success)
			{
				Interface::PrintErrorSameLine("\n");
				Interface::PrintError("Failed to end phone file transfer sequence!\n");
				return (false);
			}
		}
		else // destination == EndFileTransferPacket::kDestinationModem
		{
			EndModemFileTransferPacket *endModemFileTransferPacket = new EndModemFileTransferPacket(sequenceByteCount, 0, deviceType, isLastSequence);

			success = SendPacket(endModemFileTransferPacket, 3000);
			delete endModemFileTransferPacket;

			if (!success)
			{
				Interface::PrintErrorSameLine("\n");
				Interface::PrintError("Failed to end modem file transfer sequence!\n");
				return (false);
			}
		}

		fileTransferResponse = new ResponsePacket(ResponsePacket::kResponseTypeFileTransfer);
		success = ReceivePacket(fileTransferResponse, fileTransferSequenceTimeout);
		delete fileTransferResponse;

		if (!success)
		{
			Interface::PrintErrorSameLine("\n");
			Interface::PrintError("Failed to confirm end of file transfer sequence!\n");
			return (false);
		}
	}

	if (!verbose)
		Interface::Print("\n");

	return (true);
}

bool BridgeManager::ReceiveDump(unsigned int chipType, unsigned int chipId, FILE *file) const
{
	bool success;

	// Start file transfer
	BeginDumpPacket *beginDumpPacket = new BeginDumpPacket(chipType, chipId);
	success = SendPacket(beginDumpPacket);
	delete beginDumpPacket;

	if (!success)
	{
		Interface::PrintError("Failed to request dump!\n");
		return (false);
	}

	DumpResponse *dumpResponse = new DumpResponse();
	success = ReceivePacket(dumpResponse);
	unsigned int dumpSize = dumpResponse->GetDumpSize();
	delete dumpResponse;

	if (!success)
	{
		Interface::PrintError("Failed to receive dump size!\n");
		return (false);
	}

	unsigned int transferCount = dumpSize / ReceiveFilePartPacket::kDataSize;
	if (transferCount % ReceiveFilePartPacket::kDataSize != 0)
		transferCount++;

	char *buffer = new char[kDumpBufferSize * ReceiveFilePartPacket::kDataSize];
	unsigned int bufferOffset = 0;

	for (unsigned int i = 0; i < transferCount; i++)
	{
		DumpPartFileTransferPacket *dumpPartPacket = new DumpPartFileTransferPacket(i);
		success = SendPacket(dumpPartPacket);
		delete dumpPartPacket;

		if (!success)
		{
			Interface::PrintError("Failed to request dump part #%d!\n", i);
			delete [] buffer;
			return (false);
		}
		
		ReceiveFilePartPacket *receiveFilePartPacket = new ReceiveFilePartPacket();
		success = ReceivePacket(receiveFilePartPacket);
		
		if (!success)
		{
			Interface::PrintError("Failed to receive dump part #%d!\n", i);
			continue;
			delete receiveFilePartPacket;
			delete [] buffer;
			return (true);
		}

		if (bufferOffset + receiveFilePartPacket->GetReceivedSize() > kDumpBufferSize * ReceiveFilePartPacket::kDataSize)
		{
			// Write the buffer to the output file
			fwrite(buffer, 1, bufferOffset, file);
			bufferOffset = 0;
		}

		// Copy the packet data into pitFile.
		memcpy(buffer + bufferOffset, receiveFilePartPacket->GetData(), receiveFilePartPacket->GetReceivedSize());
		bufferOffset += receiveFilePartPacket->GetReceivedSize();

		delete receiveFilePartPacket;
	}

	if (bufferOffset != 0)
	{
		// Write the buffer to the output file
		fwrite(buffer, 1, bufferOffset, file);
	}
	
	delete [] buffer;

	// End file transfer
	FileTransferPacket *fileTransferPacket = new FileTransferPacket(FileTransferPacket::kRequestEnd);
	success = SendPacket(fileTransferPacket);
	delete fileTransferPacket;

	if (!success)
	{
		Interface::PrintError("Failed to send request to end dump transfer!\n");
		return (false);
	}

	ResponsePacket *responsePacket = new ResponsePacket(ResponsePacket::kResponseTypeFileTransfer);
	success = ReceivePacket(responsePacket);
	delete responsePacket;

	if (!success)
	{
		Interface::PrintError("Failed to receive end dump transfer verification!\n");
		return (false);
	}

	return (true);
}
