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

// C/C++ Standard Library
#include <stdio.h>

// zlib
#include "zlib.h"

// Qt
#include <QDateTime>
#include <QDir>

// Heimdall Frontend
#include "Packaging.h"

using namespace HeimdallFrontend;

const char *Packaging::ustarMagic = "ustar";

bool Packaging::ExtractTar(QTemporaryFile& tarFile, PackageData *outputPackageData)
{
	TarHeader tarHeader;

	tarFile.reset();

	bool previousEmpty = false;

	while (!tarFile.atEnd())
	{
		qint64 dataRead = tarFile.read(tarHeader.buffer, TarHeader::kBlockLength);

		if (dataRead != TarHeader::kBlockLength)
			return (false);

		bool ustarFormat = strcmp(tarHeader.fields.magic, ustarMagic) == 0;
		bool empty = true;

		for (int i = 0; i < TarHeader::kBlockLength; i++)
		{
			if (tarHeader.buffer[i] != 0)
			{
				empty = false;
				break;
			}
		}

		if (empty)
		{
			if (previousEmpty)
			{
				// Two empty blocks in a row means we've reached the end of the archive.
				// TODO: Make sure we're at the end of the file.
				break;
			}
		}
		else
		{
			// TODO: Check checksum

			bool parsed = false;
		
			// The size field is not always null terminated, so we must create a copy and null terminate it for parsing.
			char fileSizeString[13];
			memcpy(fileSizeString, tarHeader.fields.size, 12);
			fileSizeString[12] = '\0';

			qulonglong fileSize = QString(fileSizeString).toULongLong(&parsed, 8);

			if (!parsed)
			{
				// TODO: Error message?
				return (false);
			}

			if (fileSize > 0 && tarHeader.fields.typeFlag == '0')
			{
				// We're working with a file.
				QString filename = QString::fromUtf8(tarHeader.fields.name);

				// This is slightly pointless as we don't support directories...
				if (ustarFormat)
					filename.prepend(tarHeader.fields.prefix);

				QTemporaryFile *outputFile = new QTemporaryFile("XXXXXX-" + filename);
				outputFile->open();

				outputPackageData->GetFiles().append(outputFile);

				qulonglong dataRemaining = fileSize;
				char readBuffer[TarHeader::kBlockReadCount * TarHeader::kBlockLength];

				// Copy the file contents from tarFile to outputFile
				while (dataRemaining > 0)
				{
					qint64 fileDataToRead = (dataRemaining < TarHeader::kBlockReadCount * TarHeader::kBlockLength)
						? dataRemaining : TarHeader::kBlockReadCount * TarHeader::kBlockLength;

					qint64 dataRead = tarFile.read(readBuffer, fileDataToRead + (TarHeader::kBlockLength - fileDataToRead % TarHeader::kBlockLength) % TarHeader::kBlockLength);

					if (dataRead < fileDataToRead || dataRead % TarHeader::kBlockLength != 0)
						return (false);

					outputFile->write(readBuffer, fileDataToRead);

					dataRemaining -= fileDataToRead;
				}

				outputFile->close();
			}
			else
			{
				// We don't support links/directories.
				return (false);
			}
		}

		previousEmpty = empty;
	}

	return (true);
}

