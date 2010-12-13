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

// C/C++ Standard Library
#include <algorithm>
#include <map>
#include <stdio.h>
#include <string>

// Heimdall
#include "BridgeManager.h"
#include "DeviceInfoPacket.h"
#include "DeviceInfoResponse.h"
#include "EndModemFileTransferPacket.h"
#include "EndPhoneFileTransferPacket.h"
#include "InterfaceManager.h"

using namespace std;
using namespace Heimdall;

enum
{
	kFilePit = 0,
	kFileFactoryFs,
	kFileCache,
	kFileData,
	kFilePrimaryBootloader,
	kFileSecondaryBootloader,
	kFileSecondaryBootloaderBackup,
	kFileParam,
	kFileKernel,
	kFileRecovery,
	kFileEfs,
	kFileModem,
	kFileCount
};

bool flashFile(BridgeManager *bridgeManager, FILE *file, int fileIndex)
{
	switch (fileIndex)
	{
		case kFilePit:

			InterfaceManager::Print("Uploading PIT file\n");
			if (bridgeManager->SendPitFile(file))
			{
				InterfaceManager::Print("PIT file upload successful\n");
				return (true);
			}
			else
			{
				InterfaceManager::PrintError("PIT file upload failed!\n");
				return (false);
			}

		case kFileFactoryFs:

			InterfaceManager::Print("Uploading factory filesytem\n");
			if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone,
				EndPhoneFileTransferPacket::kFileFactoryFilesystem))
			{
				InterfaceManager::Print("Factory filesytem upload successful\n");
				return (true);
			}
			else
			{
				InterfaceManager::PrintError("Factory filesytem upload failed!\n");
				return (false);
			}

		case kFileCache:

			InterfaceManager::Print("Uploading cache\n");
			if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone,
				EndPhoneFileTransferPacket::kFileCache))
			{
				InterfaceManager::Print("Cache upload successful\n");
				return (true);
			}
			else
			{
				InterfaceManager::PrintError("Cache upload failed!\n");
				return (false);
			}

		case kFileData:

			InterfaceManager::Print("Uploading data database\n");
			if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone,
				EndPhoneFileTransferPacket::kFileDatabaseData))
			{
				InterfaceManager::Print("Data database upload successful\n");
				return (true);
			}
			else
			{
				InterfaceManager::PrintError("Data database upload failed!\n");
				return (false);
			}

		case kFilePrimaryBootloader:

			InterfaceManager::Print("Uploading primary bootloader\n");
			if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone,
				EndPhoneFileTransferPacket::kFilePrimaryBootloader))
			{
				InterfaceManager::Print("Primary bootloader upload successful\n");
				return (true);
			}
			else
			{
				InterfaceManager::PrintError("Primary bootloader upload failed!\n");
				return (false);
			}

		case kFileSecondaryBootloader:

			InterfaceManager::Print("Uploading secondary bootloader\n");
			if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone,
				EndPhoneFileTransferPacket::kFileSecondaryBootloader))
			{
				InterfaceManager::Print("Secondary bootloader upload successful\n");
				return (true);
			}
			else
			{
				InterfaceManager::PrintError("Secondary bootloader upload failed!\n");
				return (false);
			}

		case kFileSecondaryBootloaderBackup:

			InterfaceManager::Print("Uploading backup secondary bootloader\n");
			if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone,
				EndPhoneFileTransferPacket::kFileSecondaryBootloaderBackup))
			{
				InterfaceManager::Print("Backup secondary bootloader upload successful\n");
				return (true);
			}
			else
			{
				InterfaceManager::PrintError("Backup secondary bootloader upload failed!\n");
				return (false);
			}

		case kFileParam:

			InterfaceManager::Print("Uploading param.lfs\n");
			if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone,
				EndPhoneFileTransferPacket::kFileParamLfs))
			{
				InterfaceManager::Print("param.lfs upload successful\n");
				return (true);
			}
			else
			{
				InterfaceManager::PrintError("param.lfs upload failed!\n");
				return (false);
			}

		case kFileKernel:

			InterfaceManager::Print("Uploading kernel\n");
			if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone,
				EndPhoneFileTransferPacket::kFileKernel))
			{
				InterfaceManager::Print("Kernel upload successful\n");
				return (true);
			}
			else
			{
				InterfaceManager::PrintError("Kernel upload failed!\n");
				return (false);
			}

		case kFileModem:

			InterfaceManager::Print("Uploading modem\n");
			
			if (bridgeManager->SendFile(file, EndModemFileTransferPacket::kDestinationModem))     // <-- Odin method
			/*if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone,    // <-- Kies method. WARNING: Doesn't work on Galaxy Tab!
				EndPhoneFileTransferPacket::kFileModem))*/
			{
				InterfaceManager::Print("Modem upload successful\n");
				return (true);
			}
			else
			{
				InterfaceManager::PrintError("Modem upload failed!\n");
				return (false);
			}

		case kFileRecovery:

			InterfaceManager::Print("Uploading recovery\n");
			if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone,
				EndPhoneFileTransferPacket::kFileRecovery))
			{
				InterfaceManager::Print("Recovery upload successful\n");
				return (true);
			}
			else
			{
				InterfaceManager::PrintError("Recovery upload failed!\n");
				return (false);
			}

		case kFileEfs:

			InterfaceManager::Print("Uploading EFS\n");
			if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone,
				EndPhoneFileTransferPacket::kFileEfs))
			{
				InterfaceManager::Print("EFS upload successful\n");
				return (true);
			}
			else
			{
				InterfaceManager::PrintError("EFS upload failed!\n");
				return (false);
			}

		default:

			InterfaceManager::PrintError("ERROR: Attempted to flash unknown file!\n");
			return (false);
	}
}

