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

// C/C++ Standard Library
#include <algorithm>
#include <map>
#include <stdio.h>
#include <string>

// libpit
#include "libpit.h"

// Heimdall
#include "BridgeManager.h"
#include "DeviceInfoPacket.h"
#include "DeviceInfoResponse.h"
#include "EndModemFileTransferPacket.h"
#include "EndPhoneFileTransferPacket.h"
#include "Interface.h"

using namespace std;
using namespace Heimdall;

// Known partitions
enum
{
	kKnownPartitionPit = 0,
	kKnownPartitionFactoryFs,
	kKnownPartitionCache,
	kKnownPartitionDatabaseData,
	kKnownPartitionPrimaryBootloader,
	kKnownPartitionSecondaryBootloader,
	kKnownPartitionSecondaryBootloaderBackup,
	kKnownPartitionParam,
	kKnownPartitionKernel,
	kKnownPartitionRecovery,
	kKnownPartitionEfs,
	kKnownPartitionModem,

	kKnownPartitionNormalBoot,
	kKnownPartitionSystem,
	kKnownPartitionUserData,
	kKnownPartitionFota,
	kKnownPartitionHidden,
	kKnownPartitionMovinand,
	kKnownPartitionData,
	kKnownPartitionUms,
	kKnownPartitionEmmc,

	kKnownPartitionCount
};

vector<const char *> knownPartitionNames[kKnownPartitionCount];

struct PartitionNameFilePair
{
	string partitionName;
	FILE *file;

	PartitionNameFilePair(const char *partitionName, FILE *file)
	{
		this->partitionName = partitionName;
		this->file = file;
	}
};

void initialiseKnownPartitionNames(void)
{
	knownPartitionNames[kKnownPartitionPit].push_back("PIT");
	knownPartitionNames[kKnownPartitionFactoryFs].push_back("FACTORYFS");
	knownPartitionNames[kKnownPartitionCache].push_back("CACHE");
	knownPartitionNames[kKnownPartitionDatabaseData].push_back("DBDATAFS");

	knownPartitionNames[kKnownPartitionPrimaryBootloader].push_back("IBL+PBL");
	knownPartitionNames[kKnownPartitionPrimaryBootloader].push_back("BOOT");

	knownPartitionNames[kKnownPartitionSecondaryBootloader].push_back("SBL");
	knownPartitionNames[kKnownPartitionSecondaryBootloader].push_back("SBL1");

	knownPartitionNames[kKnownPartitionSecondaryBootloaderBackup].push_back("SBL2");
	knownPartitionNames[kKnownPartitionParam].push_back("PARAM");
	knownPartitionNames[kKnownPartitionKernel].push_back("KERNEL");
	knownPartitionNames[kKnownPartitionRecovery].push_back("RECOVERY");
	knownPartitionNames[kKnownPartitionEfs].push_back("EFS");
	knownPartitionNames[kKnownPartitionModem].push_back("MODEM");

	knownPartitionNames[kKnownPartitionNormalBoot].push_back("NORMALBOOT");
	knownPartitionNames[kKnownPartitionSystem].push_back("SYSTEM");
	knownPartitionNames[kKnownPartitionUserData].push_back("USERDATA");
	knownPartitionNames[kKnownPartitionFota].push_back("FOTA");
	knownPartitionNames[kKnownPartitionHidden].push_back("HIDDEN");
	knownPartitionNames[kKnownPartitionMovinand].push_back("MOVINAND");
	knownPartitionNames[kKnownPartitionData].push_back("DATAFS");
	knownPartitionNames[kKnownPartitionUms].push_back("UMS.EN");
	knownPartitionNames[kKnownPartitionEmmc].push_back("GANG");
}

bool isKnownPartition(const char *partitionName, unsigned int knownPartitionIndex)
{
	for (unsigned int i = 0; i < knownPartitionNames[knownPartitionIndex].size(); i++)
	{
		if (strcmp(partitionName, knownPartitionNames[knownPartitionIndex][i]) == 0)
			return (true);
	}

	return (false);
}

bool isKnownBootPartition(const char *partitionName)
{
	return (isKnownPartition(partitionName, kKnownPartitionPrimaryBootloader) ||
		isKnownPartition(partitionName, kKnownPartitionSecondaryBootloader) ||
		isKnownPartition(partitionName, kKnownPartitionSecondaryBootloaderBackup) ||
		isKnownPartition(partitionName, kKnownPartitionParam) ||
		isKnownPartition(partitionName, kKnownPartitionNormalBoot));
}

