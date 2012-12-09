/* Copyright (c) 2012 Steve Langasek <vorlon@debian.org>
 
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
#include "EndSessionPacket.h"
#include "ResponsePacket.h"
#include "RebootAction.h"
#include "Heimdall.h"
#include "Interface.h"

using namespace Heimdall;

const char *RebootAction::usage = "Action: reboot\n\
Arguments: [--verbose] [--stdout-errors] [--delay <ms>]\n\
Description: Reboot the phone.\n";

int RebootAction::Execute(int argc, char **argv)
{
	// Handle arguments

	map<string, ArgumentType> argumentTypes;
	argumentTypes["delay"] = kArgumentTypeUnsignedInteger;
	argumentTypes["verbose"] = kArgumentTypeFlag;
	argumentTypes["stdout-errors"] = kArgumentTypeFlag;

	Arguments arguments(argumentTypes);

	if (!arguments.ParseArguments(argc, argv, 2))
	{
		Interface::Print(RebootAction::usage);
		return (0);
	}

	const UnsignedIntegerArgument *communicationDelayArgument = static_cast<const UnsignedIntegerArgument *>(arguments.GetArgument("delay"));

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

	if (bridgeManager->Initialise() != BridgeManager::kInitialiseSucceeded)
	{
		delete bridgeManager;
		return (1);
	}

	Interface::Print("Attempting to reboot...\n");

	EndSessionPacket *rebootDevicePacket = new EndSessionPacket(EndSessionPacket::kRequestRebootDevice);
	bool success = bridgeManager->SendPacket(rebootDevicePacket);
	delete rebootDevicePacket;

	if (!success)
	{
		Interface::PrintError("Failed to send reboot device packet!\n");

		delete bridgeManager;
		return (1);
	}

	ResponsePacket *rebootDeviceResponse = new ResponsePacket(ResponsePacket::kResponseTypeEndSession);
	success = bridgeManager->ReceivePacket(rebootDeviceResponse);
	delete rebootDeviceResponse;
	delete bridgeManager;

	if (!success)
	{
		Interface::PrintError("Failed to receive reboot confirmation!\n");
	} else {
		Interface::Print("Attempt complete\n");
	}

	return (!success);
}
