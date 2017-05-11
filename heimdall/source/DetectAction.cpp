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

// Heimdall
#include "Arguments.h"
#include "BridgeManager.h"
#include "DetectAction.h"
#include "Heimdall.h"
#include "Interface.h"

using namespace std;
using namespace Heimdall;

const char *DetectAction::usage = "Action: detect\n\
Arguments: [--verbose] [--stdout-errors]\n\
           [--usb-log-level <none/error/warning/debug>]\n\
Description: Indicates whether or not a download mode device can be detected.\n";

int DetectAction::Execute(int argc, char **argv)
{
	// Handle arguments

	map<string, ArgumentType> argumentTypes;
	argumentTypes["verbose"] = kArgumentTypeFlag;
	argumentTypes["stdout-errors"] = kArgumentTypeFlag;
	argumentTypes["usb-log-level"] = kArgumentTypeString;

	Arguments arguments(argumentTypes);

	if (!arguments.ParseArguments(argc, argv, 2))
	{
		Interface::Print(DetectAction::usage);
		return (0);
	}

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
			Interface::Print(DetectAction::usage);
			return (0);
		}
	}

	// Download PIT file from device.

	BridgeManager *bridgeManager = new BridgeManager(verbose);
	bridgeManager->SetUsbLogLevel(usbLogLevel);

	bool detected = bridgeManager->DetectDevice();

	delete bridgeManager;

	return ((detected) ? 0 : 1);
}
