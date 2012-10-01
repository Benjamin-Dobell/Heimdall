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
#include "Heimdall.h"
#include "Interface.h"
#include "PrintPitAction.h"

using namespace Heimdall;

const char *PrintPitAction::usage = "Action: print-pit\n\
Arguments: [--file <filename>] [--verbose] [--no-reboot] [--stdout-errors]\n\
    [--delay <ms>]\n\
Description: Prints the contents of a PIT file in a human readable format. If\n\
    a filename is not provided then Heimdall retrieves the PIT file from the \n\
    connected device.\n";

int PrintPitAction::Execute(int argc, char **argv)
{
	// Handle arguments

	map<string, ArgumentType> argumentTypes;
	argumentTypes["file"] = kArgumentTypeString;
	argumentTypes["no-reboot"] = kArgumentTypeFlag;
	argumentTypes["delay"] = kArgumentTypeUnsignedInteger;
	argumentTypes["verbose"] = kArgumentTypeFlag;
	argumentTypes["stdout-errors"] = kArgumentTypeFlag;

	Arguments arguments(argumentTypes);

	if (!arguments.ParseArguments(argc, argv, 2))
	{
		Interface::Print(PrintPitAction::usage);
		return (0);
	}

	const StringArgument *fileArgument = static_cast<const StringArgument *>(arguments.GetArgument("file"));
	const UnsignedIntegerArgument *communicationDelayArgument = static_cast<const UnsignedIntegerArgument *>(arguments.GetArgument("delay"));

	bool reboot = arguments.GetArgument("no-reboot") == nullptr;
	bool verbose = arguments.GetArgument("verbose") != nullptr;
	
	if (arguments.GetArgument("stdout-errors") != nullptr)
		Interface::SetStdoutErrors(true);

	// Open file (if specified).

	FILE *localPitFile = nullptr;

	if (fileArgument)
	{
		const char *filename = fileArgument->GetValue().c_str();

		localPitFile = fopen(filename, "rb");

		if (!localPitFile)
		{
			Interface::PrintError("Failed to open file \"%s\"\n", filename);
			return (1);
		}
	}

	// Info

	Interface::PrintReleaseInfo();
	Sleep(1000);

	if (localPitFile)
	{
		// Print PIT from file; there's no need for a BridgeManager.

		fseek(localPitFile, 0, SEEK_END);
		long localPitFileSize = ftell(localPitFile);
		rewind(localPitFile);

		// Load the local pit file into memory.
		unsigned char *pitFileBuffer = new unsigned char[localPitFileSize];
		size_t dataRead = fread(pitFileBuffer, 1, localPitFileSize, localPitFile); // dataRead is discarded, it's here to remove warnings.
		fclose(localPitFile);

		PitData *pitData = new PitData();
		pitData->Unpack(pitFileBuffer);

		delete [] pitFileBuffer;

		Interface::PrintPit(pitData);
		delete pitData;

		return (0);
	}
	else
	{
		// Print PIT from a device.

		int communicationDelay = BridgeManager::kCommunicationDelayDefault;

		if (communicationDelayArgument)
			communicationDelay = communicationDelayArgument->GetValue();

		BridgeManager *bridgeManager = new BridgeManager(verbose, communicationDelay);

		if (bridgeManager->Initialise() != BridgeManager::kInitialiseSucceeded || !bridgeManager->BeginSession())
		{
			delete bridgeManager;
			return (1);
		}
		
		unsigned char *devicePit;
		bool success = bridgeManager->DownloadPitFile(&devicePit) != 0;

		if (success)
		{
			PitData *pitData = new PitData();

			if (pitData->Unpack(devicePit))
			{
				Interface::PrintPit(pitData);
			}
			else
			{
				Interface::PrintError("Failed to unpack device's PIT file!\n");
				success = false;
			}

			delete pitData;
		}
			
		delete [] devicePit;

		if (!bridgeManager->EndSession(reboot))
			success = false;

		delete bridgeManager;

		return (success ? 0 : 1);
	}
}
