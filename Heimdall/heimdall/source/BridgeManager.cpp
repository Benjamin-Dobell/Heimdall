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

// C Standard Library
#include <stdio.h>

// libusb
#include <libusb.h>

// Heimdall
#include "BeginDumpPacket.h"
#include "BridgeManager.h"
#include "DumpPartFileTransferPacket.h"
#include "DumpPartPitFilePacket.h"
#include "DumpResponse.h"
#include "EndModemFileTransferPacket.h"
#include "EndPhoneFileTransferPacket.h"
#include "FileTransferPacket.h"
#include "FlashPartFileTransferPacket.h"
#include "FlashPartPitFilePacket.h"
#include "InboundPacket.h"
#include "InterfaceManager.h"
#include "OutboundPacket.h"
#include "PitFilePacket.h"
#include "PitFileResponse.h"
#include "ReceiveFilePartPacket.h"
#include "RebootDevicePacket.h"
#include "ResponsePacket.h"
#include "SendFilePartPacket.h"
#include "SendFilePartResponse.h"

// Future versions of libusb will use usb_interface instead of interface.
#define usb_interface interface

#define CLASS_CDC 0x0A

using namespace Heimdall;

const DeviceIdentifier BridgeManager::supportedDevices[BridgeManager::kSupportedDeviceCount] = {
	DeviceIdentifier(BridgeManager::kVidSamsung, BridgeManager::kPidGalaxySDownloadMode)/*,
	DeviceIdentifier(BridgeManager::kVidSamsung, BridgeManager::kPidGalaxySInternational),
	DeviceIdentifier(BridgeManager::kVidSamsung, BridgeManager::kPidGalaxySNewInternational),
	DeviceIdentifier(BridgeManager::kVidSamsung, BridgeManager::kPidVibrantCanadaBell)*/
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
		InterfaceManager::Print("Re-attaching kernel driver...\n");
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

bool BridgeManager::Initialise(void)
{
	// Initialise libusb-1.0
	int result = libusb_init(&libusbContext);
	if (result != LIBUSB_SUCCESS)
	{
		InterfaceManager::PrintError("Failed to initialise libusb. Error: %i\n", result);
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
		InterfaceManager::PrintError("Failed to detect compatible device\n");
		return (false);
	}

	result = libusb_open(heimdallDevice, &deviceHandle);
	if (result != LIBUSB_SUCCESS)
	{
		InterfaceManager::PrintError("Failed to access device. Error: %i\n", result);
		return (false);
	}

	libusb_device_descriptor deviceDescriptor;
	result = libusb_get_device_descriptor(heimdallDevice, &deviceDescriptor);
	if (result != LIBUSB_SUCCESS)
	{
		InterfaceManager::PrintError("Failed to retrieve device description\n");
		return (false);
	}

	if (verbose)
	{
		unsigned char stringBuffer[128];
		if (libusb_get_string_descriptor_ascii(deviceHandle, deviceDescriptor.iManufacturer,
			stringBuffer, 128) >= 0)
		{
			InterfaceManager::Print("      Manufacturer: \"%s\"\n", stringBuffer);
		}

		if (libusb_get_string_descriptor_ascii(deviceHandle, deviceDescriptor.iProduct,
			stringBuffer, 128) >= 0)
		{
			InterfaceManager::Print("           Product: \"%s\"\n", stringBuffer);
		}

		if (libusb_get_string_descriptor_ascii(deviceHandle, deviceDescriptor.iSerialNumber,
			stringBuffer, 128) >= 0)
		{
			InterfaceManager::Print("         Serial No: \"%s\"\n", stringBuffer);
		}

		InterfaceManager::Print("\n            length: %d\n", deviceDescriptor.bLength);
		InterfaceManager::Print("      device class: %d\n", deviceDescriptor.bDeviceClass);
		InterfaceManager::Print("               S/N: %d\n", deviceDescriptor.iSerialNumber);
		InterfaceManager::Print("           VID:PID: %04X:%04X\n", deviceDescriptor.idVendor, deviceDescriptor.idProduct);
		InterfaceManager::Print("         bcdDevice: %04X\n", deviceDescriptor.bcdDevice);
		InterfaceManager::Print("   iMan:iProd:iSer: %d:%d:%d\n", deviceDescriptor.iManufacturer, deviceDescriptor.iProduct,
			deviceDescriptor.iSerialNumber);
		InterfaceManager::Print("          nb confs: %d\n", deviceDescriptor.bNumConfigurations);
	}

	libusb_config_descriptor *configDescriptor;
	result = libusb_get_config_descriptor(heimdallDevice, 0, &configDescriptor);
	if (result != LIBUSB_SUCCESS || !configDescriptor)
	{
		InterfaceManager::PrintError("Failed to retrieve config descriptor\n");
		return (false);
	}

	int interfaceIndex = -1;
	int altSettingIndex;

	for (int i = 0; i < configDescriptor->bNumInterfaces; i++)
	{
		for (int j = 0 ; j < configDescriptor->usb_interface[i].num_altsetting; j++)
		{
			if (verbose)
			{
				InterfaceManager::Print("\ninterface[%d].altsetting[%d]: num endpoints = %d\n",
					i, j, configDescriptor->usb_interface[i].altsetting[j].bNumEndpoints);
				InterfaceManager::Print("   Class.SubClass.Protocol: %02X.%02X.%02X\n",
					configDescriptor->usb_interface[i].altsetting[j].bInterfaceClass,
					configDescriptor->usb_interface[i].altsetting[j].bInterfaceSubClass,
					configDescriptor->usb_interface[i].altsetting[j].bInterfaceProtocol);
			}

			int inEndpointAddress = -1;
			int outEndpointAddress = -1;

			for (int k = 0; k < configDescriptor->usb_interface[i].altsetting[j].bNumEndpoints; k++)
			{
				const libusb_endpoint_descriptor *endpoint =
					&configDescriptor->usb_interface[i].altsetting[j].endpoint[k];

				if (verbose)
				{
					InterfaceManager::Print("       endpoint[%d].address: %02X\n", k, endpoint->bEndpointAddress);
					InterfaceManager::Print("           max packet size: %04X\n", endpoint->wMaxPacketSize);
					InterfaceManager::Print("          polling interval: %02X\n", endpoint->bInterval);
				}

				if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN)
					inEndpointAddress = endpoint->bEndpointAddress;
				else
					outEndpointAddress = endpoint->bEndpointAddress;
			}

			if (interfaceIndex < 0
				&& configDescriptor->usb_interface[i].altsetting[j].bNumEndpoints == 2
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
		InterfaceManager::PrintError("Failed to find correct interface configuration\n");
		return (false);
	}
	
	InterfaceManager::Print("\nClaiming interface...");
	result = libusb_claim_interface(deviceHandle, interfaceIndex);

#ifdef OS_LINUX

	if (result != LIBUSB_SUCCESS)
	{
		detachedDriver = true;
		InterfaceManager::Print("   Failed. Attempting to detach driver...\n");
		libusb_detach_kernel_driver(deviceHandle, interfaceIndex);
		InterfaceManager::Print("Claiming interface again...");
		result = libusb_claim_interface(deviceHandle, interfaceIndex);
	}

#endif

	if (result != LIBUSB_SUCCESS)
	{
		InterfaceManager::PrintError("   Failed!\n");
		return (false);
	}

	InterfaceManager::Print("   Success\n");

	InterfaceManager::Print("Setting up interface...");
	result = libusb_set_interface_alt_setting(deviceHandle, interfaceIndex, altSettingIndex);
	if (result != LIBUSB_SUCCESS)
	{
		InterfaceManager::PrintError("   Failed!\n");
		return (false);
	}

	InterfaceManager::Print("   Success\n");

	return (true);
}

bool BridgeManager::BeginSession(void) const
{
	InterfaceManager::Print("Beginning session...\n");

	unsigned char *dataBuffer = new unsigned char[7];

	int result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x22, 0x3, 0, nullptr, 0, 1000);

	if (result < 0)
	{
		InterfaceManager::PrintError("Failed to initialise usb communication!\n");
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
		InterfaceManager::PrintError("Failed to initialise usb communication!\n");
		delete [] dataBuffer;
		return (false);
	}

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x22, 0x3, 0, nullptr, 0, 1000);
	if (result < 0)
	{
		InterfaceManager::PrintError("Failed to initialise usb communication!\n");
		delete [] dataBuffer;
		return (false);
	}

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x22, 0x2, 0, nullptr, 0, 1000);
	if (result < 0)
	{
		InterfaceManager::PrintError("Failed to initialise usb communication!\n");
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
		InterfaceManager::PrintError("Failed to initialise usb communication!\n");
		delete [] dataBuffer;
		return (false);
	}

	result = libusb_control_transfer(deviceHandle, LIBUSB_REQUEST_TYPE_CLASS, 0x22, 0x2, 0, nullptr, 0, 1000);
	if (result < 0)
	{
		InterfaceManager::PrintError("Failed to initialise usb communication!\n");
		delete [] dataBuffer;
		return (false);
	}

	InterfaceManager::Print("Handshaking with Loke...");

	int dataTransferred;

	// Send "ODIN"
	strcpy((char *)dataBuffer, "ODIN");

	result = libusb_bulk_transfer(deviceHandle, outEndpoint, dataBuffer, 4, &dataTransferred, 1000);
	if (result < 0)
	{
		InterfaceManager::PrintError("   Failed!\n");

		if (verbose)
			InterfaceManager::PrintError("ERROR: Failed to send data: \"%s\"\n", dataBuffer);

		delete [] dataBuffer;
		return (false);
	}

	if (dataTransferred != 4)
	{
		InterfaceManager::PrintError("   Failed!\n");

		if (verbose)
			InterfaceManager::PrintError("ERROR: Failed to complete sending data: \"%s\"\n", dataBuffer);

		delete [] dataBuffer;
		return (false);
	}

	// Expect "LOKE"
	memset(dataBuffer, 0, 7);

	result = libusb_bulk_transfer(deviceHandle, inEndpoint, dataBuffer, 7, &dataTransferred, 1000);
	if (result < 0)
	{
		InterfaceManager::PrintError("   Failed!\n");

		if (verbose)
			InterfaceManager::PrintError("ERROR: Failed to receive response\n");
		delete [] dataBuffer;
		return (false);;
	}

	if (dataTransferred != 4 || memcmp(dataBuffer, "LOKE", 4) != 0)
	{
		InterfaceManager::PrintError("   Failed!\n");

		if (verbose)
			InterfaceManager::PrintError("ERROR: Unexpected communication.\nExpected: \"%s\"\nReceived: \"%s\"\n", "LOKE", dataBuffer);

		delete [] dataBuffer;
		return (false);
	}

	InterfaceManager::Print("   Success\n\n");
	return (true);
}

