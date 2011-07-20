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
#include <cstdarg>
#include <cstdlib>
#include <stdio.h>

// Heimdall
#include "Heimdall.h"
#include "Interface.h"

using namespace std;
using namespace libpit;
using namespace Heimdall;

bool Interface::stdoutErrors = false;

const char *Interface::version = "v1.3 (beta)";

const char *Interface::usage = "Usage: heimdall <action> <action arguments> <common arguments>\n\
\n\
Common Arguments:\n\
    [--verbose] [--no-reboot] [--stdout-errors] [--delay <ms>]\n\
\n\
\n\
Action: flash\n\
Arguments:\n\
    --repartition --pit <filename> [--factoryfs <filename>]\n\
    [--cache <filename>] [--dbdata <filename>] [--primary-boot <filename>]\n\
    [--secondary-boot <filename>] [--param <filename>] [--kernel <filename>]\n\
    [--modem <filename>] [--normal-boot <filename>] [--system <filename>]\n\
    [--user-data <filename>] [--fota <filename>] [--hidden <filename>]\n\
    [--movinand <filename>] [--data <filename>] [--ums <filename>]\n\
    [--emmc <filename>] [--<partition identifier> <filename>]\n\
  or:\n\
    [--factoryfs <filename>] [--cache <filename>] [--dbdata <filename>]\n\
    [--primary-boot <filename>] [--secondary-boot <filename>]\n\
    [--secondary-boot-backup <filename>] [--param <filename>]\n\
    [--kernel <filename>] [--recovery <filename>] [--efs <filename>]\n\
    [--modem <filename>] [--normal-boot <filename>] [--system <filename>]\n\
    [--user-data <filename>] [--fota <filename>] [--hidden <filename>]\n\
    [--movinand <filename>] [--data <filename>] [--ums <filename>]\n\
    [--emmc <filename>] [--<partition identifier> <filename>]\n\
Description: Flashes firmware files to your phone.\n\
WARNING: If you're repartitioning it's strongly recommended you specify\n\
         all files at your disposal, including bootloaders.\n\
\n\
Action: close-pc-screen\n\
Description: Attempts to get rid off the \"connect phone to PC\" screen.\n\
\n\
Action: download-pit\n\
Arguments: --output <filename>\n\
Description: Downloads the connected device's PIT file to the specified\n\
    output file.\n\
\n\
Action: detect\n\
Description: Indicates whether or not a download mode device can be detected.\n\
\n\
Action: dump\n\
Arguments: --chip-type <NAND | RAM> --chip-id <integer> --output <filename>\n\
Description: Attempts to dump data from the phone corresponding to the\n\
	specified chip type and chip ID.\n\
NOTE: Galaxy S phones don't appear to properly support this functionality.\n\
\n\
Action: print-pit\n\
Description: Dumps the PIT file from the connected device and prints it in\n\
    a human readable format.\n\
\n\
Action: version\n\
Description: Displays the version number of this binary.\n\
\n\
Action: help\n\
Description: Displays this dialogue.\n";

const char *Interface::releaseInfo = "Heimdall %s, Copyright (c) 2010-2011, Benjamin Dobell, Glass Echidna\n\
http://www.glassechidna.com.au\n\n\
This software is provided free of charge. Copying and redistribution is\nencouraged.\n\n\
If you appreciate this software and you would like to support future\ndevelopment please consider donating:\n\
http://www.glassechidna.com.au/donate/\n\n";

const char *Interface::extraInfo = "Heimdall utilises libusb-1.0 for all USB communication:\n\
    http://www.libusb.org/\n\
\n\
libusb-1.0 is licensed under the LGPL-2.1:\n\
    http://www.gnu.org/licenses/licenses.html#LGPL\n\n";

// Flash arguments
string Interface::flashValueArguments[kFlashValueArgCount] = {
	"-pit", "-factoryfs", "-cache", "-dbdata", "-primary-boot",	"-secondary-boot", "-secondary-boot-backup", "-param", "-kernel", "-recovery", "-efs", "-modem",
	"-normal-boot", "-system", "-user-data", "-fota", "-hidden", "-movinand", "-data", "-ums", "-emmc", "-%d"
};

string Interface::flashValueShortArguments[kFlashValueArgCount] = {
	"pit",  "fs",         "cache",  "db",      "boot",           "sbl",            "sbl2",                   "param",  "z",       "rec",       "efs",  "m",
	"norm",         "sys",     "udata",      "fota",  "hide",    "nand",      "data",  "ums",  "emmc",  "%d"
};

string Interface::flashValuelessArguments[kFlashValuelessArgCount] = {
	"-repartition"
};

string Interface::flashValuelessShortArguments[kFlashValuelessArgCount] = {
	"r"
};

// Download PIT arguments
string Interface::downloadPitValueArguments[kDownloadPitValueArgCount] = {
	"-output"
};

string Interface::downloadPitValueShortArguments[kDownloadPitValueArgCount] = {
	"o"
};

