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

// Heimdall
#include "Arguments.h"
#include "BridgeManager.h"
#include "EndModemFileTransferPacket.h"
#include "EndPhoneFileTransferPacket.h"
#include "FlashAction.h"
#include "Heimdall.h"
#include "Interface.h"
#include "SessionSetupResponse.h"
#include "TotalBytesPacket.h"
#include "Utility.h"

using namespace Heimdall;

const char *FlashAction::usage = "Action: flash\n\
Arguments:\n\
    --repartition --pit <filename> [--factoryfs <filename>]\n\
    [--cache <filename>] [--dbdata <filename>] [--primary-boot <filename>]\n\
    [--secondary-boot <filename>] [--param <filename>] [--kernel <filename>]\n\
    [--modem <filename>] [--radio <filename>] [--normal-boot <filename>]\n\
    [--system <filename>] [--user-data <filename>] [--fota <filename>]\n\
    [--hidden <filename>] [--movinand <filename>] [--data <filename>]\n\
    [--ums <filename>] [--emmc <filename>]\n\
    [--<partition identifier> <filename>]\n\
    [--<partition name> <filename>]\n\
    [--verbose] [--no-reboot] [--stdout-errors] [--delay <ms>]\n\
  or:\n\
    [--factoryfs <filename>] [--cache <filename>] [--dbdata <filename>]\n\
    [--primary-boot <filename>] [--secondary-boot <filename>]\n\
    [--secondary-boot-backup <filename>] [--param <filename>]\n\
    [--kernel <filename>] [--recovery <filename>] [--efs <filename>]\n\
    [--modem <filename>] [--radio <filename>] [--normal-boot <filename>]\n\
    [--system <filename>] [--user-data <filename>] [--fota <filename>]\n\
    [--hidden <filename>] [--movinand <filename>] [--data <filename>]\n\
    [--ums <filename>] [--emmc <filename>] [--pit <filename>]\n\
    [--<partition identifier> <filename>]\n\
    [--<partition name> <filename>]\n\
    [--verbose] [--no-reboot] [--stdout-errors] [--delay <ms>]\n\
Description: Flashes firmware files to your phone. Partition identifiers are\n\
    integer values, they can be obtained by executing the print-pit action.\n\
WARNING: If you're repartitioning it's strongly recommended you specify\n\
        all files at your disposal, including bootloaders.\n";

struct PartitionFlashInfo
{
	const PitEntry *pitEntry;
	FILE *file;

	PartitionFlashInfo(const PitEntry *pitEntry, FILE *file)
	{
		this->pitEntry = pitEntry;
		this->file = file;
	}
};

static void buildArgumentPartitionNamesMap(map< string, vector<string> >& argumentPartitionNamesMap, map<string, string>& shortArgumentAliases)
{
	argumentPartitionNamesMap["pit"].push_back("PIT");
	argumentPartitionNamesMap["factoryfs"].push_back("FACTORYFS");
	argumentPartitionNamesMap["cache"].push_back("CACHE");
	argumentPartitionNamesMap["dbdata"].push_back("DBDATAFS");

	argumentPartitionNamesMap["primary-boot"].push_back("IBL+PBL");
	argumentPartitionNamesMap["primary-boot"].push_back("BOOT");

	argumentPartitionNamesMap["secondary-boot"].push_back("SBL");
	argumentPartitionNamesMap["secondary-boot"].push_back("SBL1");

	argumentPartitionNamesMap["secondary-boot-backup"].push_back("SBL2");
	argumentPartitionNamesMap["param"].push_back("PARAM");
	argumentPartitionNamesMap["kernel"].push_back("KERNEL");
	argumentPartitionNamesMap["recovery"].push_back("RECOVERY");
	argumentPartitionNamesMap["efs"].push_back("EFS");
	argumentPartitionNamesMap["modem"].push_back("MODEM");
	argumentPartitionNamesMap["radio"].push_back("RADIO");
	argumentPartitionNamesMap["normal-boot"].push_back("NORMALBOOT");
	argumentPartitionNamesMap["system"].push_back("SYSTEM");
	argumentPartitionNamesMap["user-data"].push_back("USERDATA");
	argumentPartitionNamesMap["fota"].push_back("FOTA");
	argumentPartitionNamesMap["hidden"].push_back("HIDDEN");
	argumentPartitionNamesMap["movinand"].push_back("MOVINAND");
	argumentPartitionNamesMap["data"].push_back("DATAFS");
	argumentPartitionNamesMap["ums"].push_back("UMS.EN");
	argumentPartitionNamesMap["emmc"].push_back("GANG");

	shortArgumentAliases["pit"] = "pit";
	shortArgumentAliases["fs"] = "factoryfs";
	shortArgumentAliases["cache"] = "cache";
	shortArgumentAliases["db"] = "dbdata";
	shortArgumentAliases["boot"] = "primary-boot";
	shortArgumentAliases["sbl"] = "secondary-boot";
	shortArgumentAliases["sbl2"] = "secondary-boot-backup";
	shortArgumentAliases["param"] = "param";
	shortArgumentAliases["z"] = "kernel";
	shortArgumentAliases["rec"] = "recovery";
	shortArgumentAliases["efs"] = "efs";
	shortArgumentAliases["m"] = "modem";
	shortArgumentAliases["rdio"] = "radio";
	shortArgumentAliases["norm"] = "normal-boot";
	shortArgumentAliases["sys"] = "system";
	shortArgumentAliases["udata"] = "user-data";
	shortArgumentAliases["fota"] = "fota";
	shortArgumentAliases["hide"] = "hidden";
	shortArgumentAliases["nand"] = "movinand";
	shortArgumentAliases["data"] = "data";
	shortArgumentAliases["ums"] = "ums";
	shortArgumentAliases["emmc"] = "emmc";
}

