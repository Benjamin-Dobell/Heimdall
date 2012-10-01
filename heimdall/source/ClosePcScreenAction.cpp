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

// Heimdall
#include "Arguments.h"
#include "BridgeManager.h"
#include "ClosePcScreenAction.h"
#include "Heimdall.h"
#include "Interface.h"

using namespace Heimdall;

const char *ClosePcScreenAction::usage = "Action: close-pc-screen\n\
Arguments: [--verbose] [--no-reboot] [--stdout-errors] [--delay <ms>]\n\
Description: Attempts to get rid off the \"connect phone to PC\" screen.\n";

int ClosePcScreenAction::Execute(int argc, char **argv)
{
	// Handle arguments

	map<string, ArgumentType> argumentTypes;
	argumentTypes["no-reboot"] = kArgumentTypeFlag;
	argumentTypes["delay"] = kArgumentTypeUnsignedInteger;
	argumentTypes["verbose"] = kArgumentTypeFlag;
	argumentTypes["stdout-errors"] = kArgumentTypeFlag;

	Arguments arguments(argumentTypes);

	if (!arguments.ParseArguments(argc, argv, 2))
	{
		Interface::Print(ClosePcScreenAction::usage);
		return (0);
	}

	const UnsignedIntegerArgument *communicationDelayArgument = static_cast<const UnsignedIntegerArgument *>(arguments.GetArgument("delay"));

	bool reboot = arguments.GetArgument("no-reboot") == nullptr;
	bool verbose = arguments.GetArgument("verbose") != nullptr;
	
	if (arguments.GetArgument("stdout-errors") != nullptr)
		Interface::SetStdoutErrors(true);

	// Info

	Interface::PrintReleaseInfo();
	Sleep(1000);

	// Download PIT file from device.

	int communicationDelay = BridgeManager::kCommunicationDelayDefault;

	if (communicationDelayArgument)
		communicationDelay = communicationDelayArgument->GetValue();

	BridgeManager *bridgeManager = new BridgeManager(verbose, communicationDelay);

	if (bridgeManager->Initialise() != BridgeManager::kInitialiseSucceeded || !bridgeManager->BeginSession())
	{
		delete bridgeManager;
		return (1);
	}

	Interface::Print("Attempting to close connect to pc screen...\n");

	bool success = bridgeManager->EndSession(reboot);
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
