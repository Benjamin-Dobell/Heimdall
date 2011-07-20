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

// C Standard Library
#include <stdio.h>

// libusb
#include <libusb.h>

// Heimdall
#include "BeginDumpPacket.h"
#include "BridgeManager.h"
#include "DeviceInfoPacket.h"
#include "DeviceInfoResponse.h"
#include "DumpPartFileTransferPacket.h"
#include "DumpPartPitFilePacket.h"
#include "DumpResponse.h"
#include "EndModemFileTransferPacket.h"
#include "EndPhoneFileTransferPacket.h"
#include "EndSessionPacket.h"
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

// Future versions of libusb will use usb_interface instead of interface.
#define usb_interface interface

#define CLASS_CDC 0x0A

using namespace Heimdall;

const DeviceIdentifier BridgeManager::supportedDevices[BridgeManager::kSupportedDeviceCount] = {
	DeviceIdentifier(BridgeManager::kVidSamsung, BridgeManager::kPidGalaxyS),
	DeviceIdentifier(BridgeManager::kVidSamsung, BridgeManager::kPidGalaxyS2),
	DeviceIdentifier(BridgeManager::kVidSamsung, BridgeManager::kPidDroidCharge)
};

enum
{
	kMaxSequenceLength = 800
};

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

#ifdef OS_LINUX

	detachedDriver = false;

#endif
}

BridgeManager::~BridgeManager()
{
	if (interfaceIndex >= 0)
		libusb_release_interface(deviceHandle, interfaceIndex);

#ifdef OS_LINUX

	if (detachedDriver)
	{
		Interface::Print("Re-attaching kernel driver...\n");
		libusb_attach_kernel_driver(deviceHandle, interfaceIndex);
	}

#endif

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

int BridgeManager::Initialise(void)
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

	Interface::Print("Detecting device...\n");

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

	result = libusb_open(heimdallDevice, &deviceHandle);
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

	int interfaceIndex = -1;
	int altSettingIndex;

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

			if (interfaceIndex < 0 && configDescriptor->usb_interface[i].altsetting[j].bNumEndpoints == 2
				&& configDescriptor->usb_interface[i].altsetting[j].bInterfaceClass == CLASS_CDC
				&& inEndpointAddress != -1 && outEndpointAddress != -1)
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
	
	Interface::Print("Claiming interface...\n");
	result = libusb_claim_interface(deviceHandle, interfaceIndex);

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
		return (BridgeManager::kInitialiseFailed);
	}

	Interface::Print("Setting up interface...\n");
	result = libusb_set_interface_alt_setting(deviceHandle, interfaceIndex, altSettingIndex);

	if (result != LIBUSB_SUCCESS)
	{
		Interface::PrintError("Setting up interface failed!\n");
		return (BridgeManager::kInitialiseFailed);
	}

	Interface::Print("\n");

	return (BridgeManager::kInitialiseSucceeded);
}