bool BridgeManager::EndSession(void) const
{
	InterfaceManager::Print("Ending session...\n");

	RebootDevicePacket *rebootDevicePacket = new RebootDevicePacket(RebootDevicePacket::kRequestEndSession);
	bool success = SendPacket(rebootDevicePacket);
	delete rebootDevicePacket;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to send end session packet!\n");
		return (false);
	}

	ResponsePacket *rebootDeviceResponse = new ResponsePacket(ResponsePacket::kResponseTypeRebootDevice);
	success = ReceivePacket(rebootDeviceResponse);
	delete rebootDeviceResponse;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to receive session end confirmation!\n");
		return (false);
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
			InterfaceManager::PrintError("Error %i whilst sending packet. ", result);

		// Retry
		for (int i = 0; i < 5; i++)
		{
			if (verbose)
				InterfaceManager::PrintError(" Retrying...\n");

			// Wait longer each retry
			Sleep(retryDelay * (i + 1));

			result = libusb_bulk_transfer(deviceHandle, outEndpoint, packet->GetData(), packet->GetSize(),
				&dataTransferred, timeout);

			if (result >= 0)
				break;

			if (verbose)
				InterfaceManager::PrintError("Error %i whilst sending packet. ", result);
		}

		if (verbose)
				InterfaceManager::PrintError("\n");
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
			InterfaceManager::PrintError("Error %i whilst receiving packet. ", result);

		// Retry
		for (int i = 0; i < 5; i++)
		{
			if (verbose)
				InterfaceManager::PrintError(" Retrying\n");

			// Wait longer each retry
			Sleep(retryDelay * (i + 1));

			result = libusb_bulk_transfer(deviceHandle, inEndpoint, packet->GetData(), packet->GetSize(),
				&dataTransferred, timeout);

			if (result >= 0)
				break;

			if (verbose)
				InterfaceManager::PrintError("Error %i whilst receiving packet. ", result);

			if (i >= 3)
			{
				int breakHere = 0;
				breakHere++;
			}
		}

		if (verbose)
				InterfaceManager::PrintError("\n");
	}

	if (communicationDelay != 0)
		Sleep(communicationDelay);

	if (result < 0 || (dataTransferred != packet->GetSize() && !packet->IsSizeVariable()))
		return (false);

	packet->SetReceivedSize(dataTransferred);

	return (packet->Unpack());
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
		InterfaceManager::PrintError("Failed to request sending of PIT file!\n");
		return (false);
	}

	PitFileResponse *pitFileResponse = new PitFileResponse();
	success = ReceivePacket(pitFileResponse);
	delete pitFileResponse;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to confirm sending of PIT file!\n");
		return (false);
	}

	// Transfer file size
	FlashPartPitFilePacket *flashPartPitFilePacket = new FlashPartPitFilePacket(fileSize);
	success = SendPacket(flashPartPitFilePacket);
	delete flashPartPitFilePacket;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to send PIT file part information!\n");
		return (false);
	}

	pitFileResponse = new PitFileResponse();
	success = ReceivePacket(pitFileResponse);
	delete pitFileResponse;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to confirm sending of PIT file part information!\n");
		return (false);
	}

	// Flash pit file
	SendFilePartPacket *sendFilePartPacket = new SendFilePartPacket(file, fileSize);
	success = SendPacket(sendFilePartPacket);
	delete sendFilePartPacket;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to send file part packet!\n");
		return (false);
	}

	pitFileResponse = new PitFileResponse();
	success = ReceivePacket(pitFileResponse);
	delete pitFileResponse;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to receive PIT file transfer count!\n");
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
		InterfaceManager::PrintError("Failed to request receival of PIT file!\n");
		return (0);
	}

	PitFileResponse *pitFileResponse = new PitFileResponse();
	success = ReceivePacket(pitFileResponse);
	int fileSize = pitFileResponse->GetFileSize();
	delete pitFileResponse;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to receive PIT file size!\n");
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
			InterfaceManager::PrintError("Failed to request PIT file part #%i!\n", i);
			delete [] buffer;
			return (0);
		}
		
		ReceiveFilePartPacket *receiveFilePartPacket = new ReceiveFilePartPacket();
		success = ReceivePacket(receiveFilePartPacket);
		
		if (!success)
		{
			InterfaceManager::PrintError("Failed to receive PIT file part #%i!\n", i);
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
		InterfaceManager::PrintError("Failed to send request to end PIT file transfer!\n");
		delete [] buffer;
		return (0);
	}

	pitFileResponse = new PitFileResponse();
	success = ReceivePacket(pitFileResponse);
	delete pitFileResponse;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to receive end PIT file transfer verification!\n");
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
		InterfaceManager::PrintError("ERROR: Attempted to send file to unknown destination!\n");
		return (false);
	}

	if (destination == EndFileTransferPacket::kDestinationModem && fileIdentifier != -1)
	{
		InterfaceManager::PrintError("ERROR: The modem file does not have an identifier!\n");
		return (false);
	}

	FileTransferPacket *flashFileTransferPacket = new FileTransferPacket(FileTransferPacket::kRequestFlash);
	bool success = SendPacket(flashFileTransferPacket);
	delete flashFileTransferPacket;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to initialise transfer!\n");
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
		InterfaceManager::PrintError("Failed to confirm transfer initialisation!\n");
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
	InterfaceManager::Print("0%%");

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
			InterfaceManager::PrintError("\nFailed to begin file transfer sequence!\n");
			return (false);
		}

		fileTransferResponse = new ResponsePacket(ResponsePacket::kResponseTypeFileTransfer);
		bool success = ReceivePacket(fileTransferResponse);
		delete fileTransferResponse;

		if (!success)
		{
			InterfaceManager::PrintError("\nFailed to confirm beginning of file transfer sequence!\n");
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
				InterfaceManager::PrintError("\nFailed to send file part packet!\n");
				return (false);
			}

			// Response
			sendFilePartResponse = new SendFilePartResponse();
			success = ReceivePacket(sendFilePartResponse);
			int receivedPartIndex = sendFilePartResponse->GetPartIndex();

			if (verbose)
			{
				const unsigned char *data = sendFilePartResponse->GetData();
				InterfaceManager::Print("File Part #%i... Response: %X  %X  %X  %X  %X  %X  %X  %X \n", filePartIndex,
					data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
			}

			delete sendFilePartResponse;

			if (!success)
			{
				InterfaceManager::PrintError("\nFailed to receive file part response!\n");

				for (int retry = 0; retry < 4; retry++)
				{
					InterfaceManager::PrintError("\nRetrying...");

					// Send
					sendFilePartPacket = new SendFilePartPacket(file);
					success = SendPacket(sendFilePartPacket);
					delete sendFilePartPacket;

					if (!success)
					{
						InterfaceManager::PrintError("\nFailed to send file part packet!\n");
						return (false);
					}

					// Response
					sendFilePartResponse = new SendFilePartResponse();
					success = ReceivePacket(sendFilePartResponse);
					int receivedPartIndex = sendFilePartResponse->GetPartIndex();

					if (verbose)
					{
						const unsigned char *data = sendFilePartResponse->GetData();
						InterfaceManager::Print("File Part #%i... Response: %X  %X  %X  %X  %X  %X  %X  %X \n", filePartIndex,
							data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
					}

					delete sendFilePartResponse;

					if (receivedPartIndex != filePartIndex)
					{
						InterfaceManager::PrintError("\nERROR: Expected file part index: %i Received: %i\n",
							filePartIndex, receivedPartIndex);
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
				InterfaceManager::PrintError("\nERROR: Expected file part index: %i Received: %i\n",
					filePartIndex, receivedPartIndex);
				return (false);
			}

			bytesTransferred += SendFilePartPacket::kDefaultPacketSize;
			if (bytesTransferred > fileSize)
				bytesTransferred = fileSize;

			currentPercent = (int)(100.0f * ((float)bytesTransferred / (float)fileSize));

			if (!verbose)
			{
				if (currentPercent != previousPercent)
				{
					if (previousPercent < 10)
						InterfaceManager::Print("\b\b%i%%", currentPercent);
					else
						InterfaceManager::Print("\b\b\b%i%%", currentPercent);
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
				InterfaceManager::PrintError("\nFailed to end phone file transfer sequence!\n");
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
				InterfaceManager::PrintError("\nFailed to end modem file transfer sequence!\n");
				return (false);
			}
		}

		fileTransferResponse = new ResponsePacket(ResponsePacket::kResponseTypeFileTransfer);
		success = ReceivePacket(fileTransferResponse, 30000);
		delete fileTransferResponse;

		if (!success)
		{
			InterfaceManager::PrintError("\nFailed to confirm end of file transfer sequence!\n");
			return (false);
		}
	}

	if (!verbose)
		InterfaceManager::Print("\n");

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
		InterfaceManager::PrintError("Failed to request dump!\n");
		return (false);
	}

	DumpResponse *dumpResponse = new DumpResponse();
	success = ReceivePacket(dumpResponse);
	unsigned int dumpSize = dumpResponse->GetDumpSize();
	delete dumpResponse;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to receive dump size!\n");
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
			InterfaceManager::PrintError("Failed to request dump part #%i!\n", i);
			delete [] buffer;
			return (false);
		}
		
		ReceiveFilePartPacket *receiveFilePartPacket = new ReceiveFilePartPacket();
		success = ReceivePacket(receiveFilePartPacket);
		
		if (!success)
		{
			InterfaceManager::PrintError("Failed to receive dump part #%i!\n", i);
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
		InterfaceManager::PrintError("Failed to send request to end dump transfer!\n");
		return (false);
	}

	ResponsePacket *responsePacket = new ResponsePacket(ResponsePacket::kResponseTypeFileTransfer);
	success = ReceivePacket(responsePacket);
	delete responsePacket;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to receive end dump transfer verification!\n");
		return (false);
	}

	return (true);
}

bool BridgeManager::RebootDevice(void) const
{
	InterfaceManager::Print("Rebooting device...\n");

	RebootDevicePacket *rebootDevicePacket = new RebootDevicePacket(RebootDevicePacket::kRequestRebootDevice);
	bool success = SendPacket(rebootDevicePacket);
	delete rebootDevicePacket;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to send end session packet!\n");
		return (false);
	}

	ResponsePacket *rebootDeviceResponse = new ResponsePacket(ResponsePacket::kResponseTypeRebootDevice);
	success = ReceivePacket(rebootDeviceResponse);
	delete rebootDeviceResponse;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to receive reboot confirmation!\n");
		return (false);
	}

	return (true);
}