static bool openFiles(Arguments& arguments, const map< string, vector<string> >& argumentPartitionNamesMap,
	map<string, FILE *>& argumentFileMap)
{
	for (map<string, Argument *>::const_iterator it = arguments.GetArguments().begin(); it != arguments.GetArguments().end(); it++)
	{
		bool isPartitionArgument = false;
		const string& argumentName = it->first;

		if (arguments.GetArgumentTypes().find(argumentName) == arguments.GetArgumentTypes().end())
		{
			// The only way an argument could exist without being in the argument types map is if it's a wild-card.
			// The "%d" wild-card refers to a partition by identifier, where as the "%s" wild-card refers to a
			// partition by name.
			isPartitionArgument = true;
		}
		else
		{
			// The argument wasn't a wild-card, check if it's a known partition name.
			if (argumentPartitionNamesMap.find(argumentName) != argumentPartitionNamesMap.end())
				isPartitionArgument = true;
		}

		if (isPartitionArgument)
		{
			const StringArgument *stringArgument = static_cast<StringArgument *>(it->second);
			FILE *file = fopen(stringArgument->GetValue().c_str(), "rb");

			if (!file)
			{
				Interface::PrintError("Failed to open file \"%s\"\n", stringArgument->GetValue().c_str());
				return (false);
			}

			argumentFileMap[it->first] = file;
		}
	}

	return (true);
}

static void closeFiles(map<string, FILE *> argumentfileMap)
{
	for (map<string, FILE *>::iterator it = argumentfileMap.begin(); it != argumentfileMap.end(); it++)
		fclose(it->second);

	argumentfileMap.clear();
}

static bool sendTotalTransferSize(BridgeManager *bridgeManager, const map<string, FILE *>& argumentFileMap, bool repartition)
{
	int totalBytes = 0;
	for (map<string, FILE *>::const_iterator it = argumentFileMap.begin(); it != argumentFileMap.end(); it++)
	{
		if (repartition || it->first != "pit")
		{
			fseek(it->second, 0, SEEK_END);
			totalBytes += ftell(it->second);
			rewind(it->second);
		}
	}

	bool success;
	
	TotalBytesPacket *totalBytesPacket = new TotalBytesPacket(totalBytes);
	success = bridgeManager->SendPacket(totalBytesPacket);
	delete totalBytesPacket;

	if (!success)
	{
		Interface::PrintError("Failed to send total bytes device info packet!\n");
		return (false);
	}

	SessionSetupResponse *totalBytesResponse = new SessionSetupResponse();
	success = bridgeManager->ReceivePacket(totalBytesResponse);
	int totalBytesResult = totalBytesResponse->GetResult();
	delete totalBytesResponse;

	if (!success)
	{
		Interface::PrintError("Failed to receive device info response!\n");
		return (false);
	}

	if (totalBytesResult != 0)
	{
		Interface::PrintError("Unexpected device info response!\nExpected: 0\nReceived:%d\n", totalBytesResponse);
		return (false);
	}

	return (true);
}