bool BridgeManager::BeginSession(void) const
{
	Interface::Print("Beginning session...\n");

	unsigned char *dataBuffer = new unsigned char[7];

	int result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x22, 0x3, 0, nullptr, 0, 1000);

	if (result < 0)
	{
		Interface::PrintError("Failed to begin session!\n");

		delete [] dataBuffer;
		return (false);
	}

	memset(dataBuffer, 0, 7);
	dataBuffer[1] = 0xC2;
	dataBuffer[2] = 0x01;
	dataBuffer[6] = 0x07;

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x20, 0x0, 0, dataBuffer, 7, 1000);
	if (result < 0)
	{
		Interface::PrintError("Failed to begin session!\n");

		delete [] dataBuffer;
		return (false);
	}

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x22, 0x3, 0, nullptr, 0, 1000);
	if (result < 0)
	{
		Interface::PrintError("Failed to begin session!\n");

		delete [] dataBuffer;
		return (false);
	}

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x22, 0x2, 0, nullptr, 0, 1000);
	if (result < 0)
	{
		Interface::PrintError("Failed to begin session!\n");

		delete [] dataBuffer;
		return (false);
	}

	memset(dataBuffer, 0, 7);
	dataBuffer[1] = 0xC2;
	dataBuffer[2] = 0x01;
	dataBuffer[6] = 0x08;

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x20, 0x0, 0, dataBuffer, 7, 1000);
	if (result < 0)
	{
		Interface::PrintError("Failed to begin session!\n");

		delete [] dataBuffer;
		return (false);
	}

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x22, 0x2, 0, nullptr, 0, 1000);
	if (result < 0)
	{
		Interface::PrintError("Failed to begin session!\n");

		delete [] dataBuffer;
		return (false);
	}

	Interface::Print("Handshaking with Loke...\n");

	int dataTransferred;

	// Send "ODIN"
	strcpy((char *)dataBuffer, "ODIN");

	result = libusb_bulk_transfer(deviceHandle, outEndpoint, dataBuffer, 4, &dataTransferred, 1000);
	if (result < 0)
	{
		if (verbose)
			Interface::PrintError("Failed to send data: \"%s\"\n", dataBuffer);
		else
			Interface::PrintError("Failed to send data!");

		delete [] dataBuffer;
		return (false);
	}

	if (dataTransferred != 4)
	{
		if (verbose)
			Interface::PrintError("Failed to complete sending of data: \"%s\"\n", dataBuffer);
		else
			Interface::PrintError("Failed to complete sending of data!");

		delete [] dataBuffer;
		return (false);
	}

	// Expect "LOKE"
	memset(dataBuffer, 0, 7);

	result = libusb_bulk_transfer(deviceHandle, inEndpoint, dataBuffer, 7, &dataTransferred, 1000);
	if (result < 0)
	{
		Interface::PrintError("Failed to receive response!\n");

		delete [] dataBuffer;
		return (false);;
	}

	if (dataTransferred != 4 || memcmp(dataBuffer, "LOKE", 4) != 0)
	{
		Interface::PrintError("Unexpected communication!\n");

		if (verbose)
			Interface::PrintError("Expected: \"%s\"\nReceived: \"%s\"\n", "LOKE", dataBuffer);

		Interface::PrintError("Handshake failed!\n");

		delete [] dataBuffer;
		return (false);
	}

	Interface::Print("\n");

	return (true);
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

