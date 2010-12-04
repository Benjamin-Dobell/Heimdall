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

#ifndef INTERFACEMANAGER_H
#define INTERFACEMANAGER_H

// C/C++ Standard Library
#include <map>
#include <string>

using namespace std;

namespace Heimdall
{
	class InterfaceManager
	{
		public:

			enum
			{
				kActionFlash = 0,
				kActionClosePcScreen,
				kActionDump,
				kActionHelp,
				kActionCount
			};

			enum
			{
				// Valueless arguments
				kFlashArgRepartition = 0,

				// Regular arguments
				kFlashArgPit,
				kFlashArgFactoryFs,
				kFlashArgCache,
				kFlashArgData,
				kFlashArgPrimaryBootloader,
				kFlashArgSecondaryBootloader,
				kFlashArgSecondaryBootloaderBackup,
				kFlashArgParam,
				kFlashArgKernel,
				kFlashArgRecovery,
				kFlashArgEfs,
				kFlashArgModem,

				kFlashArgCount
			};

			enum
			{
				// Regular arguments
				kDumpArgChipType = 0,
				kDumpArgChipId,
				kDumpArgOutput,

				kDumpArgCount
			};

			enum
			{
				// Valueless arguments
				kCommonArgVerbose = 0,

				// Regular arguments
				kCommonArgDelay,

				kCommonArgCount
			};

			static string actionNames[kActionCount];

			static string flashArgumentNames[kFlashArgCount * 2];
			static string dumpArgumentNames[kDumpArgCount * 2];
			static string commonArgumentNames[kCommonArgCount * 2];

			// argumentNames[kActionCount][] defines common arguments. 
			static string *actionArgumentNames[kActionCount + 1];

			static int actionArgumentCounts[kActionCount + 1];
			static int actionValuelessArgumentCounts[kActionCount + 1];

			static const char *usage;

			static bool GetArguments(int argc, char **argv, map<string, string>& argumentMap, int *actionIndex);

			static void Print(const char *format, ...);
			static void PrintError(const char *format, ...);
	};
}

#endif
