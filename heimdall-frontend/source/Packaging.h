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

#ifndef PACKAGING_H
#define PACKAGING_H

// Qt
#include <QList>
#include <QString>
#include <QTemporaryFile>

// Heimdall Frontend
#include "PackageData.h"

namespace HeimdallFrontend
{
	union TarHeader
	{		
		enum
		{
			kBlockLength = 512,
			kBlockReadCount = 8,
			kBlockWriteCount = 8,

			kTarHeaderLength = 257,
			kUstarHeaderLength = 500,
		};

		enum
		{
			kModeOtherExecute = 1,
			kModeOtherWrite = 1 << 1,
			kModeOtherRead = 1 << 2,

			kModeGroupExecute = 1 << 3,
			kModeGroupWrite = 1 << 4,
			kModeGroupRead = 1 << 5,

			kModeOwnerExecute = 1 << 6,
			kModeOwnerWrite = 1 << 7,
			kModeOwnerRead = 1 << 8,

			kModeReserved = 2 << 9,
			kModeSetGid = 2 << 10,
			kModeSetUid = 2 << 11		
		};

		struct
		{
			char name[100];
			char mode[8];
			char userId[8];
			char groupId[8];
			char size[12];
			char modifiedTime[12];
			char checksum[8];
			char typeFlag;
			char linkName[100];
			char magic[6];
			char version[2];
			char userName[32];
			char groupName[32];
			char devMajor[8];
			char devMinor[8];
			char prefix[155];
		} fields;

		char buffer[kBlockLength];
	};

	class Packaging
	{
		public:

			// Would definitely prefer to use an enum but VC++ and GCC give conflicting warnings about C++0x or type overflow.
			static const qint64 kMaxFileSize;

		private:

			enum
			{
				kExtractBufferLength = 262144,
				kCompressBufferLength = 262144
			};
			
			// TODO: Add support for sparse files to both methods?
			static bool ExtractTar(QTemporaryFile& tarFile, PackageData *packageData);

			static bool WriteTarEntry(const QString& filePath, QTemporaryFile *tarFile, const QString& entryFilename);
			static bool CreateTar(const FirmwareInfo& firmwareInfo, QTemporaryFile *tarFile); // Uses original TAR format.

		public:

			static const char *ustarMagic;

			static bool ExtractPackage(const QString& packagePath, PackageData *packageData);
			static bool BuildPackage(const QString& packagePath, const FirmwareInfo& firmwareInfo);

			static QString ClashlessFilename(const QList<FileInfo>& fileInfos, int fileInfoIndex);
			static QString ClashlessFilename(const QList<FileInfo>& fileInfos, const QString& filename);
	};
}

#endif