bool BridgeManager::SendPacket(OutboundPacket *packet, int timeout) const
{
	packet->Pack();

	int dataTransferred;
	int result = libusb_bulk_transfer(deviceHandle, outEndpoint, packet->GetData(), packet->GetSize(),
		&dataTransferred, timeout);

	if (result < 0)
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

bool BridgeManager::ReceivePacket(InboundPacket *packet, int timeout) const
{
	int dataTransferred;
	int result = libusb_bulk_transfer(deviceHandle, inEndpoint, packet->GetData(), packet->GetSize(),
		&dataTransferred, timeout);

	if (result < 0)
	{
		// max(250, communicationDelay)
		int retryDelay = (communicationDelay > 250) ? communicationDelay : 250;

		if (verbose)
			Interface::PrintError("libusb error %d whilst receiving packet.", result);

		// Retry
		for (int i = 0; i < 5; i++)
		{
			if (verbose)
				Interface::PrintErrorSameLine(" Retrying...\n");

			// Wait longer each retry
			Sleep(retryDelay * (i + 1));

			result = libusb_bulk_transfer(deviceHandle, inEndpoint, packet->GetData(), packet->GetSize(),
				&dataTransferred, timeout);

			if (result >= 0)
				break;

			if (verbose)
				Interface::PrintError("libusb error %d whilst receiving packet.", result);

			if (i >= 3)
			{
				int breakHere = 0;
				breakHere++;
			}
		}

		if (verbose)
			Interface::PrintErrorSameLine("\n");
	}

	if (communicationDelay != 0)
		Sleep(communicationDelay);

	if (result < 0 || (dataTransferred != packet->GetSize() && !packet->IsSizeVariable()))
		return (false);

	packet->SetReceivedSize(dataTransferred);

	return (packet->Unpack());
}

bool BridgeManager::RequestDeviceInfo(unsigned int request, int *result) const
{
	DeviceInfoPacket deviceInfoPacket(request);
	bool success = SendPacket(&deviceInfoPacket);

	if (!success)
	{
		Interface::PrintError("Failed to request device info packet!\n");

		if (verbose)
			Interface::PrintError("Failed request: %u\n", request);

		return (false);
	}

	DeviceInfoResponse deviceInfoResponse;
	success = ReceivePacket(&deviceInfoResponse);
	*result = deviceInfoResponse.GetUnknown();

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
		Interface::PrintError("Failed to receive PIT file transfer count!\n");
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
	int fileSize = pitFileResponse->GetFileSize();
	delete pitFileResponse;

	if (!success)
	{
		Interface::PrintError("Failed to receive PIT file size!\n");
		return (0);
	}

	int transferCount = fileSize / ReceiveFilePartPacket::kDataSize;
	if (fileSize % ReceiveFilePartPacket::kDataSize != 0)
		transferCount++;

	unsigned char *buffer = new unsigned char[fileSize];
	int offset = 0;

	// NOTE: The PIT file appears to always be padded out to exactly 4 kilobytes.
	for (int i = 0; i < transferCount; i++)
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

bool BridgeManager::SendFile(FILE *file, int destination, int fileIdentifier) const
{
	if (destination != EndFileTransferPacket::kDestinationModem && destination != EndFileTransferPacket::kDestinationPhone)
	{
		Interface::PrintError("Attempted to send file to unknown destination!\n");
		return (false);
	}

	if (destination == EndFileTransferPacket::kDestinationModem && fileIdentifier != -1)
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

	int sequenceCount = fileSize / (kMaxSequenceLength * SendFilePartPacket::kDefaultPacketSize);
	int lastSequenceSize = kMaxSequenceLength;
	int partialPacketLength = fileSize % SendFilePartPacket::kDefaultPacketSize;
	if  (fileSize % (kMaxSequenceLength * SendFilePartPacket::kDefaultPacketSize) != 0)
	{
		sequenceCount++;

		int lastSequenceBytes = fileSize % (kMaxSequenceLength * SendFilePartPacket::kDefaultPacketSize);
		lastSequenceSize = lastSequenceBytes / SendFilePartPacket::kDefaultPacketSize;
		if (partialPacketLength != 0)
			lastSequenceSize++;
	}

	long bytesTransferred = 0;
	int currentPercent;
	int previousPercent = 0;
	Interface::Print("0%%");

	for (int sequenceIndex = 0; sequenceIndex < sequenceCount; sequenceIndex++)
	{
		// Min(lastSequenceSize, 131072)
		bool isLastSequence = sequenceIndex == sequenceCount - 1;
		int sequenceSize = (isLastSequence) ? lastSequenceSize : kMaxSequenceLength;

		FlashPartFileTransferPacket *beginFileTransferPacket = new FlashPartFileTransferPacket(0, 2 * sequenceSize);
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

		for (int filePartIndex = 0; filePartIndex < sequenceSize; filePartIndex++)
		{
			// Send
			sendFilePartPacket = new SendFilePartPacket(file);
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
					sendFilePartPacket = new SendFilePartPacket(file);
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

			bytesTransferred += SendFilePartPacket::kDefaultPacketSize;
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

		int lastFullPacketIndex = 2 * ((isLastSequence && partialPacketLength != 0) ? sequenceSize - 1 : sequenceSize);

		if (destination == EndFileTransferPacket::kDestinationPhone)
		{
			EndPhoneFileTransferPacket *endPhoneFileTransferPacket = new EndPhoneFileTransferPacket(
				(isLastSequence) ? partialPacketLength : 0, lastFullPacketIndex, 0, 0, fileIdentifier, isLastSequence);

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
			EndModemFileTransferPacket *endModemFileTransferPacket = new EndModemFileTransferPacket(
				(isLastSequence) ? partialPacketLength : 0, lastFullPacketIndex, 0, 0, isLastSequence);

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
		success = ReceivePacket(fileTransferResponse, 30000);
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

bool BridgeManager::ReceiveDump(int chipType, int chipId, FILE *file) const
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
	int bufferOffset = 0;

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
