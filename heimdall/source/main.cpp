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

// C/C++ Standard Library
#include <algorithm>
#include <map>
#include <stdio.h>
#include <string>

// libpit
#include "libpit.h"

// Heimdall
#include "Heimdall.h"
#include "HelpAction.h"
#include "Interface.h"

using namespace std;
using namespace Heimdall;

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		Interface::PrintUsage();
		return (0);
	}

	int result = 0;
	map<string, Interface::ActionInfo>::const_iterator actionIt = Interface::GetActionMap().find(argv[1]);

	if (actionIt != Interface::GetActionMap().end())
		result = actionIt->second.executeFunction(argc, argv);
	else
		result = HelpAction::Execute(argc, argv);
	
	return (result);
}