bool openFiles(const map<string, string>& argumentMap, map<string, FILE *>& argumentFileMap)
{
	map<string, string>::const_iterator it = argumentMap.begin();

	for (it = argumentMap.begin(); it != argumentMap.end(); it++)
	{
		bool isFileArgument = false;

		int partitionIndex = atoi(it->first.substr(it->first.find_first_not_of('-')).c_str());

		// Was the argument a partition index?
		if (partitionIndex > 0 || it->first.compare("-0") == 0)
		{
			isFileArgument = true;
		}
		else
		{
			// The argument wasn't a partition index, check if it's a known partition name.
			for (int knownPartition = 0; knownPartition < kKnownPartitionCount; knownPartition++)
			{
				if (it->first == Interface::actions[Interface::kActionFlash].valueArguments[knownPartition])
				{
					isFileArgument = true;
					break;
				}
			}
		}

		if (!isFileArgument)
			continue;

		pair<string, FILE *> argumentFilePair;
		argumentFilePair.first = it->first;
		argumentFilePair.second = fopen(it->second.c_str(), "rb");

		if (!argumentFilePair.second)
		{
			Interface::PrintError("Failed to open file \"%s\"\n", it->second.c_str());
			return (false);
		}

		argumentFileMap.insert(argumentFilePair);
	}

	return (true);
}

bool mapFilesToPartitions(const map<string, FILE *>& argumentFileMap, const PitData *pitData, map<unsigned int, PartitionNameFilePair>& partitionFileMap)
{
	map<string, FILE *>::const_iterator it = argumentFileMap.begin();

	for (it = argumentFileMap.begin(); it != argumentFileMap.end(); it++)
	{
		int partitionIndex = atoi(it->first.substr(it->first.find_first_not_of('-')).c_str());

		const PitEntry *pitEntry = nullptr;

		// Was the argument a partition index?
		if (partitionIndex > 0 || it->first.compare("-0") == 0)
		{
			pitEntry = pitData->FindEntry(partitionIndex);
		}
		else
		{
			// The argument wasn't a partition index, so it must be a known partition name.
			int knownPartition;

			for (knownPartition = 0; knownPartition < kKnownPartitionCount; knownPartition++)
			{
				if (it->first == Interface::actions[Interface::kActionFlash].valueArguments[knownPartition])
					break;
			}

			// Check for the partition in the PIT file using all known names.
			for (unsigned int i = 0; i < knownPartitionNames[knownPartition].size(); i++)
			{
				pitEntry = pitData->FindEntry(knownPartitionNames[knownPartition][i]);

				if (pitEntry)
					break;
			}

			if (!pitEntry && knownPartition == kKnownPartitionPit)
			{
				PartitionNameFilePair partitionNameFilePair(knownPartitionNames[kKnownPartitionPit][0], it->second);
				partitionFileMap.insert(pair<unsigned int, PartitionNameFilePair>(static_cast<unsigned int>(-1), partitionNameFilePair));

				return (true);
			}
		}

		if (!pitEntry)
		{
			Interface::PrintError("Partition corresponding to %s argument could not be located\n", it->first.c_str());
			return (false);
		}

		PartitionNameFilePair partitionNameFilePair(pitEntry->GetPartitionName(), it->second);
		partitionFileMap.insert(pair<unsigned int, PartitionNameFilePair>(pitEntry->GetPartitionIdentifier(), partitionNameFilePair));
	}

	return (true);
}

void closeFiles(map<string, FILE *> argumentfileMap)
{
	for (map<string, FILE *>::iterator it = argumentfileMap.begin(); it != argumentfileMap.end(); it++)
		fclose(it->second);

	argumentfileMap.clear();
}

