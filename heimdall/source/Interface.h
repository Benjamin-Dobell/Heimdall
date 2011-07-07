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

#ifndef INTERFACE_H
#define INTERFACE_H

// C/C++ Standard Library
#include <map>
#include <string>

// libpit
#include "libpit.h"

using namespace std;
using namespace libpit;

namespace Heimdall
{
	struct Action
	{
		public:

			string name;
			
			string *valueArguments;
			string *valueShortArguments;
			unsigned int valueArgumentCount;

			string *valuelessArguments;
			string *valuelessShortArguments;
			unsigned int valuelessArgumentCount;

			Action(const char *name, string *valueArguments, string *valueShortArguments, unsigned int valueArgumentCount,
				string *valuelessArguments, string *valuelessShortArguments, unsigned int valuelessArgumentCount)
			{
				this->name = name;

				this->valueArguments = valueArguments;
				this->valueShortArguments = valueShortArguments;
				this->valueArgumentCount = valueArgumentCount;

				this->valuelessArguments = valuelessArguments;
				this->valuelessShortArguments = valuelessShortArguments;
				this->valuelessArgumentCount = valuelessArgumentCount;
			}
	};

	class Interface
	{
		public:

			// Actions
			enum
			{
				kActionFlash = 0,
				kActionClosePcScreen,
				kActionDump,
				kActionPrintPit,
				kActionVersion,
				kActionHelp,
				kActionDetect,
				kActionCount
			};

			// Flash value arguments
			enum
			{
				kFlashValueArgPit,
				kFlashValueArgFactoryFs,
				kFlashValueArgCache,
				kFlashValueArgDatabaseData,
				kFlashValueArgPrimaryBootloader,
				kFlashValueArgSecondaryBootloader,
				kFlashValueArgSecondaryBootloaderBackup,
				kFlashValueArgParam,
				kFlashValueArgKernel,
				kFlashValueArgRecovery,
				kFlashValueArgEfs,
				kFlashValueArgModem,

				kFlashValueArgNormalBoot,
				kFlashValueArgSystem,
				kFlashValueArgUserData,
				kFlashValueArgFota,
				kFlashValueArgHidden,
				kFlashValueArgMovinand,
				kFlashValueArgData,
				kFlashValueArgUms,
				kFlashValueArgEmmc,

				kFlashValueArgPartitionIndex,

				kFlashValueArgCount
			};

			// Flash valueless arguments
			enum
			{
				kFlashValuelessArgRepartition = 0,

				kFlashValuelessArgCount
			};

			// Close PC Screen value arguments
			enum
			{
				kClosePcScreenValueArgCount = 0
			};

			// Close PC Screen valueless arguments
			enum
			{
				kClosePcScreenValuelessArgCount = 0
			};

			// Dump value arguments
			enum
			{
				kDumpValueArgChipType = 0,
				kDumpValueArgChipId,
				kDumpValueArgOutput,

				kDumpValueArgCount
			};

			// Dump valueless arguments
			enum
			{
				kDumpValuelessArgCount = 0
			};

			// Print PIT value arguments
			enum
			{
				kPrintPitValueArgCount = 0
			};

			// Print PIT valueless arguments
			enum
			{
				kPrintPitValuelessArgCount = 0
			};

			// Version value arguments
			enum
			{
				kVersionValueArgCount = 0
			};

			// Version valueless arguments
			enum
			{
				kVersionValuelessArgCount = 0
			};

			// Help value arguments
			enum
			{
				kHelpValueArgCount = 0
			};

			// Help valueless arguments
			enum
			{
				kHelpValuelessArgCount = 0
			};

			// Detect value arguments
			enum
			{
				kDetectValueArgCount = 0
			};

			// Detect valueless arguments
			enum
			{
				kDetectValuelessArgCount = 0
			};

			// Common value arguments
			enum
			{
				kCommonValueArgDelay = 0,

				kCommonValueArgCount
			};

			// Comon valueless arguments
			enum
			{
				kCommonValuelessArgVerbose = 0,
				kCommonValuelessArgNoReboot,

				kCommonValuelessArgCount
			};

		private:
		
			static const char *version;
			static const char *usage;
			static const char *releaseInfo;			

			// Flash arguments
			static string flashValueArguments[kFlashValueArgCount];
			static string flashValueShortArguments[kFlashValueArgCount];

			static string flashValuelessArguments[kFlashValuelessArgCount];
			static string flashValuelessShortArguments[kFlashValuelessArgCount];

			// Dump arguments
			static string dumpValueArguments[kDumpValueArgCount];
			static string dumpValueShortArguments[kDumpValueArgCount];

		public:

			// Common arguments
			static string commonValueArguments[kCommonValueArgCount];
			static string commonValueShortArguments[kCommonValueArgCount];

			static string commonValuelessArguments[kCommonValuelessArgCount];
			static string commonValuelessShortArguments[kCommonValuelessArgCount];

			static Action actions[kActionCount];

			static bool GetArguments(int argc, char **argv, map<string, string>& argumentMap, int *actionIndex);

			static void Print(const char *format, ...);
			static void PrintError(const char *format, ...);

			static void PrintVersion(void);
			static void PrintUsage(void);
			static void PrintReleaseInfo(void);

			static void PrintPit(const PitData *pitData);

			static string& GetPitArgument(void)
			{
				return (flashValueArguments[kFlashValueArgPit]);
			}
	};
}

#endif
