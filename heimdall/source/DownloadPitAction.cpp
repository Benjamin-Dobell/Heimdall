/* Copyright (c) 2010-2017 Benjamin Dobell, Glass Echidna
 
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
#include "DownloadPitAction.h"
#include "Heimdall.h"
#include "Interface.h"

using namespace std;
using namespace Heimdall;

const char *DownloadPitAction::usage = "Action: download-pit\n\
Arguments: --output <filename> [--verbose] [--no-reboot] [--stdout-errors]\n\
    [--usb-log-level <none/error/warning/debug>]\n\
Description: Downloads the connected device's PIT file to the specified\n\
    output file.\n\
Note: --no-reboot causes the device to remain in download mode after the action\n\
      is completed. If you wish to perform another action whilst remaining in\n\
      download mode, then the following action must specify the --resume flag.\n";

int DownloadPitAction::Execute(int argc, char **argv)
{
	// Handle arguments

	map<string, ArgumentType> argumentTypes;
	argumentTypes["output"] = kArgumentTypeString;
	argumentTypes["no-reboot"] = kArgumentTypeFlag;
	argumentTypes["resume"] = kArgumentTypeFlag;
	argumentTypes["verbose"] = kArgumentTypeFlag;
	argumentTypes["stdout-errors"] = kArgumentTypeFlag;
	argumentTypes["usb-log-level"] = kArgumentTypeString;

	Arguments arguments(argumentTypes);

	if (!arguments.ParseArguments(argc, argv, 2))
	{
		Interface::Print(DownloadPitAction::usage);
		return (0);
	}

	const StringArgument *outputArgument = static_cast<const StringArgument *>(arguments.GetArgument("output"));

	if (!outputArgument)
	{
		Interface::Print("Output file was not specified.\n\n");
		Interface::Print(DownloadPitAction::usage);
		return (0);
	}

	bool reboot = arguments.GetArgument("no-reboot") == nullptr;
	bool resume = arguments.GetArgument("resume") != nullptr;
	bool verbose = arguments.GetArgument("verbose") != nullptr;
	
	if (arguments.GetArgument("stdout-errors") != nullptr)
		Interface::SetStdoutErrors(true);

	const StringArgument *usbLogLevelArgument = static_cast<const StringArgument *>(arguments.GetArgument("usb-log-level"));

	BridgeManager::UsbLogLevel usbLogLevel = BridgeManager::UsbLogLevel::Default;

	if (usbLogLevelArgument)
	{
		const string& usbLogLevelString = usbLogLevelArgument->GetValue();

		if (usbLogLevelString.compare("none") == 0 || usbLogLevelString.compare("NONE") == 0)
		{
			usbLogLevel = BridgeManager::UsbLogLevel::None;
		}
		else if (usbLogLevelString.compare("error") == 0 || usbLogLevelString.compare("ERROR") == 0)
		{
			usbLogLevel = BridgeManager::UsbLogLevel::Error;
		}
		else if (usbLogLevelString.compare("warning") == 0 || usbLogLevelString.compare("WARNING") == 0)
		{
			usbLogLevel = BridgeManager::UsbLogLevel::Warning;
		}
		else if (usbLogLevelString.compare("info") == 0 || usbLogLevelString.compare("INFO") == 0)
		{
			usbLogLevel = BridgeManager::UsbLogLevel::Info;
		}
		else if (usbLogLevelString.compare("debug") == 0 || usbLogLevelString.compare("DEBUG") == 0)
		{
			usbLogLevel = BridgeManager::UsbLogLevel::Debug;
		}
		else
		{
			Interface::Print("Unknown USB log level: %s\n\n", usbLogLevelString.c_str());
			Interface::Print(DownloadPitAction::usage);
			return (0);
		}
	}

	// Info

	Interface::PrintReleaseInfo();
	Sleep(1000);

	// Open output file

	const char *outputFilename = outputArgument->GetValue().c_str();
	FILE *outputPitFile = FileOpen(outputFilename, "wb");

	if (!outputPitFile)
	{
		Interface::PrintError("Failed to open output file \"%s\"\n", outputFilename);
		return (1);
	}

	// Download PIT file from device.

	BridgeManager *bridgeManager = new BridgeManager(verbose);
	bridgeManager->SetUsbLogLevel(usbLogLevel);

	if (bridgeManager->Initialise(resume) != BridgeManager::kInitialiseSucceeded || !bridgeManager->BeginSession())
	{
		FileClose(outputPitFile);
		delete bridgeManager;

		return (1);
	}

	unsigned char *pitBuffer;
	int fileSize = bridgeManager->DownloadPitFile(&pitBuffer);

	bool success = true;

	if (fileSize > 0)
	{
		if (fwrite(pitBuffer, 1, fileSize, outputPitFile) != fileSize)
		{
			Interface::PrintError("Failed to write PIT data to output file.\n");
			success = false;
		}
	}
	else
	{
		success = false;
	}

	if (!bridgeManager->EndSession(reboot))
		success = false;

	delete bridgeManager;
	
	FileClose(outputPitFile);
	delete [] pitBuffer;

	return (success ? 0 : 1);
}