// Dump arguments
string Interface::dumpValueArguments[kDumpValueArgCount] = {
	"-chip-type", "-chip-id", "-output"
};

string Interface::dumpValueShortArguments[kDumpValueArgCount] = {
	"type",       "id",       "out"
};

// Common arguments
string Interface::commonValueArguments[kCommonValueArgCount] = {
	"-delay"
};

string Interface::commonValueShortArguments[kCommonValueArgCount] = {
	"d"
};

string Interface::commonValuelessArguments[kCommonValuelessArgCount] = {
	"-verbose", "-no-reboot", "-stdout-errors"
};

string Interface::commonValuelessShortArguments[kCommonValuelessArgCount] = {
	"v",        "nobt",       "err"
};

Action Interface::actions[Interface::kActionCount] = {
	// kActionFlash
	Action("flash", flashValueArguments, flashValueShortArguments, kFlashValueArgCount,
		flashValuelessArguments, flashValuelessShortArguments, kFlashValuelessArgCount),

	// kActionClosePcScreen
	Action("close-pc-screen", nullptr, nullptr, kClosePcScreenValueArgCount,
		nullptr, nullptr, kClosePcScreenValuelessArgCount),

	// kActionDump
	Action("dump", dumpValueArguments, dumpValueShortArguments, kDumpValueArgCount,
		nullptr, nullptr, kDumpValuelessArgCount),

	// kActionPrintPit
	Action("print-pit", nullptr, nullptr, kPrintPitValueArgCount,
		nullptr, nullptr, kPrintPitValuelessArgCount),

	// kActionVersion
	Action("version", nullptr, nullptr, kVersionValueArgCount,
		nullptr, nullptr, kVersionValuelessArgCount),

	// kActionHelp
	Action("help", nullptr, nullptr, kHelpValueArgCount,
		nullptr, nullptr, kHelpValuelessArgCount),

	// kActionDetect
	Action("detect", nullptr, nullptr, kDetectValueArgCount,
		nullptr, nullptr, kDetectValuelessArgCount),

	// kActionDownloadPit
	Action("download-pit", downloadPitValueArguments, downloadPitValueShortArguments, kDownloadPitValueArgCount,
		nullptr, nullptr, kDownloadPitValuelessArgCount),

	// kActionInfo
	Action("info", nullptr, nullptr, kInfoValueArgCount,
		nullptr, nullptr, kInfoValuelessArgCount)
};

bool Interface::GetArguments(int argc, char **argv, map<string, string>& argumentMap, int *actionIndex)
{
	if (argc < 2)
	{
		Print(usage, version);
		return (false);
	}

	const char *actionName = argv[1];
	*actionIndex = -1;

	for (int i = 0; i < kActionCount; i++)
	{
		if (actions[i].name == actionName)
		{
			*actionIndex = i;
			break;
		}
	}

	if (*actionIndex < 0)
	{
		Print("Unknown action \"%s\"\n\n", actionName);
		Print(usage, version);
		return (false);
	}

	const Action& action = actions[*actionIndex]; 

	for (int argIndex = 2; argIndex < argc; argIndex++)
	{
		if (*(argv[argIndex]) != '-')
		{
			Print(usage, version);
			return (false);
		}

		string argumentName = (char *)(argv[argIndex] + 1);

		// Check if the argument is a valid valueless argument
		bool valid = false;

		for (unsigned int i = 0; i < action.valuelessArgumentCount; i++)
		{
			if (argumentName == action.valuelessArguments[i] || argumentName == action.valuelessShortArguments[i])
			{
				argumentName = action.valuelessArguments[i];
				valid = true;
				break;
			}
		}

		if (!valid)
		{
			// Check if it's a common valueless argument
			for (unsigned int i = 0; i < kCommonValuelessArgCount; i++)
			{
				if (argumentName == commonValuelessArguments[i] || argumentName == commonValuelessShortArguments[i])
				{
					argumentName = commonValuelessArguments[i];
					valid = true;
					break;
				}
			}
		}

		if (valid)
		{
			// The argument is valueless
			argumentMap.insert(pair<string, string>(argumentName, ""));
			continue;
		}

		// Check if the argument is a valid value argument
		for (unsigned int i = 0; i < action.valueArgumentCount; i++)
		{
			// Support for --<integer> and -<integer> parameters.
			if (argumentName.length() > 1 && action.valueArguments[i] == "-%d")
			{
				if (atoi(argumentName.substr(1).c_str()) > 0 || argumentName == "-0")
				{
					valid = true;
					break;
				}
			}
			else if (action.valueArguments[i] == "%d")
			{
				if (atoi(argumentName.c_str()) > 0 || argumentName == "0")
				{
					argumentName = "-" + argumentName;
					valid = true;
					break;
				}
			}

			if (argumentName == action.valueArguments[i] || argumentName == action.valueShortArguments[i])
			{
				argumentName = action.valueArguments[i];
				valid = true;
				break;
			}
		}

		if (!valid)
		{
			// Check if it's a common value argument
			for (unsigned int i = 0; i < kCommonValueArgCount; i++)
			{
				// Support for --<integer> and -<integer> parameters.
				if (argumentName.length() > 1 && commonValueArguments[i] == "-%d")
				{
					if (atoi(argumentName.substr(1).c_str()) > 0 || argumentName == "-0")
					{
						valid = true;
						break;
					}
				}
				else if (commonValueArguments[i] == "%d")
				{
					if (atoi(argumentName.c_str()) > 0 || argumentName == "0")
					{
						argumentName = "-" + argumentName;
						valid = true;
						break;
					}
				}

				if (argumentName == commonValueArguments[i] || argumentName == commonValueShortArguments[i])
				{
					argumentName = commonValueArguments[i];
					valid = true;
					break;
				}
			}
		}

		if (!valid)
		{
			PrintError("\"%s\" is not a valid argument\n", argumentName.c_str());
			return (false);
		}

		argIndex++;

		if (argIndex >= argc)
		{
			PrintError("\"%s\" is missing a value\n", argumentName.c_str());
			return (false);
		}

		argumentMap.insert(pair<string, string>(argumentName, argv[argIndex]));
	}

	return (true);
}

