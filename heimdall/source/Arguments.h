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

#ifndef ARGUMENTS_H
#define ARGUMENTS_H

// C/C++ Standard Library
#include <map>
#include <string>
#include <vector>

// Heimdall
#include "Heimdall.h"

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

			std::string name;
			ArgumentType type;

		protected:

			Argument(const std::string& name, ArgumentType type)
			{
				this->name = name;
				this->type = type;
			}

		public:

			virtual ~Argument()
			{
			}

			const std::string& GetName(void) const
			{
				return name;
			}

			ArgumentType GetType(void) const
			{
				return type;
			}
	};

	class FlagArgument : public Argument
	{
		private:
		
			FlagArgument(const std::string& name) : Argument(name, kArgumentTypeFlag)
			{
			}

		public:

			static FlagArgument *ParseArgument(const std::string& name, int argc, char **argv, int& argi);
	};

	class StringArgument : public Argument
	{
		private:

			std::string value;

			StringArgument(const std::string& name, const std::string& value) : Argument(name, kArgumentTypeString)
			{
				this->value = value;
			}

		public:

			static StringArgument *ParseArgument(const std::string& name, int argc, char **argv, int& argi);

			const std::string& GetValue(void) const
			{
				return (value);
			}
	};

	class UnsignedIntegerArgument : public Argument
	{
		private:

			unsigned int value;

			UnsignedIntegerArgument(const std::string& name, unsigned int value) : Argument(name, kArgumentTypeUnsignedInteger)
			{
				this->value = value;
			}

		public:

			static UnsignedIntegerArgument *ParseArgument(const std::string& name, int argc, char **argv, int& argi);

			unsigned int GetValue(void) const
			{
				return (value);
			}
	};

	class Arguments
	{
		private:
			
			const std::map<std::string, ArgumentType> argumentTypes;
			const std::map<std::string, std::string> shortArgumentAliases;
			const std::map<std::string, std::string> argumentAliases;

			std::vector<const Argument *> argumentVector;
			std::map<std::string, const Argument *> argumentMap;

		public:

			Arguments(const std::map<std::string, ArgumentType>& argumentTypes,
				const std::map<std::string, std::string>& shortArgumentAliases = (std::map<std::string, std::string>()),
				const std::map<std::string, std::string>& argumentAliases = (std::map<std::string, std::string>()));
			
			~Arguments();

			// argi is the index of the first argument to parse.
			bool ParseArguments(int argc, char **argv, int argi);

			const Argument *GetArgument(std::string argumentName) const
			{
				std::map<std::string, const Argument *>::const_iterator it = argumentMap.find(argumentName);
				return (it != argumentMap.end() ? it->second : nullptr);
			}

			const std::map<std::string, ArgumentType>& GetArgumentTypes(void) const
			{
				return (argumentTypes);
			}

			const std::vector<const Argument *>& GetArguments(void) const
			{
				return (argumentVector);
			}
	};
}

#endif
