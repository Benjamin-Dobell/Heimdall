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

#ifndef LIBPIT_H
#define LIBPIT_H

#ifdef WIN32
#pragma warning(disable : 4996)
#endif

#if (!(defined _MSC_VER) || (_MSC_VER < 1700))

#ifndef nullptr
#define nullptr 0
#endif

#endif

// C/C++ Standard Library
#include <cstring>
#include <string>
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
				kFlashFilenameMaxLength = 32,
				kFotaFilenameMaxLength = 32
			};

			enum
			{
				kBinaryTypeApplicationProcessor = 0,
				kBinaryTypeCommunicationProcessor = 1
			};

			enum
			{
				kDeviceTypeOneNand = 0,
				kDeviceTypeFile, // FAT
				kDeviceTypeMMC,
				kDeviceTypeAll // ?
			};

			enum
			{
				kAttributeWrite = 1,
				kAttributeSTL = 1 << 1/*,
				kAttributeBML = 1 << 2*/ // ???
			};

			enum
			{
				kUpdateAttributeFota = 1,
				kUpdateAttributeSecure = 1 << 1
			};

		private:

			unsigned int binaryType;
			unsigned int deviceType;
			unsigned int identifier;
			unsigned int attributes;
			unsigned int updateAttributes;

			unsigned int blockSizeOrOffset;
			unsigned int blockCount;

			unsigned int fileOffset; // Obsolete
			unsigned int fileSize; // Obsolete

			char partitionName[kPartitionNameMaxLength];
			char flashFilename[kFlashFilenameMaxLength]; // USB flash filename
			char fotaFilename[kFotaFilenameMaxLength]; // Firmware over the air

		public:

			PitEntry();
			~PitEntry();

			bool Matches(const PitEntry *otherPitEntry) const;

			bool IsFlashable(void) const
			{
				return strlen(partitionName) != 0;
			}

			unsigned int GetBinaryType(void) const
			{
				return binaryType;
			}

			void SetBinaryType(unsigned int binaryType)
			{
				this->binaryType = binaryType;
			}

			unsigned int GetDeviceType(void) const
			{
				return deviceType;
			}

			void SetDeviceType(unsigned int deviceType)
			{
				this->deviceType = deviceType;
			}

			unsigned int GetIdentifier(void) const
			{
				return identifier;
			}

			void SetIdentifier(unsigned int identifier)
			{
				this->identifier = identifier;
			}

			unsigned int GetAttributes(void) const
			{
				return attributes;
			}

			void SetAttributes(unsigned int attributes)
			{
				this->attributes = attributes;
			}

			unsigned int GetUpdateAttributes(void) const
			{
				return updateAttributes;
			}

			void SetUpdateAttributes(unsigned int updateAttributes)
			{
				this->updateAttributes = updateAttributes;
			}
			
			// Different versions of Loke (secondary bootloaders) on different devices intepret this differently.
			unsigned int GetBlockSizeOrOffset(void) const
			{
				return blockSizeOrOffset;
			}

			void SetBlockSizeOrOffset(unsigned int blockSizeOrOffset)
			{
				this->blockSizeOrOffset = blockSizeOrOffset;
			}

			unsigned int GetBlockCount(void) const
			{
				return blockCount;
			}

			void SetBlockCount(unsigned int blockCount)
			{
				this->blockCount = blockCount;
			}

			unsigned int GetFileOffset(void) const
			{
				return fileOffset;
			}

			void SetFileOffset(unsigned int fileOffset)
			{
				this->fileOffset = fileOffset;
			}

			unsigned int GetFileSize(void) const
			{
				return fileSize;
			}

			void SetFileSize(unsigned int fileSize)
			{
				this->fileSize = fileSize;
			}

			const char *GetPartitionName(void) const
			{
				return partitionName;
			}

			void SetPartitionName(const char *partitionName)
			{
				// This isn't strictly necessary but ensures no junk is left in our PIT file.
				memset(this->partitionName, 0, kPartitionNameMaxLength);

				if (strlen(partitionName) < kPartitionNameMaxLength)
					strcpy(this->partitionName, partitionName);
				else
					memcpy(this->partitionName, partitionName, kPartitionNameMaxLength - 1);
			}

			const char *GetFlashFilename(void) const
			{
				return flashFilename;
			}

			void SetFlashFilename(const char *flashFilename)
			{
				// This isn't strictly necessary but ensures no junk is left in our PIT file.
				memset(this->flashFilename, 0, kFlashFilenameMaxLength);

				if (strlen(partitionName) < kFlashFilenameMaxLength)
					strcpy(this->flashFilename, flashFilename);
				else
					memcpy(this->flashFilename, flashFilename, kFlashFilenameMaxLength - 1);
			}

			const char *GetFotaFilename(void) const
			{
				return fotaFilename;
			}

			void SetFotaFilename(const char *fotaFilename)
			{
				// This isn't strictly necessary but ensures no junk is left in our PIT file.
				memset(this->fotaFilename, 0, kFotaFilenameMaxLength);

				if (strlen(partitionName) < kFotaFilenameMaxLength)
					strcpy(this->fotaFilename, fotaFilename);
				else
					memcpy(this->fotaFilename, fotaFilename, kFotaFilenameMaxLength - 1);
			}
	};

	class PitData
	{
		public:

			enum
			{
				kFileIdentifier = 0x12349876,
				kHeaderDataSize = 28,
				kPaddedSizeMultiplicand = 4096
			};

		private:

			unsigned int entryCount; // 0x04
			unsigned int unknown1;   // 0x08
			unsigned int unknown2;   // 0x0C

			unsigned short unknown3; // 0x10
			unsigned short unknown4; // 0x12

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

			unsigned int GetDataSize(void) const
			{
				return PitData::kHeaderDataSize + entryCount * PitEntry::kDataSize;
			}

			unsigned int GetPaddedSize(void) const
			{
				unsigned int dataSize = GetDataSize();
				unsigned int paddedSize = (dataSize / kPaddedSizeMultiplicand) * kPaddedSizeMultiplicand;

				if (dataSize % kPaddedSizeMultiplicand != 0)
					paddedSize += kPaddedSizeMultiplicand;

				return paddedSize;
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