bool attemptFlash(BridgeManager *bridgeManager, FILE **fileArray, bool repartition)
{
	bool success;
	
	// ---------- GET DEVICE INFORMATION ----------

	DeviceInfoPacket *deviceInfoPacket = new DeviceInfoPacket(DeviceInfoPacket::kUnknown1);
	success = bridgeManager->SendPacket(deviceInfoPacket);
	delete deviceInfoPacket;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to send device info packet!\nFailed Request: kUnknown1\n");
		return (false);
	}

	DeviceInfoResponse *deviceInfoResponse = new DeviceInfoResponse();
	success = bridgeManager->ReceivePacket(deviceInfoResponse);
	int unknown = deviceInfoResponse->GetUnknown();
	delete deviceInfoResponse;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to receive device info response!\n");
		return (false);
	}

	if (unknown != 0)
	{
		InterfaceManager::PrintError("Unexpected device info response!\nExpected: 0\nReceived:%i\n", unknown);

		if (!bridgeManager->EndSession())
			return (false);
		bridgeManager->RebootDevice();

		return (false);
	}

	// -------------------- KIES DOESN'T DO THIS --------------------
	deviceInfoPacket = new DeviceInfoPacket(DeviceInfoPacket::kUnknown2);
	success = bridgeManager->SendPacket(deviceInfoPacket);
	delete deviceInfoPacket;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to send device info packet!\nFailed Request: kUnknown2\n");
		return (false);
	}	

	deviceInfoResponse = new DeviceInfoResponse();
	success = bridgeManager->ReceivePacket(deviceInfoResponse);
	unknown = deviceInfoResponse->GetUnknown();
	delete deviceInfoResponse;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to receive device info response!\n");
		return (false);
	}

	// TODO: Work out what this value is... it has been either 180 or 0 for Galaxy S phones, and 3 on the Galaxy Tab.
	if (unknown != 180 && unknown != 0 && unknown != 3)
	{
		InterfaceManager::PrintError("Unexpected device info response!\nExpected: 180, 0 or 3\nReceived:%i\n", unknown);

		if (!bridgeManager->EndSession())
			return (false);
		bridgeManager->RebootDevice();

		return (false);
	}
	// --------------------------------------------------------------

	int totalBytes = 0;
	for (int i = kFileFactoryFs; i < kFileCount; i++)
	{
		if (fileArray[i])
		{
			fseek(fileArray[i], 0, SEEK_END);
			totalBytes += ftell(fileArray[i]);
			rewind(fileArray[i]);
		}
	}

	if (repartition)
	{
		// When repartitioning we send the PIT file to the device.
		fseek(fileArray[kFilePit], 0, SEEK_END);
		totalBytes += ftell(fileArray[kFilePit]);
		rewind(fileArray[kFilePit]);
	}
	
	deviceInfoPacket = new DeviceInfoPacket(DeviceInfoPacket::kTotalBytes, totalBytes);
	success = bridgeManager->SendPacket(deviceInfoPacket);
	delete deviceInfoPacket;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to send total bytes device info packet!\n");
		return (false);
	}

	deviceInfoResponse = new DeviceInfoResponse();
	success = bridgeManager->ReceivePacket(deviceInfoResponse);
	unknown = deviceInfoResponse->GetUnknown();
	delete deviceInfoResponse;

	if (!success)
	{
		InterfaceManager::PrintError("Failed to receive device info response!\n");
		return (false);
	}

	if (unknown != 0)
	{
		InterfaceManager::PrintError("Unexpected device info response!\nExpected: 0\nReceived:%i\n", unknown);

		if (!bridgeManager->EndSession())
			return (false);
		bridgeManager->RebootDevice();

		return (false);
	}

	// -----------------------------------------------------

	if (fileArray[kFilePit])
	{
		if (repartition)
		{
			flashFile(bridgeManager, fileArray[kFilePit], kFilePit);
		}
		else // We're performing a PIT check
		{
			// Load the local pit file into memory.
			char *localPit = new char[4096];
			memset(localPit, 0, 4096);

			fseek(fileArray[kFilePit], 0, SEEK_END);
			long localPitFileSize = ftell(fileArray[kFilePit]);
			rewind(fileArray[kFilePit]);

			fread(localPit, 1, localPitFileSize, fileArray[kFilePit]);

			InterfaceManager::Print("Downloading device's PIT file...\n");

			unsigned char *devicePit;
			int devicePitFileSize = bridgeManager->ReceivePitFile(&devicePit);

			if (!devicePit)
			{
				InterfaceManager::PrintError("Failed to download PIT file!\n");

				if (!bridgeManager->EndSession())
					return (false);
				bridgeManager->RebootDevice();

				return (false);
			}

			InterfaceManager::Print("PIT file download sucessful\n\n");

			bool pitFilesMatch = !memcmp(localPit, devicePit, localPitFileSize);

			delete [] localPit;
			delete [] devicePit;

			if (!pitFilesMatch)
			{
				InterfaceManager::Print("Optional PIT check failed! To disable this check don't use the --pit parameter.");

				if (!bridgeManager->EndSession())
					return (false);
				bridgeManager->RebootDevice();

				return (false);
			}
		}
	}

	// Flash specified files
	for (int fileIndex = kFileFactoryFs; fileIndex < kFileCount; fileIndex++)
	{
		if (fileArray[fileIndex])
		{
			if (!flashFile(bridgeManager, fileArray[fileIndex], fileIndex))
				return (false);
		}
	}

	return (bridgeManager->EndSession() && bridgeManager->RebootDevice());
}

