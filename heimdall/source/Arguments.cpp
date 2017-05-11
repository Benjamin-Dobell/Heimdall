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
#include "Heimdall.h"
#include "Interface.h"
#include "Utility.h"

using namespace std;
using namespace Heimdall;

FlagArgument *FlagArgument::ParseArgument(const std::string& name, int argc, char **argv, int& argi)
{
	return new FlagArgument(name);
}



StringArgument *StringArgument::ParseArgument(const std::string& name, int argc, char **argv, int& argi)
{
	if (++argi < argc)
	{
		return (new StringArgument(name, argv[argi]));
	}
	else
	{
		Interface::Print("Missing parameter for argument: %s\n\n", argv[argi - 1]);
		return (nullptr);
	}
}



UnsignedIntegerArgument *UnsignedIntegerArgument::ParseArgument(const std::string& name, int argc, char **argv, int& argi)
{
	UnsignedIntegerArgument *unsignedIntegerArgument = nullptr;

	if (++argi < argc)
	{
		unsigned int value;
		
		if (Utility::ParseUnsignedInt(value, argv[argi]) == kNumberParsingStatusSuccess)
			unsignedIntegerArgument = new UnsignedIntegerArgument(name, value);
		else
			Interface::Print("%s must be a positive integer.", argv[argi - 1]);
	}
	else
	{
		Interface::Print("Missing parameter for argument: %s\n\n", argv[argi - 1]);
	}

	return (unsignedIntegerArgument);
}



Arguments::Arguments(const map<string, ArgumentType>& argumentTypes, const map<string, string>& shortArgumentAliases,
	const map<string, string>& argumentAliases) :
		argumentTypes(argumentTypes),
		shortArgumentAliases(shortArgumentAliases),
		argumentAliases(argumentAliases)
{
}

Arguments::~Arguments()
{
	for (vector<const Argument *>::const_iterator it = argumentVector.begin(); it != argumentVector.end(); it++)
		delete *it;
}

bool Arguments::ParseArguments(int argc, char **argv, int argi)
{
	for (; argi < argc; ++argi)
	{
		string argumentName = argv[argi];
		string nonwildcardArgumentName;

		if (argumentName.find_first_of("--") == 0)
		{
			// Regular argument
			argumentName = argumentName.substr(2);
			nonwildcardArgumentName = argumentName;
		}
		else if (argumentName.find_first_of("-") == 0)
		{
			// Short argument alias
			string shortArgumentAlias = argumentName.substr(1);
			map<string, string>::const_iterator shortAliasIt = shortArgumentAliases.find(shortArgumentAlias);

			if (shortAliasIt != shortArgumentAliases.end())
			{
				argumentName = shortAliasIt->second;
				nonwildcardArgumentName = argumentName;
			}
			else
			{
				Interface::Print("Unknown argument: %s\n\n", argv[argi]);
				return (false);
			}
		}
		else
		{
			Interface::Print("Invalid argument: %s\n\n", argv[argi]);
			return (false);
		}

		map<string, ArgumentType>::const_iterator argumentTypeIt = argumentTypes.find(argumentName);

		if (argumentTypeIt == argumentTypes.end())
		{
			// No argument with that name, maybe it's an alias...
			map<string, string>::const_iterator aliasIt = argumentAliases.find(argumentName);

			if (aliasIt != argumentAliases.end())
			{
				argumentName = aliasIt->second;
				nonwildcardArgumentName = argumentName;

				argumentTypeIt = argumentTypes.find(argumentName);
			}
		}

		// Handle wilcards
		
		unsigned int unsignedIntName;
		
		if (argumentTypeIt == argumentTypes.end())
		{
			// Look for the unsigned integer wildcard "%d".
			if (Utility::ParseUnsignedInt(unsignedIntName, argumentName.c_str()) == kNumberParsingStatusSuccess)
			{
				argumentTypeIt = argumentTypes.find("%d");
				argumentName = "%d";
			}

			// Look for the string wildcard "%s"
			if (argumentTypeIt == argumentTypes.end())
			{
				argumentTypeIt = argumentTypes.find("%s");
				argumentName = "%s";
			}
		}

		// We don't want to insert wild-cards into our argument map.
		if (argumentName == "%d" || argumentName == "%s")
			argumentName = nonwildcardArgumentName;

		Argument *argument = nullptr;
		
		if (argumentTypeIt != argumentTypes.end())
		{
			switch (argumentTypeIt->second)
			{
				case kArgumentTypeFlag:
					argument = FlagArgument::ParseArgument(argumentName, argc, argv, argi);
					break;

				case kArgumentTypeString:
					argument = StringArgument::ParseArgument(argumentName, argc, argv, argi);
					break;

				case kArgumentTypeUnsignedInteger:
					argument = UnsignedIntegerArgument::ParseArgument(argumentName, argc, argv, argi);
					break;

				default:
					Interface::Print("Unknown argument type: %s\n\n", argv[argi]);
					break;
			}
		}
		else
		{
			Interface::Print("Unknown argument: %s\n\n", argv[argi]);
		}

		if (argument)
		{
			pair<map<string, const Argument *>::iterator, bool> insertResult = argumentMap.insert(pair<string, const Argument *>(argumentName, argument));

			if (!insertResult.second)
			{
				Interface::Print("Duplicate argument: %s (%s)\n\n", argv[argi], argumentName.c_str());
				delete argument;

				return (false);
			}

			argumentVector.push_back(argument);
		}
		else
		{
			return (false);
		}
	}

	return (true);
}
