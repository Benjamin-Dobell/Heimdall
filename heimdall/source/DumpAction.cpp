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
#include "DumpAction.h"
#include "Heimdall.h"
#include "Interface.h"

using namespace Heimdall;

const char *DumpAction::usage = "Action: dump\n\
Arguments: --chip-type <NAND | RAM> --chip-id <integer> --output <filename>\n\
    [--verbose] [--no-reboot] [--stdout-errors] [--delay <ms>]\n\
Description: Attempts to dump data from the phone corresponding to the\n\
    specified chip type and chip ID.\n\
NOTE: Galaxy S phones don't appear to properly support this functionality.\n";

int DumpAction::Execute(int argc, char **argv)
{
	// Handle arguments

	map<string, ArgumentType> argumentTypes;

	argumentTypes["chip-type"] = kArgumentTypeString;
	argumentTypes["chip-id"] = kArgumentTypeUnsignedInteger;
	argumentTypes["output"] = kArgumentTypeString;

	argumentTypes["no-reboot"] = kArgumentTypeFlag;
	argumentTypes["delay"] = kArgumentTypeUnsignedInteger;
	argumentTypes["verbose"] = kArgumentTypeFlag;
	argumentTypes["stdout-errors"] = kArgumentTypeFlag;

	Arguments arguments(argumentTypes);

	if (!arguments.ParseArguments(argc, argv, 2))
	{
		Interface::Print(DumpAction::usage);
		return (0);
	}

	const StringArgument *chipTypeArgument = static_cast<const StringArgument *>(arguments.GetArgument("chip-type"));
	const UnsignedIntegerArgument *chipIdArgument = static_cast<const UnsignedIntegerArgument *>(arguments.GetArgument("chip-id"));
	const StringArgument *outputArgument = static_cast<const StringArgument *>(arguments.GetArgument("output"));

	if (!outputArgument)
	{
		Interface::Print("Output file was not specified.\n\n");
		Interface::Print(DumpAction::usage);
		return (false);
	}

	if (!chipTypeArgument)
	{
		Interface::Print("You must specify a chip type.\n\n");
		Interface::Print(DumpAction::usage);
		return (false);
	}

	if (!(chipTypeArgument->GetValue() == "RAM" || chipTypeArgument->GetValue() == "ram" || chipTypeArgument->GetValue() == "NAND"
		|| chipTypeArgument->GetValue() == "nand"))
	{
		Interface::Print("Unknown chip type: %s.\n\n", chipTypeArgument->GetValue().c_str());
		Interface::Print(DumpAction::usage);
		return (false);
	}

	if (!chipIdArgument)
	{
		Interface::Print("You must specify a chip ID.\n\n");
		Interface::Print(DumpAction::usage);
		return (false);
	}

	const UnsignedIntegerArgument *communicationDelayArgument = static_cast<const UnsignedIntegerArgument *>(arguments.GetArgument("delay"));

	bool reboot = arguments.GetArgument("no-reboot") == nullptr;
	bool verbose = arguments.GetArgument("verbose") != nullptr;
	
	if (arguments.GetArgument("stdout-errors") != nullptr)
		Interface::SetStdoutErrors(true);

	// Open output file

	const char *outputFilename = outputArgument->GetValue().c_str();
	FILE *dumpFile = fopen(outputFilename, "wb");

	if (!dumpFile)
	{
		Interface::PrintError("Failed to open file \"%s\"\n", outputFilename);
		return (1);
	}

	// Info

	Interface::PrintReleaseInfo();
	Sleep(1000);

	// Dump

	int communicationDelay = BridgeManager::kCommunicationDelayDefault;

	if (communicationDelayArgument)
		communicationDelay = communicationDelayArgument->GetValue();

	BridgeManager *bridgeManager = new BridgeManager(verbose, communicationDelay);

	if (bridgeManager->Initialise() != BridgeManager::kInitialiseSucceeded || !bridgeManager->BeginSession())
	{
		fclose(dumpFile);
		delete bridgeManager;
		return (1);
	}

	int chipType = 0;
			
	if (chipTypeArgument->GetValue() == "NAND" || chipTypeArgument->GetValue() == "nand")
		chipType = 1;

	bool success = bridgeManager->ReceiveDump(chipType, chipIdArgument->GetValue(), dumpFile);
	fclose(dumpFile);

	if (!bridgeManager->EndSession(reboot))
		success = false;

	delete bridgeManager;

	if (success)
	{
		Interface::Print("Attempt complete\n");
		return (0);
	}
	else
	{
		return (1);
	}
}