static bool setupPartitionFlashInfo(const map<string, FILE *>& argumentFileMap, const map< string, vector<string> >& argumentPartitionNamesMap,
	const PitData *pitData, vector<PartitionFlashInfo>& partitionFlashInfos)
{
	for (map<string, FILE *>::const_iterator it = argumentFileMap.begin(); it != argumentFileMap.end(); it++)
	{
		const string& argumentName = it->first;
		FILE *partitionFile = it->second;

		const PitEntry *pitEntry = nullptr;

		// Was the argument a partition identifier?
		unsigned int partitionIdentifier;

		if (Utility::ParseUnsignedInt(partitionIdentifier, argumentName.c_str()) == kNumberParsingStatusSuccess)
		{
			pitEntry = pitData->FindEntry(partitionIdentifier);

			if (!pitEntry)
			{
				Interface::PrintError("No partition with identifier \"%s\" exists in the specified PIT.\n", argumentName.c_str());
				return (false);
			}
		}
		else
		{
			// The argument wasn't a partition identifier. Was it a known human-readable partition name?
			map< string, vector<string> >::const_iterator argumentPartitionNamesIt = argumentPartitionNamesMap.find(argumentName);

			if (argumentPartitionNamesIt != argumentPartitionNamesMap.end())
			{
				const vector<string>& partitionNames = argumentPartitionNamesIt->second;

				// Check for the partition in the PIT file using all known names.
				for (vector<string>::const_iterator nameIt = partitionNames.begin(); nameIt != partitionNames.end(); nameIt++)
				{
					pitEntry = pitData->FindEntry(nameIt->c_str());

					if (pitEntry)
						break;
				}

				if (!pitEntry)
				{
					Interface::PrintError("Partition name for \"%s\" could not be located\n", argumentName.c_str());
					return (false);
				}
			}
			else
			{
				// The argument must be an actual partition name. e.g. "ZIMAGE", instead of human-readable "kernel".
				pitEntry = pitData->FindEntry(argumentName.c_str());

				if (!pitEntry)
				{
					Interface::PrintError("Partition \"%s\" does not exist in the specified PIT.\n", argumentName.c_str());
					return (false);
				}
			}
		}

		partitionFlashInfos.push_back(PartitionFlashInfo(pitEntry, partitionFile));
	}

	return (true);
}

static bool isKnownPartition(const map<string, vector<string> >& argumentPartitionNamesMap, const string& argumentName, const string& partitionName)
{
	const vector<string>& partitionNames = argumentPartitionNamesMap.find(argumentName)->second;

	for (vector<string>::const_iterator it = partitionNames.begin(); it != partitionNames.end(); it++)
	{
		if (partitionName == *it)
			return (true);
	}

	return (false);
}

static bool isKnownBootPartition(const map<string, vector<string> >& argumentPartitionNamesMap, const char *partitionName)
{
	return (isKnownPartition(argumentPartitionNamesMap, "primary-boot", partitionName)
		|| isKnownPartition(argumentPartitionNamesMap, "secondary-boot", partitionName)
		|| isKnownPartition(argumentPartitionNamesMap, "secondary-boot-backup", partitionName)
		|| isKnownPartition(argumentPartitionNamesMap, "param", partitionName)
		|| isKnownPartition(argumentPartitionNamesMap, "normal-boot", partitionName)
		|| strcmp(partitionName, "SBL3") == 0
		|| strcmp(partitionName, "ABOOT") == 0 
		|| strcmp(partitionName, "RPM") == 0
		|| strcmp(partitionName, "TZ") == 0);
}

