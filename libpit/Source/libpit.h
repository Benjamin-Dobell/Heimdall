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

#ifndef LIBPIT_H
#define LIBPIT_H

#ifdef WIN32
#pragma warning(disable : 4996)
#endif

// C Standard Library
#include <string.h>
#include <vector>

namespace libpit
{
	class PitEntry
	{
		public:

			enum
			{
				kDataSize = 132,
				kPartitionNameMaxLength = 32,
				kFilenameMaxLength = 64
			};

			enum
			{
				kPartitionTypeRfs       = 0,
				kPartitionTypeBlank     = 1, // ?
				kPartitionTypeExt4      = 2
			};

			enum
			{
				kPartitionFlagWrite     = 1 << 1
			};

		private:

			bool unused;

			unsigned int partitionType;
			unsigned int partitionIdentifier;
			unsigned int partitionFlags;

			unsigned int unknown1;

			unsigned int partitionBlockSize;
			unsigned int partitionBlockCount;

			unsigned int unknown2;
			unsigned int unknown3;

			char partitionName[kPartitionNameMaxLength];
			char filename[kFilenameMaxLength];

		public:

			PitEntry();
			~PitEntry();

			bool Matches(const PitEntry *otherPitEntry) const;

			bool GetUnused(void) const
			{
				return unused;
			}

			void SetUnused(bool unused)
			{
				this->unused = unused;
			}

			unsigned int GetPartitionType(void) const
			{
				return partitionType;
			}

			void SetPartitionType(unsigned int partitionType)
			{
				this->partitionType = partitionType;
			}

			unsigned int GetPartitionIdentifier(void) const
			{
				return partitionIdentifier;
			}

			void SetPartitionIdentifier(unsigned int partitionIdentifier)
			{
				this->partitionIdentifier = partitionIdentifier;
			}

			unsigned int GetPartitionFlags(void) const
			{
				return partitionFlags;
			}

			void SetPartitionFlags(unsigned int partitionFlags)
			{
				this->partitionFlags = partitionFlags;
			}

			unsigned int GetUnknown1(void) const
			{
				return unknown1;
			}

			void SetUnknown1(unsigned int unknown1)
			{
				this->unknown1 = unknown1;
			}

			unsigned int GetPartitionBlockSize(void) const
			{
				return partitionBlockSize;
			}

			void SetPartitionBlockSize(unsigned int partitionBlockSize)
			{
				this->partitionBlockSize = partitionBlockSize;
			}

			unsigned int GetPartitionBlockCount(void) const
			{
				return partitionBlockCount;
			}

			void SetPartitionBlockCount(unsigned int partitionBlockCount)
			{
				this->partitionBlockCount = partitionBlockCount;
			}

			unsigned int GetUnknown2(void) const
			{
				return unknown2;
			}

			void SetUnknown2(unsigned int unknown2)
			{
				this->unknown2 = unknown2;
			}

			unsigned int GetUnknown3(void) const
			{
				return unknown3;
			}

			void SetUnknown3(unsigned int unknown3)
			{
				this->unknown3 = unknown3;
			}

			const char *GetPartitionName(void) const
			{
				return partitionName;
			}

			void SetPartitionName(const char *partitionName)
			{
				// This isn't strictly necessary but ensures no junk is left in our PIT file.
				memset(this->partitionName, 0, 64);

				if (strlen(partitionName) < 64)
					strcpy(this->partitionName, partitionName);
				else
					memcpy(this->partitionName, partitionName, 63);
			}

			const char *GetFilename(void) const
			{
				return filename;
			}

			void SetFilename(const char *filename)
			{
				// This isn't strictly necessary but ensures no junk is left in our PIT file.
				memset(this->filename, 0, 32);

				if (strlen(partitionName) < 32)
					strcpy(this->filename, filename);
				else
					memcpy(this->filename, filename, 31);
			}
	};

	class PitData
	{
		public:

			enum
			{
				kFileIdentifier = 0x12349876,
				kHeaderDataSize = 28
			};

		private:

			unsigned int entryCount; // 0x04
			unsigned int unknown1;   // 0x08
			unsigned int unknown2;   // 0x0C

			unsigned short unknown3; // 0x10 (7508 = I9000, 7703 = I9100 & P1000)?
			unsigned short unknown4; // 0x12 (Always 65, probably flags of some sort)

			unsigned short unknown5; // 0x14
			unsigned short unknown6; // 0x16

			unsigned short unknown7; // 0x18
			unsigned short unknown8; // 0x1A

			// Entries start at 0x1C
			std::vector<PitEntry *> entries;

			static int UnpackInteger(const unsigned char *data, unsigned int offset)
			{
#ifdef WORDS_BIGENDIAN
				int value = (data[offset] << 24) | (data[offset + 1] << 16) |
					(data[offset + 2] << 8) | data[offset + 3];
#else
				// Flip endianness
				int value = data[offset] | (data[offset + 1] << 8) |
					(data[offset + 2] << 16) | (data[offset + 3] << 24);
#endif
				return (value);
			}

			static int UnpackShort(const unsigned char *data, unsigned int offset)
			{
#ifdef WORDS_BIGENDIAN
				short value = (data[offset] << 8) | data[offset + 1];
#else
				// Flip endianness
				short value = data[offset] | (data[offset + 1] << 8);
#endif
				return (value);
			}

			static void PackInteger(unsigned char *data, unsigned int offset, unsigned int value)
			{
#ifdef WORDS_BIGENDIAN
				data[offset] = (value & 0xFF000000) >> 24;
				data[offset + 1] = (value & 0x00FF0000) >> 16;
				data[offset + 2] = (value & 0x0000FF00) >> 8;
				data[offset + 3] = value & 0x000000FF;
#else
				// Flip endianness
				data[offset] = value & 0x000000FF;
				data[offset + 1] = (value & 0x0000FF00) >> 8;
				data[offset + 2] = (value & 0x00FF0000) >> 16;
				data[offset + 3] = (value & 0xFF000000) >> 24;
#endif
			}

			static void PackShort(unsigned char *data, unsigned int offset, unsigned short value)
			{
#ifdef WORDS_BIGENDIAN
				data[offset] = (value & 0xFF00) >> 8;
				data[offset + 1] = value & 0x00FF;
#else
				// Flip endianness
				data[offset] = value & 0x00FF;
				data[offset + 1] = (value & 0xFF00) >> 8;
#endif
			}

		public:

			PitData();
			~PitData();

			bool Unpack(const unsigned char *data);
			void Pack(unsigned char *data) const;

			bool Matches(const PitData *otherPitData) const;

			void Clear(void);

			PitEntry *GetEntry(unsigned int index);
			const PitEntry *GetEntry(unsigned int index) const;

			PitEntry *FindEntry(const char *partitionName);
			const PitEntry *FindEntry(const char *partitionName) const;

			PitEntry *FindEntry(unsigned int partitionIdentifier);
			const PitEntry *FindEntry(unsigned int partitionIdentifier) const;

			unsigned int GetEntryCount(void) const
			{
				return entryCount;
			}

			unsigned int GetUnknown1(void) const
			{
				return unknown1;
			}

			unsigned int GetUnknown2(void) const
			{
				return unknown2;
			}

			unsigned short GetUnknown3(void) const
			{
				return unknown3;
			}

			unsigned short GetUnknown4(void) const
			{
				return unknown4;
			}

			unsigned short GetUnknown5(void) const
			{
				return unknown5;
			}

			unsigned short GetUnknown6(void) const
			{
				return unknown6;
			}

			unsigned short GetUnknown7(void) const
			{
				return unknown7;
			}

			unsigned short GetUnknown8(void) const
			{
				return unknown8;
			}
	};
}

#endif