bool getDeviceInfo(BridgeManager *bridgeManager)
{
	// ---------- GET DEVICE INFORMATION ----------

	int deviceInfoResult;
	
	if (!bridgeManager->RequestDeviceInfo(DeviceInfoPacket::kUnknown1, &deviceInfoResult))
		return (false);

	// 131072 for Galaxy S II, 0 for other devices.
	if (deviceInfoResult != 0 && deviceInfoResult != 131072)
	{
		Interface::PrintError("Unexpected device info response!\nExpected: 0\nReceived:%d\n", deviceInfoResult);
		return (false);
	}

	// -------------------- KIES DOESN'T DO THIS --------------------

	if (!bridgeManager->RequestDeviceInfo(DeviceInfoPacket::kUnknown2, &deviceInfoResult))
		return (false);

	// TODO: Work out what this value is... it has been either 180 or 0 for Galaxy S phones, 3 on the Galaxy Tab, 190 for SHW-M110S.
	if (deviceInfoResult != 180 && deviceInfoResult != 0 && deviceInfoResult != 3 && deviceInfoResult != 190)
	{
		Interface::PrintError("Unexpected device info response!\nExpected: 180, 0 or 3\nReceived:%d\n", deviceInfoResult);
		return (false);
	}
}

int downloadPitFile(BridgeManager *bridgeManager, unsigned char **pitBuffer)
{
	Interface::Print("Downloading device's PIT file...\n");

	int devicePitFileSize = bridgeManager->ReceivePitFile(pitBuffer);

	if (!*pitBuffer)
	{
		Interface::PrintError("Failed to download PIT file!\n");

		return (-1);
	}

	Interface::Print("PIT file download sucessful\n\n");
	return (devicePitFileSize);
}

bool flashFile(BridgeManager *bridgeManager, unsigned int partitionIndex, const char *partitionName, FILE *file)
{
	// PIT files need to be handled differently, try determine if the partition we're flashing to is a PIT partition.
	bool isPit = false;

	for (unsigned int i = 0; i < knownPartitionNames[kKnownPartitionPit].size(); i++)
	{
		if (strcmp(partitionName, knownPartitionNames[kKnownPartitionPit][i]) == 0)
		{
			isPit = true;
			break;
		}
	}

	if (isPit)
	{
		Interface::Print("Uploading %s\n", partitionName);

		if (bridgeManager->SendPitFile(file))
		{
			Interface::Print("%s upload successful\n", partitionName);
			return (true);
		}
		else
		{
			Interface::Print("%s upload failed!\n", partitionName);
			return (false);
		}
	}
	else
	{
		// Modems need to be handled differently, try determine if the partition we're flashing to is a modem partition.
		bool isModem = false;

		for (unsigned int i = 0; i < knownPartitionNames[kKnownPartitionModem].size(); i++)
		{
			if (strcmp(partitionName, knownPartitionNames[kKnownPartitionModem][i]) == 0)
			{
				isModem = true;
				break;
			}
		}

		if (isModem)
		{			
			Interface::Print("Uploading %s\n", partitionName);

			//if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone,    // <-- Kies method. WARNING: Doesn't work on Galaxy Tab!
			//	EndPhoneFileTransferPacket::kFileModem))
			if (bridgeManager->SendFile(file, EndModemFileTransferPacket::kDestinationModem))     // <-- Odin method
			{
				Interface::Print("%s upload successful\n", partitionName);
				return (true);
			}
			else
			{
				Interface::Print("%s upload failed!\n", partitionName);
				return (false);
			}
		}
		else
		{
			// We're uploading to a phone partition
			Interface::Print("Uploading %s\n", partitionName);

			if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone, partitionIndex))
			{
				Interface::Print("%s upload successful\n", partitionName);
				return (true);
			}
			else
			{
				Interface::Print("%s upload failed!\n", partitionName);
				return (false);
			}
		}
	}

	return (true);
}

