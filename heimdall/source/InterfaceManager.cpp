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
#include <cstdarg>
#include <cstdlib>
#include <stdio.h>

// Heimdall
#include "Heimdall.h"
#include "InterfaceManager.h"

using namespace std;
using namespace Heimdall;

string InterfaceManager::actionNames[kActionCount] = { "flash", "close-pc-screen", "dump", "print-pit", "help" };

string InterfaceManager::flashArgumentNames[kFlashArgCount * 2] = {
	// --- Long Names ---
	"-repartition",

	"-pit", "-factoryfs", "-cache", "-dbdata", "-primary-boot",	"-secondary-boot", "-secondary-boot-backup", "-param", "-kernel", "-recovery", "-efs", "-modem",
	"-normal-boot", "-system", "-user-data", "-fota", "-hidden", "-movinand", "-data", "-ums", "-emmc", "-%d",

	// --- Short Names ---
	"r",

	"pit",  "fs",         "cache",  "db",      "boot",           "sbl",            "sbl2",                   "param",  "z",       "rec",       "efs",  "m",
	"norm",         "sys",     "udata",      "fota",  "hide",    "nand",      "data",  "ums",  "emmc",  "%d"
};

string InterfaceManager::dumpArgumentNames[kDumpArgCount * 2] = {
	// --- Long Names ---
	"-chip-type", "-chip-id", "-output",

	// --- Short Names ---
	"type",       "id",       "out"
};

string InterfaceManager::commonArgumentNames[kCommonArgCount * 2] = {
	// --- Long Names ---
	"-verbose", "-no-reboot",

	"-delay",

	// --- Short Names ---
	"v",        "nobt",

	"d"
};

// argumentNames[kActionCount][] stores common arguments. 
string *InterfaceManager::actionArgumentNames[kActionCount + 1] = {
	// kActionFlash
	flashArgumentNames,

	// kActionClosePcScreen
	nullptr,

	// kActionDump
	dumpArgumentNames,

	// kActionPrintPit
	nullptr,

	// kActionHelp
	nullptr,

	// Common (kActionCount)
	commonArgumentNames
};

int InterfaceManager::actionArgumentCounts[kActionCount + 1] = {
	kFlashArgCount, 0, kDumpArgCount, 0, 0, kCommonArgCount
};

int InterfaceManager::actionValuelessArgumentCounts[kActionCount + 1] = {
	kFlashArgPit, 0, kDumpArgChipType, 0, 0, kCommonArgDelay
};

const char *InterfaceManager::usage = "\nHeimdall v1.2.0, Copyright (c) 2010-2011, Benjamin Dobell, Glass Echidna\n\n\
Usage: heimdall <action> <arguments> [--verbose] [--no-reboot] [--delay <ms>]\n\
\n\
action: flash\n\
arguments:\n\
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
description: Flashes firmware files to your phone.\n\
WARNING: If you're repartitioning it's strongly recommended you specify\n\
         all files at your disposal, including bootloaders.\n\
\n\
action: close-pc-screen\n\
description: Attempts to get rid off the \"connect phone to PC\" screen.\n\
\n\
action: dump\n\
arguments: --chip-type <NAND | RAM> --chip-id <integer> --output <filename>\n\
description: Attempts to dump data from the phone corresponding to the\n\
	specified chip type and chip ID.\n\
NOTE: Galaxy S phones don't appear to properly support this functionality.\n\
\n\
action: print-pit\n\
description: Dumps the PIT file from the connected device and prints it in\n\
    a human readable format.\n\
\n\
action: help\n\
description: Display this dialogue.\n";

bool InterfaceManager::GetArguments(int argc, char **argv, map<string, string>& argumentMap, int *actionIndex)
{
	if (argc < 2)
	{
		Print(usage);
		return (false);
	}

	const char *actionName = argv[1];
	*actionIndex = -1;

	for (int i = 0; i < kActionCount; i++)
	{
		if (actionNames[i] == actionName)
		{
			*actionIndex = i;
			break;
		}
	}

	if (*actionIndex < 0)
	{
		Print("Unknown action \"%s\"\n\n", actionName);
		Print(usage);
		return (false);
	}

	int actionArgumentCount = actionArgumentCounts[*actionIndex];
	int commonArgumentCount = actionArgumentCounts[kActionCount];

	int actionValuelessArgumentCount = actionValuelessArgumentCounts[*actionIndex];
	int commonValuelessArgumentCount = actionValuelessArgumentCounts[kActionCount];

	string *argumentNames = actionArgumentNames[*actionIndex];
	string *commonArgumentNames = actionArgumentNames[kActionCount];

	for (int argIndex = 2; argIndex < argc; argIndex++)
	{
		if (*(argv[argIndex]) != '-')
		{
			Print(usage);
			return (false);
		}

		string argumentName = (char *)(argv[argIndex] + 1);

		// Check if the argument is a valid valueless argument
		bool valid = false;

		for (int i = 0; i < actionValuelessArgumentCount; i++)
		{
			if (argumentName == argumentNames[i] || argumentName == argumentNames[actionArgumentCount + i])
			{
				argumentName = argumentNames[i];
				valid = true;
				break;
			}
		}

		if (!valid)
		{
			// Check if it's a common valueless argument
			for (int i = 0; i < commonValuelessArgumentCount; i++)
			{
				if (argumentName == commonArgumentNames[i] || argumentName == commonArgumentNames[commonArgumentCount + i])
				{
					argumentName = commonArgumentNames[i];
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

		if (argumentNames != nullptr)
		{
			// Check if the argument is a valid regular argument
			for (int i = actionValuelessArgumentCount; i < actionArgumentCount; i++)
			{
				// Support for --<integer> and -<integer> parameters.
				if (argumentName.length() > 1 && argumentNames[i].compare("-%d") == 0)
				{
					if (atoi(argumentName.substr(1).c_str()) > 0 || argumentName.compare("-0") == 0)
					{
						valid = true;
						break;
					}
				}
				else if (argumentNames[i].compare("%d") == 0)
				{
					if (atoi(argumentName.c_str()) > 0 || argumentName.compare("0") == 0)
					{
						argumentName = "-" + argumentName;
						valid = true;
						break;
					}
				}

				if (argumentName == argumentNames[i] || argumentName == argumentNames[actionArgumentCount + i])
				{
					argumentName = argumentNames[i];
					valid = true;
					break;
				}
			}
		}

		if (!valid)
		{
			// Check if it's a common regular argument
			for (int i = commonValuelessArgumentCount; i < commonArgumentCount; i++)
			{
				// Support for --<integer> and -<integer> parameters.
				if (argumentName.length() > 1 && commonArgumentNames[i].compare("-%d"))
				{
					if (atoi(argumentName.substr(1).c_str()) > 0 || argumentName.compare("-0") == 0)
					{
						valid = true;
						break;
					}
				}
				else if (commonArgumentNames[i].compare("%d"))
				{
					if (atoi(argumentName.c_str()) > 0 || argumentName.compare("0") == 0)
					{
						argumentName = "-" + argumentName;
						valid = true;
						break;
					}
				}

				if (argumentName == commonArgumentNames[i] || argumentName == commonArgumentNames[commonArgumentCount + i])
				{
					argumentName = commonArgumentNames[i];
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

void InterfaceManager::Print(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);

	fflush(stdout);	// Make sure output isn't buffered.
}

void InterfaceManager::PrintError(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fflush(stderr);	// Make sure output isn't buffered.
}