bool openFiles(const map<string, string>& argumentMap, FILE **fileArray)
{
	for (int fileIndex = 0; fileIndex < kFileCount; fileIndex++)
	{
		// kFlashArgPit + kFile<Name> == kFlashArg<Name>
		map<string, string>::const_iterator it = argumentMap.find(InterfaceManager::flashArgumentNames[InterfaceManager::kFlashArgPit + fileIndex]);
		if (it == argumentMap.end())
			continue;

		fileArray[fileIndex] = fopen(it->second.c_str(), "rb");
		if (!fileArray[fileIndex])
		{
			InterfaceManager::PrintError("Failed to open file \"%s\"\n", it->second.c_str());
			return (false);
		}
	}

	return (true);
}

void closeFiles(FILE **fileArray)
{
	for (int fileIndex = 0; fileIndex < kFileCount; fileIndex++)
	{
		if (fileArray[fileIndex] != nullptr)
			fclose(fileArray[fileIndex]);
	}
}

int main(int argc, char **argv)
{
	map<string, string> argumentMap;
	int actionIndex;

	if (!InterfaceManager::GetArguments(argc, argv, argumentMap, &actionIndex))
	{
		Sleep(1000);
		return (0);
	}

	if (actionIndex == InterfaceManager::kActionHelp)
	{
		InterfaceManager::Print(InterfaceManager::usage);
		return (0);
	}
	else if (actionIndex == InterfaceManager::kActionFlash)
	{
		if (argumentMap.find(InterfaceManager::flashArgumentNames[InterfaceManager::kFlashArgRepartition]) != argumentMap.end()
			&& (argumentMap.find(InterfaceManager::flashArgumentNames[InterfaceManager::kFlashArgPit]) == argumentMap.end()
			|| argumentMap.find(InterfaceManager::flashArgumentNames[InterfaceManager::kFlashArgFactoryFs]) == argumentMap.end()
			|| argumentMap.find(InterfaceManager::flashArgumentNames[InterfaceManager::kFlashArgCache]) == argumentMap.end()
			|| argumentMap.find(InterfaceManager::flashArgumentNames[InterfaceManager::kFlashArgData]) == argumentMap.end()
			|| argumentMap.find(InterfaceManager::flashArgumentNames[InterfaceManager::kFlashArgPrimaryBootloader]) == argumentMap.end()
			|| argumentMap.find(InterfaceManager::flashArgumentNames[InterfaceManager::kFlashArgSecondaryBootloader]) == argumentMap.end()
			|| argumentMap.find(InterfaceManager::flashArgumentNames[InterfaceManager::kFlashArgParam]) == argumentMap.end()
			|| argumentMap.find(InterfaceManager::flashArgumentNames[InterfaceManager::kFlashArgKernel]) == argumentMap.end()))
		{
			InterfaceManager::Print("If you wish to repartition then factoryfs, cache, dbdata, primary and secondary\nbootloaders, param, kernel and a PIT file must all be specified.\n");
			return (0);
		}
	}
	else if (actionIndex == InterfaceManager::kActionDump)
	{
		if (argumentMap.find(InterfaceManager::dumpArgumentNames[InterfaceManager::kDumpArgOutput]) == argumentMap.end())
		{
			InterfaceManager::Print("Output file not specified.\n\n");
			InterfaceManager::Print(InterfaceManager::usage);
			return (0);
		}

		if (argumentMap.find(InterfaceManager::dumpArgumentNames[InterfaceManager::kDumpArgChipType]) == argumentMap.end())
		{
			InterfaceManager::Print("You must specify a chip type.\n\n");
			InterfaceManager::Print(InterfaceManager::usage);
			return (0);
		}

		string chipType = argumentMap.find(InterfaceManager::dumpArgumentNames[InterfaceManager::kDumpArgChipType])->second;
		if (!(chipType == "RAM" || chipType == "ram" || chipType == "NAND" || chipType == "nand"))
		{
			InterfaceManager::Print("Unknown chip type: %s.\n\n", chipType.c_str());
			InterfaceManager::Print(InterfaceManager::usage);
			return (0);
		}

		if (argumentMap.find(InterfaceManager::dumpArgumentNames[InterfaceManager::kDumpArgChipId]) == argumentMap.end())
		{
			InterfaceManager::Print("You must specify a Chip ID.\n\n");
			InterfaceManager::Print(InterfaceManager::usage);
			return (0);
		}

		int chipId = atoi(argumentMap.find(InterfaceManager::dumpArgumentNames[InterfaceManager::kDumpArgChipId])->second.c_str());
		if (chipId < 0)
		{
			InterfaceManager::Print("Chip ID must be a non-negative integer.\n");
			return (0);
		}
	}

	InterfaceManager::Print("\nHeimdall, Copyright (c) 2010, Benjamin Dobell, Glass Echidna\n");
	InterfaceManager::Print("http://www.glassechidna.com.au\n\n");
	InterfaceManager::Print("This software is provided free of charge. Copying and redistribution is\nencouraged.\n\n");
	InterfaceManager::Print("If you appreciate this software and you would like to support future\ndevelopment please consider donating:\n");
	InterfaceManager::Print("http://www.glassechidna.com.au/donate/\n\n");
	
	Sleep(1000);

	bool verbose = argumentMap.find(InterfaceManager::commonArgumentNames[InterfaceManager::kCommonArgVerbose]) != argumentMap.end();

	int communicationDelay = BridgeManager::kCommunicationDelayDefault;
	if (argumentMap.find(InterfaceManager::commonArgumentNames[InterfaceManager::kCommonArgDelay]) != argumentMap.end())
		communicationDelay = atoi(argumentMap.find(InterfaceManager::commonArgumentNames[InterfaceManager::kCommonArgDelay])->second.c_str());

	BridgeManager *bridgeManager = new BridgeManager(verbose, communicationDelay);

	if (!bridgeManager->Initialise())
	{
		delete bridgeManager;
		return (-2);
	}

	bool success;

	switch (actionIndex)
	{
		case InterfaceManager::kActionFlash:
		{
			FILE **fileArray = new FILE *[kFileCount];
			for (int i = 0; i < kFileCount; i++)
				fileArray[i] = nullptr;

			// We open the files before doing anything else to ensure they exist.
			if (!openFiles(argumentMap, fileArray))
			{
				closeFiles(fileArray);
				delete [] fileArray;

				delete bridgeManager;

				return (0);
			}

			if (!bridgeManager->BeginSession())
			{
				closeFiles(fileArray);
				delete [] fileArray;

				delete bridgeManager;

				return (-1);
			}

			bool repartition = argumentMap.find(InterfaceManager::flashArgumentNames[InterfaceManager::kFlashArgRepartition]) != argumentMap.end();
			success = attemptFlash(bridgeManager, fileArray, repartition);

			closeFiles(fileArray);
			delete [] fileArray;

			break;
		}

		case InterfaceManager::kActionClosePcScreen:
		{
			if (!bridgeManager->BeginSession())
			{
				delete bridgeManager;
				return (-1);
			}

			InterfaceManager::Print("Attempting to close connect to pc screen...\n");
			success = bridgeManager->EndSession() && bridgeManager->RebootDevice();

			if (success)
				InterfaceManager::Print("Attempt complete\n");

			break;
		}

		case InterfaceManager::kActionDump:
		{
			const char *outputFilename = argumentMap.find(InterfaceManager::dumpArgumentNames[InterfaceManager::kDumpArgOutput])->second.c_str();
			FILE *dumpFile = fopen(outputFilename, "wb");
			if (!dumpFile)
			{
				InterfaceManager::PrintError("Failed to open file \"%s\"\n", outputFilename);

				delete bridgeManager;
				return (-1);
			}

			int chipType = 0;
			string chipTypeName = argumentMap.find(InterfaceManager::dumpArgumentNames[InterfaceManager::kDumpArgChipType])->second;
			if (chipTypeName == "NAND" || chipTypeName == "nand")
				chipType = 1;

			int chipId = atoi(argumentMap.find(InterfaceManager::dumpArgumentNames[InterfaceManager::kDumpArgChipId])->second.c_str());

			if (!bridgeManager->BeginSession())
			{
				fclose(dumpFile);

				delete bridgeManager;

				return (-1);
			}

			success = bridgeManager->ReceiveDump(chipType, chipId, dumpFile);

			fclose(dumpFile);

			if (success)
				success = bridgeManager->EndSession() && bridgeManager->RebootDevice();

			break;
		}
	}

	delete bridgeManager;

	return ((success) ? 0 : -1);
}