bool attemptFlash(BridgeManager *bridgeManager, map<string, FILE *> argumentFileMap, bool repartition)
{
	bool success;

	// ------------- SEND TOTAL BYTES TO BE TRANSFERRED -------------

	int totalBytes = 0;
	for (map<string, FILE *>::const_iterator it = argumentFileMap.begin(); it != argumentFileMap.end(); it++)
	{
		if (repartition || it->first != Interface::GetPitArgument())
		{
			fseek(it->second, 0, SEEK_END);
			totalBytes += ftell(it->second);
			rewind(it->second);
		}
	}
	
	DeviceInfoPacket *deviceInfoPacket = new DeviceInfoPacket(DeviceInfoPacket::kTotalBytes, totalBytes);
	success = bridgeManager->SendPacket(deviceInfoPacket);
	delete deviceInfoPacket;

	if (!success)
	{
		Interface::PrintError("Failed to send total bytes device info packet!\n");
		return (false);
	}

	DeviceInfoResponse *deviceInfoResponse = new DeviceInfoResponse();
	success = bridgeManager->ReceivePacket(deviceInfoResponse);
	int deviceInfoResult = deviceInfoResponse->GetUnknown();
	delete deviceInfoResponse;

	if (!success)
	{
		Interface::PrintError("Failed to receive device info response!\n");
		return (false);
	}

	if (deviceInfoResult != 0)
	{
		Interface::PrintError("Unexpected device info response!\nExpected: 0\nReceived:%d\n", deviceInfoResult);
		return (false);
	}

	// -----------------------------------------------------

	PitData *pitData;
	PitData *localPitData = nullptr;

	FILE *localPitFile = nullptr;

	// If a PIT file was passed as an argument then we must unpack it.
	map<string, FILE *>::iterator it = argumentFileMap.find(Interface::actions[Interface::kActionFlash].valueArguments[Interface::kFlashValueArgPit]);

	if (it != argumentFileMap.end())
	{
		localPitFile = it->second;

		// Load the local pit file into memory.
		unsigned char *pitFileBuffer = new unsigned char[4096];
		memset(pitFileBuffer, 0, 4096);

		fseek(localPitFile, 0, SEEK_END);
		long localPitFileSize = ftell(localPitFile);
		rewind(localPitFile);

		// dataRead is discarded, it's here to remove warnings.
		int dataRead = fread(pitFileBuffer, 1, localPitFileSize, localPitFile);
		rewind(localPitFile);

		localPitData = new PitData();
		localPitData->Unpack(pitFileBuffer);

		delete [] pitFileBuffer;
	}
	
	if (repartition)
	{
		// Use the local PIT file data.
		pitData = localPitData;
	}
	else
	{
		// If we're not repartitioning then we need to retrieve the device's PIT file and unpack it.
		unsigned char *pitFileBuffer;
		downloadPitFile(bridgeManager, &pitFileBuffer);

		pitData = new PitData();
		pitData->Unpack(pitFileBuffer);

		delete [] pitFileBuffer;

		if (localPitData != nullptr)
		{
			// The user has specified a PIT without repartitioning, we should verify the local and device PIT data match!
			bool pitsMatch = pitData->Matches(localPitData);
			delete localPitData;

			if (!pitsMatch)
			{
				Interface::Print("Local and device PIT files don't match and repartition wasn't specified!\n");
				Interface::PrintError("Flash aborted!\n");
				
				delete pitData;
				return (false);
			}
		}
	}

	map<unsigned int, PartitionNameFilePair> partitionFileMap;

	// Map the files being flashed to partitions stored in the PIT file.
	if (!mapFilesToPartitions(argumentFileMap, pitData, partitionFileMap))
	{
		delete pitData;
		return (false);
	}

	delete pitData;

	// If we're repartitioning then we need to flash the PIT file first.
	if (repartition)
	{
		for (map<unsigned int, PartitionNameFilePair>::iterator it = partitionFileMap.begin(); it != partitionFileMap.end(); it++)
		{
			if (it->second.file == localPitFile)
			{
				if (!flashFile(bridgeManager, it->first, it->second.partitionName.c_str(), it->second.file))
					return (false);

				break;
			}
		}
	}

	// Flash partitions not involved in the boot process second.
	for (map<unsigned int, PartitionNameFilePair>::iterator it = partitionFileMap.begin(); it != partitionFileMap.end(); it++)
	{
		if (!isKnownPartition(it->second.partitionName.c_str(), kKnownPartitionPit) && !isKnownBootPartition(it->second.partitionName.c_str()))
		{
			if (!flashFile(bridgeManager, it->first, it->second.partitionName.c_str(), it->second.file))
				return (false);
		}
	}

	// Flash boot partitions last.
	for (map<unsigned int, PartitionNameFilePair>::iterator it = partitionFileMap.begin(); it != partitionFileMap.end(); it++)
	{
		if (isKnownBootPartition(it->second.partitionName.c_str()))
		{
			if (!flashFile(bridgeManager, it->first, it->second.partitionName.c_str(), it->second.file))
				return (false);
		}
	}

	return (true);
}

