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

// C/C++ Standard Library
#include <algorithm>
#include <map>
#include <stdio.h>
#include <string>

// libpit
#include "libpit.h"

// Heimdall
#include "Heimdall.h"
#include "HelpAction.h"
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

struct PartitionInfo
{
	unsigned int chipIdentifier;
	string partitionName;
	FILE *file;

	PartitionInfo(unsigned int chipIdentifier, const char *partitionName, FILE *file)
	{
		this->chipIdentifier = chipIdentifier;
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
	knownPartitionNames[kKnownPartitionPrimaryBootloader].push_back("BOOTLOADER");

	knownPartitionNames[kKnownPartitionSecondaryBootloader].push_back("SBL");
	knownPartitionNames[kKnownPartitionSecondaryBootloader].push_back("SBL1");

	knownPartitionNames[kKnownPartitionSecondaryBootloaderBackup].push_back("SBL2");
	knownPartitionNames[kKnownPartitionParam].push_back("PARAM");

	knownPartitionNames[kKnownPartitionKernel].push_back("KERNEL");
	knownPartitionNames[kKnownPartitionKernel].push_back("BOOT");

	knownPartitionNames[kKnownPartitionRecovery].push_back("RECOVERY");
	knownPartitionNames[kKnownPartitionEfs].push_back("EFS");

	knownPartitionNames[kKnownPartitionModem].push_back("MODEM");
	knownPartitionNames[kKnownPartitionModem].push_back("RADIO");

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

bool mapFilesToPartitions(const map<string, FILE *>& argumentFileMap, const PitData *pitData, map<unsigned int, PartitionInfo>& partitionInfoMap)
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
				// NOTE: We're assuming a PIT file always has chipIdentifier zero.
				PartitionInfo partitionInfo(0, knownPartitionNames[kKnownPartitionPit][0], it->second);
				partitionInfoMap.insert(pair<unsigned int, PartitionInfo>(0xFFFFFFFF, partitionInfo));

				return (true);
			}
		}

		if (!pitEntry)
		{
			Interface::PrintError("Partition corresponding to %s argument could not be located\n", it->first.c_str());
			return (false);
		}

		PartitionInfo partitionInfo(pitEntry->GetChipIdentifier(), pitEntry->GetPartitionName(), it->second);
		partitionInfoMap.insert(pair<unsigned int, PartitionInfo>(pitEntry->GetPartitionIdentifier(), partitionInfo));
	}

	return (true);
}

void closeFiles(map<string, FILE *> argumentfileMap)
{
	for (map<string, FILE *>::iterator it = argumentfileMap.begin(); it != argumentfileMap.end(); it++)
		fclose(it->second);

	argumentfileMap.clear();
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

bool flashFile(BridgeManager *bridgeManager, unsigned int chipIdentifier, unsigned int partitionIndex, const char *partitionName, FILE *file)
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
			if (bridgeManager->SendFile(file, EndModemFileTransferPacket::kDestinationModem, chipIdentifier))     // <-- Odin method
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

			if (bridgeManager->SendFile(file, EndPhoneFileTransferPacket::kDestinationPhone, chipIdentifier, partitionIndex))
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
	
	SetupSessionPacket *deviceInfoPacket = new SetupSessionPacket(SetupSessionPacket::kTotalBytes, totalBytes);
	success = bridgeManager->SendPacket(deviceInfoPacket);
	delete deviceInfoPacket;

	if (!success)
	{
		Interface::PrintError("Failed to send total bytes device info packet!\n");
		return (false);
	}

	SetupSessionResponse *deviceInfoResponse = new SetupSessionResponse();
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

	map<unsigned int, PartitionInfo> partitionInfoMap;

	// Map the files being flashed to partitions stored in the PIT file.
	if (!mapFilesToPartitions(argumentFileMap, pitData, partitionInfoMap))
	{
		delete pitData;
		return (false);
	}

	delete pitData;

	// If we're repartitioning then we need to flash the PIT file first.
	if (repartition)
	{
		for (map<unsigned int, PartitionInfo>::iterator it = partitionInfoMap.begin(); it != partitionInfoMap.end(); it++)
		{
			if (it->second.file == localPitFile)
			{
				PartitionInfo *partitionInfo = &(it->second);

				if (!flashFile(bridgeManager, partitionInfo->chipIdentifier, it->first, partitionInfo->partitionName.c_str(), partitionInfo->file))
					return (false);

				break;
			}
		}
	}

	// Flash partitions not involved in the boot process second.
	for (map<unsigned int, PartitionInfo>::iterator it = partitionInfoMap.begin(); it != partitionInfoMap.end(); it++)
	{
		if (!isKnownPartition(it->second.partitionName.c_str(), kKnownPartitionPit) && !isKnownBootPartition(it->second.partitionName.c_str()))
		{
			PartitionInfo *partitionInfo = &(it->second);

			if (!flashFile(bridgeManager, partitionInfo->chipIdentifier, it->first, partitionInfo->partitionName.c_str(), partitionInfo->file))
				return (false);
		}
	}

	// Flash boot partitions last.
	for (map<unsigned int, PartitionInfo>::iterator it = partitionInfoMap.begin(); it != partitionInfoMap.end(); it++)
	{
		if (isKnownBootPartition(it->second.partitionName.c_str()))
		{
			PartitionInfo *partitionInfo = &(it->second);

			if (!flashFile(bridgeManager, partitionInfo->chipIdentifier, it->first, partitionInfo->partitionName.c_str(), partitionInfo->file))
				return (false);
		}
	}

	return (true);
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		Interface::PrintUsage();
		return (0);
	}

	int result = 0;
	map<string, Interface::ActionInfo>::const_iterator actionIt = Interface::GetActionMap().find(argv[1]);

	if (actionIt != Interface::GetActionMap().end())
		result = actionIt->second.executeFunction(argc, argv);
	else
		result = HelpAction::Execute(argc, argv);
	
	return (result);
}