static bool flashFile(BridgeManager *bridgeManager, const map< string, vector<string> >& argumentPartitionNamesMap,
	const PartitionFlashInfo& partitionFlashInfo)
{
	// PIT files need to be handled differently, try determine if the partition we're flashing to is a PIT partition.

	if (isKnownPartition(argumentPartitionNamesMap, "pit", partitionFlashInfo.pitEntry->GetPartitionName()))
	{
		Interface::Print("Uploading %s\n", partitionFlashInfo.pitEntry->GetPartitionName());

		if (bridgeManager->SendPitFile(partitionFlashInfo.file))
		{
			Interface::Print("%s upload successful\n\n", partitionFlashInfo.pitEntry->GetPartitionName());
			return (true);
		}
		else
		{
			Interface::PrintError("%s upload failed!\n\n", partitionFlashInfo.pitEntry->GetPartitionName());
			return (false);
		}
	}
	else
	{
		if (partitionFlashInfo.pitEntry->GetBinaryType() == PitEntry::kBinaryTypeCommunicationProcessor) // Modem
		{			
			Interface::Print("Uploading %s\n", partitionFlashInfo.pitEntry->GetPartitionName());

			if (bridgeManager->SendFile(partitionFlashInfo.file, EndModemFileTransferPacket::kDestinationModem,
				partitionFlashInfo.pitEntry->GetDeviceType()))     // <-- Odin method
			{
				Interface::Print("%s upload successful\n\n", partitionFlashInfo.pitEntry->GetPartitionName());
				return (true);
			}
			else
			{
				Interface::PrintError("%s upload failed!\n\n", partitionFlashInfo.pitEntry->GetPartitionName());
				return (false);
			}
		}
		else // partitionFlashInfo.pitEntry->GetBinaryType() == PitEntry::kBinaryTypeApplicationProcessor
		{
			Interface::Print("Uploading %s\n", partitionFlashInfo.pitEntry->GetPartitionName());

			if (bridgeManager->SendFile(partitionFlashInfo.file, EndPhoneFileTransferPacket::kDestinationPhone,
				partitionFlashInfo.pitEntry->GetDeviceType(), partitionFlashInfo.pitEntry->GetIdentifier()))
			{
				Interface::Print("%s upload successful\n\n", partitionFlashInfo.pitEntry->GetPartitionName());
				return (true);
			}
			else
			{
				Interface::PrintError("%s upload failed!\n\n", partitionFlashInfo.pitEntry->GetPartitionName());
				return (false);
			}
		}
	}

	return (true);
}

static bool flashPartitions(const map<string, FILE *>& argumentFileMap, const map< string, vector<string> >& argumentPartitionNamesMap,
	const PitData *pitData, BridgeManager *bridgeManager, bool repartition)
{
	vector<PartitionFlashInfo> partitionFlashInfos;

	// Map the files being flashed to partitions stored in the PIT file.
	if (!setupPartitionFlashInfo(argumentFileMap, argumentPartitionNamesMap, pitData, partitionFlashInfos))
		return (false);

	// If we're repartitioning then we need to flash the PIT file first.
	if (repartition)
	{
		vector<PartitionFlashInfo>::const_iterator it;

		for (it = partitionFlashInfos.begin(); it != partitionFlashInfos.end(); it++)
		{
			if (isKnownPartition(argumentPartitionNamesMap, "pit", it->pitEntry->GetPartitionName()))
			{
				if (!flashFile(bridgeManager, argumentPartitionNamesMap, *it))
					return (false);

				break;
			}
		}

		if (it == partitionFlashInfos.end())
		{
			Interface::PrintError("Could not identify the PIT partition within the specified PIT file.\n\n");
			return (false);
		}
	}

	// Flash partitions not involved in the boot process second.
	for (vector<PartitionFlashInfo>::const_iterator it = partitionFlashInfos.begin(); it != partitionFlashInfos.end(); it++)
	{
		if (!isKnownPartition(argumentPartitionNamesMap, "pit", it->pitEntry->GetPartitionName())
			&& !isKnownBootPartition(argumentPartitionNamesMap, it->pitEntry->GetPartitionName()))
		{
			if (!flashFile(bridgeManager, argumentPartitionNamesMap, *it))
				return (false);
		}
	}

	// Flash boot partitions last.
	for (vector<PartitionFlashInfo>::const_iterator it = partitionFlashInfos.begin(); it != partitionFlashInfos.end(); it++)
	{
		if (isKnownBootPartition(argumentPartitionNamesMap, it->pitEntry->GetPartitionName()))
		{
			if (!flashFile(bridgeManager, argumentPartitionNamesMap, *it))
				return (false);
		}
	}

	return (true);
}

static PitData *getPitData(const map<string, FILE *>& argumentFileMap, BridgeManager *bridgeManager, bool repartition)
{
	PitData *pitData;
	PitData *localPitData = nullptr;

	// If a PIT file was passed as an argument then we must unpack it.

	map<string, FILE *>::const_iterator localPitFileIt = argumentFileMap.find("pit");

	if (localPitFileIt != argumentFileMap.end())
	{
		FILE *localPitFile = localPitFileIt->second;

		// Load the local pit file into memory.
		fseek(localPitFile, 0, SEEK_END);
		long localPitFileSize = ftell(localPitFile);
		rewind(localPitFile);

		unsigned char *pitFileBuffer = new unsigned char[localPitFileSize];
		memset(pitFileBuffer, 0, localPitFileSize);

		// dataRead is discarded, it's here to remove warnings.
		int dataRead = fread(pitFileBuffer, 1, localPitFileSize, localPitFile);

		if (dataRead > 0)
		{
			rewind(localPitFile);

			localPitData = new PitData();
			localPitData->Unpack(pitFileBuffer);

			delete [] pitFileBuffer;
		}
		else
		{
			Interface::PrintError("Failed to read PIT file.\n");

			delete [] pitFileBuffer;
			return (nullptr);
		}
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

		if (bridgeManager->DownloadPitFile(&pitFileBuffer) == 0)
			return (nullptr);

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
				return (nullptr);
			}
		}
	}

	return (pitData);
}