int main(int argc, char **argv)
{
	map<string, string> argumentMap;
	int actionIndex;

	if (!Interface::GetArguments(argc, argv, argumentMap, &actionIndex))
	{
		Sleep(250);
		return (0);
	}

	initialiseKnownPartitionNames();

	switch (actionIndex)
	{
		case Interface::kActionFlash:
			if (argumentMap.find(Interface::actions[Interface::kActionFlash].valuelessArguments[Interface::kFlashValuelessArgRepartition]) != argumentMap.end()
				&& argumentMap.find(Interface::actions[Interface::kActionFlash].valueArguments[Interface::kFlashValueArgPit]) == argumentMap.end())
			{
				Interface::Print("If you wish to repartition then a PIT file must be specified.\n\n");
				Interface::PrintUsage();
				return (0);
			}

			break;

		case Interface::kActionDownloadPit:
			if (argumentMap.find(Interface::actions[Interface::kActionDownloadPit].valueArguments[Interface::kDownloadPitValueArgOutput]) == argumentMap.end())
			{
				Interface::Print("Output file was not specified.\n\n");
				Interface::PrintUsage();
				return (0);
			}

			break;

		case Interface::kActionDump:
		{
			if (argumentMap.find(Interface::actions[Interface::kActionDump].valueArguments[Interface::kDumpValueArgOutput]) == argumentMap.end())
			{
				Interface::Print("Output file was not specified.\n\n");
				Interface::PrintUsage();
				return (0);
			}

			if (argumentMap.find(Interface::actions[Interface::kActionDump].valueArguments[Interface::kDumpValueArgChipType]) == argumentMap.end())
			{
				Interface::Print("You must specify a chip type.\n\n");
				Interface::PrintUsage();
				return (0);
			}

			string chipType = argumentMap.find(Interface::actions[Interface::kActionDump].valueArguments[Interface::kDumpValueArgChipType])->second;
			if (!(chipType == "RAM" || chipType == "ram" || chipType == "NAND" || chipType == "nand"))
			{
				Interface::Print("Unknown chip type: %s.\n\n", chipType.c_str());
				Interface::PrintUsage();
				return (0);
			}

			if (argumentMap.find(Interface::actions[Interface::kActionDump].valueArguments[Interface::kDumpValueArgChipId]) == argumentMap.end())
			{
				Interface::Print("You must specify a Chip ID.\n\n");
				Interface::PrintUsage();
				return (0);
			}

			int chipId = atoi(argumentMap.find(Interface::actions[Interface::kActionDump].valueArguments[Interface::kDumpValueArgChipId])->second.c_str());
			if (chipId < 0)
			{
				Interface::Print("Chip ID must be a non-negative integer.\n");
				Interface::PrintUsage();
				return (0);
			}

			break;
		}

		case Interface::kActionVersion:
			Interface::PrintVersion();
			return (0);

		case Interface::kActionHelp:
			Interface::PrintUsage();
			return (0);

		case Interface::kActionInfo:
			Interface::PrintFullInfo();
			return (0);
	}

	bool verbose = argumentMap.find(Interface::commonValuelessArguments[Interface::kCommonValuelessArgVerbose]) != argumentMap.end();
	bool reboot = argumentMap.find(Interface::commonValuelessArguments[Interface::kCommonValuelessArgNoReboot]) == argumentMap.end();

	Interface::SetStdoutErrors(argumentMap.find(Interface::commonValuelessArguments[Interface::kCommonValuelessArgStdoutErrors]) != argumentMap.end());

	int communicationDelay = BridgeManager::kCommunicationDelayDefault;

	if (argumentMap.find(Interface::commonValueArguments[Interface::kCommonValueArgDelay]) != argumentMap.end())
		communicationDelay = atoi(argumentMap.find(Interface::commonValueArguments[Interface::kCommonValueArgDelay])->second.c_str());

	BridgeManager *bridgeManager = new BridgeManager(verbose, communicationDelay);

	if (actionIndex == Interface::kActionDetect)
	{
		bool detected = bridgeManager->DetectDevice();
		delete bridgeManager;

		return ((detected) ? 0 : 1);
	}

	Interface::PrintReleaseInfo();
	Sleep(1000);

	if (!bridgeManager->Initialise())
	{
		delete bridgeManager;
		return (0);
	}

	bool success;

	switch (actionIndex)
	{
		case Interface::kActionFlash:
		{
			map<string, FILE *> argumentFileMap;

			// We open the files before doing anything else to ensure they exist.
			if (!openFiles(argumentMap, argumentFileMap))
			{
				closeFiles(argumentFileMap);
				delete bridgeManager;

				return (0);
			}

			if (!bridgeManager->BeginSession() || !getDeviceInfo(bridgeManager))
			{
				closeFiles(argumentFileMap);
				delete bridgeManager;

				return (-1);
			}

			bool repartition = argumentMap.find(Interface::actions[Interface::kActionFlash].valuelessArguments[Interface::kFlashValuelessArgRepartition]) != argumentMap.end();
			success = attemptFlash(bridgeManager, argumentFileMap, repartition);

			success = bridgeManager->EndSession(reboot) && success;

			closeFiles(argumentFileMap);

			break;
		}

		case Interface::kActionClosePcScreen:
		{
			if (!bridgeManager->BeginSession() || !getDeviceInfo(bridgeManager))
			{
				delete bridgeManager;
				return (-1);
			}

			Interface::Print("Attempting to close connect to pc screen...\n");

			success = bridgeManager->EndSession(reboot);

			if (success)
				Interface::Print("Attempt complete\n");

			break;
		}

		case Interface::kActionDownloadPit:
		{
			map<string, string>::const_iterator it = argumentMap.find(Interface::actions[Interface::kActionDownloadPit].valueArguments[Interface::kDownloadPitValueArgOutput]);
			FILE *outputPitFile = fopen(it->second.c_str(), "wb");

			if (!outputPitFile)
			{
				delete bridgeManager;
				return (0);
			}

			if (!bridgeManager->BeginSession() || !getDeviceInfo(bridgeManager))
			{
				delete bridgeManager;
				fclose(outputPitFile);
				return (-1);
			}

			unsigned char *pitBuffer;
			int fileSize = downloadPitFile(bridgeManager, &pitBuffer);

			if (fileSize > 0)
			{
				success = fwrite(pitBuffer, 1, fileSize, outputPitFile) == fileSize;
				fclose(outputPitFile);

				if (!success)
					Interface::PrintError("Failed to write PIT data to output file.\n");

				success = bridgeManager->EndSession(reboot) && success;
			}
			else
			{
				fclose(outputPitFile);
				success = false;
				bridgeManager->EndSession(reboot);
			}

			delete [] pitBuffer;

			break;
		}

		case Interface::kActionDump:
		{
			const char *outputFilename = argumentMap.find(Interface::actions[Interface::kActionDump].valueArguments[Interface::kDumpValueArgOutput])->second.c_str();
			FILE *dumpFile = fopen(outputFilename, "wb");
			if (!dumpFile)
			{
				Interface::PrintError("Failed to open file \"%s\"\n", outputFilename);

				delete bridgeManager;
				return (-1);
			}

			int chipType = 0;
			string chipTypeName = argumentMap.find(Interface::actions[Interface::kActionDump].valueArguments[Interface::kDumpValueArgChipType])->second;
			if (chipTypeName == "NAND" || chipTypeName == "nand")
				chipType = 1;

			int chipId = atoi(argumentMap.find(Interface::actions[Interface::kActionDump].valueArguments[Interface::kDumpValueArgChipId])->second.c_str());

			if (!bridgeManager->BeginSession() || !getDeviceInfo(bridgeManager))
			{
				fclose(dumpFile);

				delete bridgeManager;
				return (-1);
			}

			success = bridgeManager->ReceiveDump(chipType, chipId, dumpFile);

			fclose(dumpFile);

			success = bridgeManager->EndSession(reboot) && success;

			break;
		}

		case Interface::kActionPrintPit:
		{
			if (!bridgeManager->BeginSession() || !getDeviceInfo(bridgeManager))
			{
				delete bridgeManager;
				return (-1);
			}

			unsigned char *devicePit;

			if (downloadPitFile(bridgeManager, &devicePit) < -1)
			{
				bridgeManager->EndSession(reboot);

				delete bridgeManager;
				return (-1);
			}

			PitData *pitData = new PitData();

			if (pitData->Unpack(devicePit))
			{
				Interface::PrintPit(pitData);
				success = true;
			}
			else
			{
				Interface::PrintError("Failed to unpack device's PIT file!\n");
				success = false;
			}
			
			delete [] devicePit;
			delete pitData;

			success = bridgeManager->EndSession(reboot) && success;

			break;
		}
	}

	delete bridgeManager;

	return ((success) ? 0 : -1);
}
