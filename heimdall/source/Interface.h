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

#ifndef INTERFACE_H
#define INTERFACE_H

// C/C++ Standard Library
#include <map>
#include <string>

// libpit
#include "libpit.h"

// Heimdall
#include "Heimdall.h"

namespace Heimdall
{
	namespace Interface
	{
		typedef int (*ActionExecuteFunction)(int, char **);

		typedef struct ActionInfo
		{
			ActionExecuteFunction executeFunction;
			const char *usage;

			ActionInfo()
			{
				executeFunction = nullptr;
				usage = nullptr;
			}

			ActionInfo(ActionExecuteFunction executeFunction, const char *usage)
			{
				this->executeFunction = executeFunction;
				this->usage = usage;
			}

		} ActionInfo;

		const std::map<std::string, ActionInfo>& GetActionMap(void);

		void Print(const char *format, ...);
		void PrintWarning(const char *format, ...);
		void PrintWarningSameLine(const char *format, ...);
		void PrintError(const char *format, ...);
		void PrintErrorSameLine(const char *format, ...);

		void PrintVersion(void);
		void PrintUsage(void);
		void PrintReleaseInfo(void);
		void PrintFullInfo(void);

		void PrintDeviceDetectionFailed(void);

		void PrintPit(const libpit::PitData *pitData);

		void SetStdoutErrors(bool enabled);
	}
}

#endif