int FlashAction::Execute(int argc, char **argv)
{
	// Setup argument types

	map<string, ArgumentType> argumentTypes;

	argumentTypes["repartition"] = kArgumentTypeFlag;

	argumentTypes["no-reboot"] = kArgumentTypeFlag;
	argumentTypes["delay"] = kArgumentTypeUnsignedInteger;
	argumentTypes["verbose"] = kArgumentTypeFlag;
	argumentTypes["stdout-errors"] = kArgumentTypeFlag;

	map< string, vector<string> > argumentPartitionNamesMap;
	map<string, string> shortArgumentAliases;

	buildArgumentPartitionNamesMap(argumentPartitionNamesMap, shortArgumentAliases);

	for (map< string, vector<string> >::const_iterator it = argumentPartitionNamesMap.begin(); it != argumentPartitionNamesMap.end(); it++)
		argumentTypes[it->first] = kArgumentTypeString;

	// Add wild-cards "%d" and "%s", for partition identifiers and partition names respectively.
	argumentTypes["%d"] = kArgumentTypeString;
	shortArgumentAliases["%d"] = "%d";

	argumentTypes["%s"] = kArgumentTypeString;
	shortArgumentAliases["%s"] = "%s";

	map<string, string> argumentAliases;
	argumentAliases["PIT"] = "pit"; // Map upper-case PIT argument (i.e. partition name) to known lower-case pit argument.

	// Handle arguments

	Arguments arguments(argumentTypes, shortArgumentAliases, argumentAliases);

	if (!arguments.ParseArguments(argc, argv, 2))
	{
		Interface::Print(FlashAction::usage);
		return (0);
	}

	const UnsignedIntegerArgument *communicationDelayArgument = static_cast<const UnsignedIntegerArgument *>(arguments.GetArgument("delay"));

	bool reboot = arguments.GetArgument("no-reboot") == nullptr;
	bool verbose = arguments.GetArgument("verbose") != nullptr;
	
	if (arguments.GetArgument("stdout-errors") != nullptr)
		Interface::SetStdoutErrors(true);

	const StringArgument *pitArgument = static_cast<const StringArgument *>(arguments.GetArgument("pit"));

	bool repartition = arguments.GetArgument("repartition") != nullptr;

	if (repartition && !pitArgument)
	{
		Interface::Print("If you wish to repartition then a PIT file must be specified.\n\n");
		Interface::Print(FlashAction::usage);
		return (0);
	}

	// Open files

	map<string, FILE *> argumentFileMap;

	if (!openFiles(arguments, argumentPartitionNamesMap, argumentFileMap))
	{
		closeFiles(argumentFileMap);
		return (1);
	}

	if (argumentFileMap.size() == 0)
	{
		Interface::Print(FlashAction::usage);
		return (0);
	}

	// Info

	Interface::PrintReleaseInfo();
	Sleep(1000);

	// Perform flash

	int communicationDelay = BridgeManager::kCommunicationDelayDefault;

	if (communicationDelayArgument)
		communicationDelay = communicationDelayArgument->GetValue();

	BridgeManager *bridgeManager = new BridgeManager(verbose, communicationDelay);

	if (bridgeManager->Initialise() != BridgeManager::kInitialiseSucceeded || !bridgeManager->BeginSession())
	{
		closeFiles(argumentFileMap);
		delete bridgeManager;

		return (1);
	}

	bool success = sendTotalTransferSize(bridgeManager, argumentFileMap, repartition);

	if (success)
	{
		PitData *pitData = getPitData(argumentFileMap, bridgeManager, repartition);
	
		if (pitData)
			success = flashPartitions(argumentFileMap, argumentPartitionNamesMap, pitData, bridgeManager, repartition);
		else
			success = false;

		delete pitData;
	}
	
	closeFiles(argumentFileMap);

	if (!bridgeManager->EndSession(reboot))
		success = false;

	delete bridgeManager;

	return (success ? 0 : 1);
}