void Interface::Print(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	vfprintf(stdout, format, args);
	fflush(stdout);

	va_end(args);
	
}

void Interface::PrintError(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	fprintf(stderr, "ERROR: ");
	vfprintf(stderr, format, args);
	fflush(stderr);

	if (stdoutErrors)
	{
		fprintf(stdout, "ERROR: ");
		vfprintf(stdout, format, args);
		fflush(stdout);
	}

	va_end(args);
}

void Interface::PrintErrorSameLine(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	vfprintf(stderr, format, args);
	fflush(stderr);

	if (stdoutErrors)
	{
		vfprintf(stdout, format, args);
		fflush(stdout);
	}

	va_end(args);
}

void Interface::PrintVersion(void)
{
	Print("%s\n", version);
}

void Interface::PrintUsage(void)
{
	Print(usage);
}

void Interface::PrintReleaseInfo(void)
{
	Print(releaseInfo, version);
}

void Interface::PrintFullInfo(void)
{
	Print(releaseInfo, version);
	Print(extraInfo);
}

void Interface::PrintDeviceDetectionFailed(void)
{
	Print("Failed to detect compatible download-mode device.\n");
}

void Interface::PrintPit(const PitData *pitData)
{
	Interface::Print("Entry Count: %d\n", pitData->GetEntryCount());

	Interface::Print("Unknown 1: %d\n", pitData->GetUnknown1());
	Interface::Print("Unknown 2: %d\n", pitData->GetUnknown2());
	Interface::Print("Unknown 3: %d\n", pitData->GetUnknown3());
	Interface::Print("Unknown 4: %d\n", pitData->GetUnknown4());
	Interface::Print("Unknown 5: %d\n", pitData->GetUnknown5());
	Interface::Print("Unknown 6: %d\n", pitData->GetUnknown6());
	Interface::Print("Unknown 7: %d\n", pitData->GetUnknown7());
	Interface::Print("Unknown 8: %d\n", pitData->GetUnknown8());

	for (unsigned int i = 0; i < pitData->GetEntryCount(); i++)
	{
		const PitEntry *entry = pitData->GetEntry(i);

		Interface::Print("\n\n--- Entry #%d ---\n", i);
		Interface::Print("Unused: %s\n", (entry->GetUnused()) ? "Yes" : "No");

		const char *partitionTypeText = "Unknown";

		if (entry->GetPartitionType() == PitEntry::kPartitionTypeRfs)
			partitionTypeText = "RFS";
		else if (entry->GetPartitionType() == PitEntry::kPartitionTypeExt4)
			partitionTypeText = "EXT4";

		Interface::Print("Partition Type: %d (%s)\n", entry->GetPartitionType(), partitionTypeText);

		Interface::Print("Partition Identifier: %d\n", entry->GetPartitionIdentifier());

		Interface::Print("Partition Flags: %d (", entry->GetPartitionFlags());

		if (entry->GetPartitionFlags() & PitEntry::kPartitionFlagWrite)
			Interface::Print("R/W");
		else
			Interface::Print("R");

		Interface::Print(")\n");

		Interface::Print("Unknown 1: %d\n", entry->GetUnknown1());

		Interface::Print("Partition Block Size: %d\n", entry->GetPartitionBlockSize());
		Interface::Print("Partition Block Count: %d\n", entry->GetPartitionBlockCount());

		Interface::Print("Unknown 2: %d\n", entry->GetUnknown2());
		Interface::Print("Unknown 3: %d\n", entry->GetUnknown3());

		Interface::Print("Partition Name: %s\n", entry->GetPartitionName());
		Interface::Print("Filename: %s\n", entry->GetFilename());
	}

	Interface::Print("\n");
}
