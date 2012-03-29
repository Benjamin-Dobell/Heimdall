/* Copyright (c) 2010-2011 Benjamin Dobell, Glass Echidna
 
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

// libpit
#include "libpit.h"

using namespace libpit;

PitEntry::PitEntry()
{
	unused = false;
	chipIdentifier = 0;
	partitionIdentifier = 0;
	partitionFlags = 0;
	unknown1 = 0;
	partitionBlockSize = 0;
	partitionBlockCount = 0;
	unknown2 = 0;
	unknown3 = 0;

	memset(partitionName, 0, 32);
	memset(filename, 0, 64);
}

PitEntry::~PitEntry()
{
}

bool PitEntry::Matches(const PitEntry *otherPitEntry) const
{
	if (unused == otherPitEntry->unused && chipIdentifier == otherPitEntry->chipIdentifier && partitionIdentifier == otherPitEntry->partitionIdentifier
		&& partitionFlags == otherPitEntry->partitionFlags && unknown1 == otherPitEntry->unknown1 && partitionBlockSize == otherPitEntry->partitionBlockSize
		&& partitionBlockCount == otherPitEntry->partitionBlockCount && unknown2 == otherPitEntry->unknown2 && unknown3 == otherPitEntry->unknown3
		&& strcmp(partitionName, otherPitEntry->partitionName) == 0 && strcmp(filename, otherPitEntry->filename) == 0)
	{
		return (true);
	}
	else
	{
		return (false);
	}
}



PitData::PitData()
{
	entryCount = 0;

	unknown1 = 0;
	unknown2 = 0;

	unknown3 = 0;
	unknown4 = 0;

	unknown5 = 0;
	unknown6 = 0;

	unknown7 = 0;
	unknown8 = 0;
}

PitData::~PitData()
{
	for (unsigned int i = 0; i < entries.size(); i++)
		delete entries[i];
}

bool PitData::Unpack(const unsigned char *data)
{
	if (PitData::UnpackInteger(data, 0) != PitData::kFileIdentifier)
		return (false);

	// Remove existing entries
	for (unsigned int i = 0; i < entries.size(); i++)
		delete entries[i];

	entryCount = PitData::UnpackInteger(data, 4);

	entries.resize(entryCount);

	unknown1 = PitData::UnpackInteger(data, 8);
	unknown2 = PitData::UnpackInteger(data, 12);

	unknown3 = PitData::UnpackShort(data, 16);
	unknown4 = PitData::UnpackShort(data, 18);

	unknown5 = PitData::UnpackShort(data, 20);
	unknown6 = PitData::UnpackShort(data, 22);

	unknown7 = PitData::UnpackShort(data, 24);
	unknown8 = PitData::UnpackShort(data, 26);

	unsigned int integerValue;
	unsigned int entryOffset;

	for (unsigned int i = 0; i < entryCount; i++)
	{
		entryOffset = PitData::kHeaderDataSize + i * PitEntry::kDataSize;

		entries[i] = new PitEntry();

		integerValue = PitData::UnpackInteger(data, entryOffset);
		entries[i]->SetUnused((integerValue != 0) ? true : false);

		integerValue = PitData::UnpackInteger(data, entryOffset + 4);
		entries[i]->SetChipIdentifier(integerValue);

		integerValue = PitData::UnpackInteger(data, entryOffset + 8);
		entries[i]->SetPartitionIdentifier(integerValue);

		integerValue = PitData::UnpackInteger(data, entryOffset + 12);
		entries[i]->SetPartitionFlags(integerValue);

		integerValue = PitData::UnpackInteger(data, entryOffset + 16);
		entries[i]->SetUnknown1(integerValue);

		integerValue = PitData::UnpackInteger(data, entryOffset + 20);
		entries[i]->SetPartitionBlockSize(integerValue);

		integerValue = PitData::UnpackInteger(data, entryOffset + 24);
		entries[i]->SetPartitionBlockCount(integerValue);

		integerValue = PitData::UnpackInteger(data, entryOffset + 28);
		entries[i]->SetUnknown2(integerValue);

		integerValue = PitData::UnpackInteger(data, entryOffset + 32);
		entries[i]->SetUnknown3(integerValue);

		entries[i]->SetPartitionName((const char *)data + entryOffset + 36);
		entries[i]->SetFilename((const char *)data + entryOffset + 36 + PitEntry::kPartitionNameMaxLength);
	}

	return (true);
}

void PitData::Pack(unsigned char *data) const
{
	PitData::PackInteger(data, 0, PitData::kFileIdentifier);

	PitData::PackInteger(data, 4, entryCount);

	PitData::PackInteger(data, 8, unknown1);
	PitData::PackInteger(data, 12, unknown2);

	PitData::PackShort(data, 16, unknown3);
	PitData::PackShort(data, 18, unknown4);

	PitData::PackShort(data, 20, unknown5);
	PitData::PackShort(data, 22, unknown6);

	PitData::PackShort(data, 24, unknown7);
	PitData::PackShort(data, 26, unknown8);

	int entryOffset;

	for (unsigned int i = 0; i < entryCount; i++)
	{
		entryOffset = PitData::kHeaderDataSize + i * PitEntry::kDataSize;

		PitData::PackInteger(data, entryOffset, (entries[i]->GetUnused()) ? 1 : 0);

		PitData::PackInteger(data, entryOffset + 4, entries[i]->GetChipIdentifier());
		PitData::PackInteger(data, entryOffset + 8, entries[i]->GetPartitionIdentifier());
		PitData::PackInteger(data, entryOffset + 12, entries[i]->GetPartitionFlags());

		PitData::PackInteger(data, entryOffset + 16, entries[i]->GetUnknown1());

		PitData::PackInteger(data, entryOffset + 20, entries[i]->GetPartitionBlockSize());
		PitData::PackInteger(data, entryOffset + 24, entries[i]->GetPartitionBlockCount());

		PitData::PackInteger(data, entryOffset + 28, entries[i]->GetUnknown2());
		PitData::PackInteger(data, entryOffset + 32, entries[i]->GetUnknown3());

		memcpy(data + entryOffset + 36, entries[i]->GetPartitionName(), PitEntry::kPartitionNameMaxLength);
		memcpy(data + entryOffset + 36 + PitEntry::kPartitionNameMaxLength, entries[i]->GetPartitionName(), PitEntry::kFilenameMaxLength);
	}
}

bool PitData::Matches(const PitData *otherPitData) const
{
	if (entryCount == otherPitData->entryCount && unknown1 == otherPitData->unknown1 && unknown2 == otherPitData->unknown2
		&& unknown3 == otherPitData->unknown3 && unknown4 == otherPitData->unknown4 && unknown5 == otherPitData->unknown5
		&& unknown6 == otherPitData->unknown6 && unknown7 == otherPitData->unknown7 && unknown8 == otherPitData->unknown8)
	{
		for (unsigned int i = 0; i < entryCount; i++)
		{
			if (!entries[i]->Matches(otherPitData->entries[i]))
				return (false);
		}

		return (true);
	}
	else
	{
		return (false);
	}
}

void PitData::Clear(void)
{
	entryCount = 0;

	unknown1 = 0;
	unknown2 = 0;

	unknown3 = 0;
	unknown4 = 0;

	unknown5 = 0;
	unknown6 = 0;

	unknown7 = 0;
	unknown8 = 0;

	for (unsigned int i = 0; i < entries.size(); i++)
		delete entries[i];

	entries.clear();
}

PitEntry *PitData::GetEntry(unsigned int index)
{
	return (entries[index]);
}

const PitEntry *PitData::GetEntry(unsigned int index) const
{
	return (entries[index]);
}

PitEntry *PitData::FindEntry(const char *partitionName)
{
	for (unsigned int i = 0; i < entries.size(); i++)
	{
		if (!entries[i]->GetUnused() && strcmp(entries[i]->GetPartitionName(), partitionName) == 0)
			return (entries[i]);
	}

	return (nullptr);
}

const PitEntry *PitData::FindEntry(const char *partitionName) const
{
	for (unsigned int i = 0; i < entries.size(); i++)
	{
		if (!entries[i]->GetUnused() && strcmp(entries[i]->GetPartitionName(), partitionName) == 0)
			return (entries[i]);
	}

	return (nullptr);
}

PitEntry *PitData::FindEntry(unsigned int partitionIdentifier)
{
	for (unsigned int i = 0; i < entries.size(); i++)
	{
		if (!entries[i]->GetUnused() && entries[i]->GetPartitionIdentifier() == partitionIdentifier)
			return (entries[i]);
	}

	return (nullptr);
}

const PitEntry *PitData::FindEntry(unsigned int partitionIdentifier) const
{
	for (unsigned int i = 0; i < entries.size(); i++)
	{
		if (!entries[i]->GetUnused() && entries[i]->GetPartitionIdentifier() == partitionIdentifier)
			return (entries[i]);
	}

	return (nullptr);
}
