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
#include <cerrno>
#include <limits.h>
#include <stdlib.h>

// Heimdall
#include "Heimdall.h"
#include "Utility.h"

using namespace Heimdall;

NumberParsingStatus Utility::ParseInt(int &intValue, const char *string, int base)
{
    errno = 0;

    char *end;
    long longValue = strtol(string, &end, base);

	if (*string == '\0' || *end != '\0')
	{
		return (kNumberParsingStatusInconvertible);
	}
	else if (errno == ERANGE)
	{
		intValue = (longValue == LONG_MAX) ? INT_MAX : INT_MIN;
		return (kNumberParsingStatusRangeError);
	}
	else if (longValue > INT_MAX)
	{
		intValue = INT_MAX;
		return (kNumberParsingStatusRangeError);
	}
	else if (longValue < INT_MIN)
	{
		intValue = INT_MIN;
		return (kNumberParsingStatusRangeError);
	}

	intValue = longValue;
	return (kNumberParsingStatusSuccess);
}

NumberParsingStatus Utility::ParseUnsignedInt(unsigned int &uintValue, const char *string, int base)
{
    errno = 0;

    char *end;
    unsigned long ulongValue = strtoul(string, &end, base);

	if (*string == '\0' || *end != '\0')
	{
		return kNumberParsingStatusInconvertible;
	}
	else if (errno == ERANGE || ulongValue > INT_MAX)
	{
		uintValue = UINT_MAX;
		return (kNumberParsingStatusRangeError);
	}

	uintValue = ulongValue;
	return (kNumberParsingStatusSuccess);
}
