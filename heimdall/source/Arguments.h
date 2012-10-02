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

#ifndef ARGUMENTS_H
#define ARGUMENTS_H

// C/C++ Standard Library
#include <map>
#include <string>

// Heimdall
#include "Heimdall.h"

using namespace std;

namespace Heimdall
{
	typedef enum 
	{
		kArgumentTypeFlag = 0,
		kArgumentTypeString,
		kArgumentTypeUnsignedInteger
		
	} ArgumentType;

	class Argument
	{
		private:

			ArgumentType argumentType;

		protected:

			Argument(ArgumentType argumentType)
			{
				this->argumentType = argumentType;
			}

		public:

			virtual ~Argument()
			{
			}

			ArgumentType GetArgumentType(void) const
			{
				return argumentType;
			}
	};

	class FlagArgument : public Argument
	{
		private:
		
			FlagArgument() : Argument(kArgumentTypeFlag)
			{
			}

		public:

			static FlagArgument *ParseArgument(int argc, char **argv, int& argi);
	};

	class StringArgument : public Argument
	{
		private:

			string value;

			StringArgument(const string& value) : Argument(kArgumentTypeString)
			{
				this->value = value;
			}

		public:

			static StringArgument *ParseArgument(int argc, char **argv, int& argi);

			const string& GetValue(void) const
			{
				return (value);
			}
	};

	class UnsignedIntegerArgument : public Argument
	{
		private:

			unsigned int value;

			UnsignedIntegerArgument(unsigned int value) : Argument(kArgumentTypeUnsignedInteger)
			{
				this->value = value;
			}

		public:

			static UnsignedIntegerArgument *ParseArgument(int argc, char **argv, int& argi);

			unsigned int GetValue(void) const
			{
				return (value);
			}
	};

	class Arguments
	{
		private:
			
			const map<string, ArgumentType> argumentTypes;
			const map<string, string> shortArgumentAliases;
			const map<string, string> argumentAliases;

			map<string, Argument *> arguments;

		public:

			Arguments(const map<string, ArgumentType>& argumentTypes, const map<string, string>& shortArgumentAliases = (map<string, string>()),
				const map<string, string>& argumentAliases = (map<string, string>()));
			
			~Arguments();

			// argi is the index of the first argument to parse.
			bool ParseArguments(int argc, char **argv, int argi);

			const Argument *GetArgument(string argumentName) const
			{
				map<string, Argument *>::const_iterator it = arguments.find(argumentName);
				return (it != arguments.end() ? it->second : nullptr);
			}

			const map<string, ArgumentType>& GetArgumentTypes(void) const
			{
				return (argumentTypes);
			}

			const map<string, Argument *>& GetArguments(void) const
			{
				return (arguments);
			}
	};
}

#endif