bool Packaging::CreateTar(const PackageData& packageData, QTemporaryFile *outputTarFile)
{
	const QList<FileInfo>& fileInfos = packageData.GetFirmwareInfo().GetFileInfos();

	if (!outputTarFile->open())
	{
		// TODO: "Failed to open \"%s\""
		return (false);
	}

	bool failure = false;

	TarHeader tarHeader;

	for (int i = 0; i < fileInfos.length(); i++)
	{
		memset(tarHeader.buffer, 0, TarHeader::kBlockLength);

		QFile file(fileInfos[i].GetFilename());

		if (!file.open(QFile::ReadOnly))
		{
			// TODO: "Failed to open \"%s\""
			failure = true;
			break;
		}

		if (file.size() > TarHeader::kMaxFileSize)
		{
			// TODO: "File is too large to packaged"
			failure = true;
			break;
		}

		QFileInfo qtFileInfo(file);
		strcpy(tarHeader.fields.name, qtFileInfo.fileName().toUtf8().constData());
		
		unsigned int mode = 0;

		QFile::Permissions permissions = file.permissions();

		// Other
		if (permissions.testFlag(QFile::ExeOther))
			mode |= TarHeader::kModeOtherExecute;
		if (permissions.testFlag(QFile::WriteOther))
			mode |= TarHeader::kModeOtherWrite;
		if (permissions.testFlag(QFile::ReadOther))
			mode |= TarHeader::kModeOtherRead;

		// Group
		if (permissions.testFlag(QFile::ExeGroup))
			mode |= TarHeader::kModeGroupExecute;
		if (permissions.testFlag(QFile::WriteGroup))
			mode |= TarHeader::kModeGroupWrite;
		if (permissions.testFlag(QFile::ReadGroup))
			mode |= TarHeader::kModeGroupRead;

		// Owner
		if (permissions.testFlag(QFile::ExeOwner))
			mode |= TarHeader::kModeOwnerExecute;
		if (permissions.testFlag(QFile::WriteOwner))
			mode |= TarHeader::kModeOwnerWrite;
		if (permissions.testFlag(QFile::ReadOwner))
			mode |= TarHeader::kModeOwnerRead;

		sprintf(tarHeader.fields.mode, "%o", mode);

		sprintf(tarHeader.fields.userId, "%o", qtFileInfo.ownerId());
		sprintf(tarHeader.fields.groupId, "%o", qtFileInfo.groupId());

		// Note: We don't support base-256 encoding. Support could be added in future.
		sprintf(tarHeader.fields.size, "%o", file.size());

		sprintf(tarHeader.fields.modifiedTime, "%o", qtFileInfo.lastModified().toMSecsSinceEpoch());
		
		// Regular File
		tarHeader.fields.typeFlag = '0';

		// Calculate checksum
		int checksum = 0;

		for (int i = 0; i < TarHeader::kTarHeaderLength; i++)
			checksum += tarHeader.buffer[i];

		sprintf(tarHeader.fields.checksum, "%o", checksum);

		// Write the header to the TAR file.
		outputTarFile->write(tarHeader.buffer, TarHeader::kBlockLength);

		char buffer[TarHeader::kBlockWriteCount * TarHeader::kBlockLength];

		for (qint64 i = 0; i < file.size(); i++)
		{
			qint64 dataRead = file.read(buffer, TarHeader::kBlockWriteCount * TarHeader::kBlockLength);

			if (outputTarFile->write(buffer, dataRead) != dataRead)
			{
				// TODO: "Failed to write data to the temporary TAR file."
				failure = true;
				break;
			}

			if (dataRead % TarHeader::kBlockLength != 0)
			{
				int remainingBlockLength = TarHeader::kBlockLength - dataRead % TarHeader::kBlockLength;
				memset(buffer, 0, remainingBlockLength);

				if (outputTarFile->write(buffer, remainingBlockLength) != remainingBlockLength)
				{
					// TODO: "Failed to write data to the temporary TAR file."
					failure = true;
					break;
				}
			}

			i += dataRead;
		}

		if (failure)
			break;
	}

	if (failure)
	{
		outputTarFile->resize(0);
		outputTarFile->close();
		return (false);
	}

	// Write two empty blocks to signify the end of the archive.
	memset(tarHeader.buffer, 0, TarHeader::kBlockLength);
	outputTarFile->write(tarHeader.buffer, TarHeader::kBlockLength);
	outputTarFile->write(tarHeader.buffer, TarHeader::kBlockLength);

	outputTarFile->close();

	return (true);
}

bool Packaging::ExtractPackage(const QString& packagePath, PackageData *outputPackageData)
{
	FILE *compressedPackageFile = fopen(packagePath.toStdString().c_str(), "rb");
	gzFile packageFile = gzdopen(fileno(compressedPackageFile), "rb");

	QTemporaryFile outputTar("XXXXXX.tar");

	if (!outputTar.open())
	{
		gzclose(packageFile);

		return (false);
	}

	char buffer[32768];

	int bytesRead;

	do
	{
		bytesRead = gzread(packageFile, buffer, 32768);

		if (bytesRead == -1)
		{
			gzclose(packageFile);

			return (false);
		}

		outputTar.write(buffer, bytesRead);
	} while (bytesRead > 0);

	gzclose(packageFile); // Closes packageFile and compressedPackageFile

	if (!ExtractTar(outputTar, outputPackageData))
		return (false);

	// Find and read firmware.xml
	for (int i = 0; i < outputPackageData->GetFiles().length(); i++)
	{
		QTemporaryFile *file = outputPackageData->GetFiles()[i];

		if (file->fileTemplate() == "XXXXXX-firmware.xml")
		{
			if (!outputPackageData->ReadFirmwareInfo(file))
			{
				outputPackageData->Clear();
				return (false);
			}

			return (true);
		}
	}

	return (false);
}

bool Packaging::BuildPackage(const QString& packagePath, const PackageData& packageData)
{
	QTemporaryFile temporaryFile("XXXXXX.tar");

	if (!CreateTar(packageData, &temporaryFile))
		return (false);

	return (true);
}
