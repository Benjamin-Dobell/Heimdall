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

#ifdef WIN32
#pragma warning(disable : 4996)
#endif

// C/C++ Standard Library
#include <stdio.h>

// zlib
#include "zlib.h"

// Qt
#include <QDateTime>
#include <QDir>
#include <QProgressDialog>

// Heimdall Frontend
#include "Packaging.h"

using namespace HeimdallFrontend;

const char *Packaging::ustarMagic = "ustar";

bool Packaging::ExtractTar(QTemporaryFile& tarFile, PackageData *packageData)
{
	TarHeader tarHeader;

	if (!tarFile.open())
	{
		// TODO: "Error opening temporary TAR archive."
		return (false);
	}

	bool previousEmpty = false;

	QProgressDialog progressDialog("Extracting files...", "Cancel", 0, tarFile.size());
	progressDialog.setWindowModality(Qt::ApplicationModal);
	progressDialog.setWindowTitle("Heimdall Frontend");

	while (!tarFile.atEnd())
	{
		qint64 dataRead = tarFile.read(tarHeader.buffer, TarHeader::kBlockLength);

		if (dataRead != TarHeader::kBlockLength)
		{
			// TODO: "Package's TAR archive is malformed."
			tarFile.close();
			progressDialog.close();

			return (false);
		}

		progressDialog.setValue(tarFile.pos());

		if (progressDialog.wasCanceled())
		{
			tarFile.close();
			progressDialog.close();

			return (false);
		}

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
			int checksum = 0;

			for (char *bufferIndex = tarHeader.buffer; bufferIndex < tarHeader.fields.checksum; bufferIndex++)
				checksum += static_cast<unsigned char>(*bufferIndex);

			checksum += 8 * ' ';
			checksum += static_cast<unsigned char>(tarHeader.fields.typeFlag);

			// Both the TAR and USTAR formats have terrible documentation, it's not clear if the following code is required.
			/*if (ustarFormat)
			{
				for (char *bufferIndex = tarHeader.fields.linkName; bufferIndex < tarHeader.fields.prefix + 155; bufferIndex++)
					checksum += static_cast<unsigned char>(*bufferIndex);
			}*/

			bool parsed = false;
		
			// The size field is not always null terminated, so we must create a copy and null terminate it for parsing.
			char fileSizeString[13];
			memcpy(fileSizeString, tarHeader.fields.size, 12);
			fileSizeString[12] = '\0';

			qulonglong fileSize = QString(fileSizeString).toULongLong(&parsed, 8);

			if (!parsed)
			{
				// TODO: Error message?
				tarFile.close();
				return (false);
			}

			if (fileSize > 0 && tarHeader.fields.typeFlag == '0')
			{
				// We're working with a file.
				QString filename = QString::fromUtf8(tarHeader.fields.name);

				QTemporaryFile *outputFile = new QTemporaryFile("XXXXXX-" + filename);
				packageData->GetFiles().append(outputFile);

				if (!outputFile->open())
				{
					// TODO: "Failed to open output file \"%s\""
					tarFile.close();
					return (false);
				}

				qulonglong dataRemaining = fileSize;
				char readBuffer[TarHeader::kBlockReadCount * TarHeader::kBlockLength];

				// Copy the file contents from tarFile to outputFile
				while (dataRemaining > 0)
				{
					qint64 fileDataToRead = (dataRemaining < TarHeader::kBlockReadCount * TarHeader::kBlockLength)
						? dataRemaining : TarHeader::kBlockReadCount * TarHeader::kBlockLength;

					qint64 dataRead = tarFile.read(readBuffer, fileDataToRead + (TarHeader::kBlockLength - fileDataToRead % TarHeader::kBlockLength) % TarHeader::kBlockLength);

					if (dataRead < fileDataToRead || dataRead % TarHeader::kBlockLength != 0)
					{
						// TODO: "Unexpected error extracting package files."
						tarFile.close();
						outputFile->close();

						remove(outputFile->fileName().toStdString().c_str());

						return (false);
					}

					outputFile->write(readBuffer, fileDataToRead);

					dataRemaining -= fileDataToRead;

					progressDialog.setValue(tarFile.pos());

					if (progressDialog.wasCanceled())
					{
						tarFile.close();
						outputFile->close();

						remove(outputFile->fileName().toStdString().c_str());

						progressDialog.close();

						return (false);
					}
				}

				outputFile->close();
			}
			else
			{
				// TODO: "Heimdall packages shouldn't contain links or directories."
				tarFile.close();
				return (false);
			}
		}

		previousEmpty = empty;
	}

	progressDialog.close();
	tarFile.close();

	return (true);
}

bool Packaging::WriteTarEntry(const QString& filename, QTemporaryFile *tarFile, bool firmwareXml)
{
	TarHeader tarHeader;
	memset(tarHeader.buffer, 0, TarHeader::kBlockLength);

	QFile file(filename);

	if (!file.open(QFile::ReadOnly))
	{
		// TODO: "Failed to open \"%s\""
		return (false);
	}

	if (file.size() > TarHeader::kMaxFileSize)
	{
		// TODO: "File is too large to packaged"
		return (false);
	}

	QFileInfo qtFileInfo(file);
	QByteArray utfFilename;

	if (firmwareXml)
	{
		utfFilename = QString("firmware.xml").toUtf8();
	}
	else
	{
		utfFilename = qtFileInfo.fileName().toUtf8();

		if (utfFilename.length() > 100)
		{
			// TODO: "Filename is too long"
			return (false);
		}
	}

	strcpy(tarHeader.fields.name, utfFilename.constData());
		
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

	sprintf(tarHeader.fields.mode, "%07o", mode);

	// Owner id
	uint id = qtFileInfo.ownerId();

	if (id < 2097151)
		sprintf(tarHeader.fields.userId, "%07o", id);
	else
		sprintf(tarHeader.fields.userId, "%07o", 0);

	// Group id
	id = qtFileInfo.groupId();

	if (id < 2097151)
		sprintf(tarHeader.fields.groupId, "%07o", id);
	else
		sprintf(tarHeader.fields.groupId, "%07o", 0);

	// Note: We don't support base-256 encoding. Support could be added later.
	sprintf(tarHeader.fields.size, "%011o", file.size());
	sprintf(tarHeader.fields.modifiedTime, "%011o", qtFileInfo.lastModified().toMSecsSinceEpoch() / 1000);
		
	// Regular File
	tarHeader.fields.typeFlag = '0';

	// Calculate checksum
	int checksum = 0;
	memset(tarHeader.fields.checksum, ' ', 8);
		
	for (int i = 0; i < TarHeader::kTarHeaderLength; i++)
		checksum += static_cast<unsigned char>(tarHeader.buffer[i]);

	sprintf(tarHeader.fields.checksum, "%07o", checksum);

	// Write the header to the TAR file.
	tarFile->write(tarHeader.buffer, TarHeader::kBlockLength);

	char buffer[TarHeader::kBlockWriteCount * TarHeader::kBlockLength];
	qint64 offset = 0;

	while (offset < file.size())
	{
		qint64 dataRead = file.read(buffer, TarHeader::kBlockWriteCount * TarHeader::kBlockLength);

		if (tarFile->write(buffer, dataRead) != dataRead)
		{
			// TODO: "Failed to write data to the temporary TAR file."
			return (false);
		}

		if (dataRead % TarHeader::kBlockLength != 0)
		{
			int remainingBlockLength = TarHeader::kBlockLength - dataRead % TarHeader::kBlockLength;
			memset(buffer, 0, remainingBlockLength);

			if (tarFile->write(buffer, remainingBlockLength) != remainingBlockLength)
			{
				// TODO: "Failed to write data to the temporary TAR file."
				return (false);
			}
		}

		offset += dataRead;
	}

	return (true);
}

bool Packaging::CreateTar(const FirmwareInfo& firmwareInfo, QTemporaryFile *tarFile)
{
	const QList<FileInfo>& fileInfos = firmwareInfo.GetFileInfos();

	QProgressDialog progressDialog("Packaging files...", "Cancel", 0, fileInfos.length() + 2);
	progressDialog.setWindowModality(Qt::ApplicationModal);
	progressDialog.setWindowTitle("Heimdall Frontend");

	QTemporaryFile firmwareXmlFile("XXXXXX-firmware.xml");

	if (!firmwareXmlFile.open())
	{
		// TODO: "Failed to create temporary file "%s"
		return (false);
	}

	firmwareInfo.WriteXml(QXmlStreamWriter(&firmwareXmlFile));

	firmwareXmlFile.close();

	if (!tarFile->open())
	{
		// TODO: "Failed to open \"%s\""
		return (false);
	}

	for (int i = 0; i < fileInfos.length(); i++)
	{
		if (!WriteTarEntry(fileInfos[i].GetFilename(), tarFile))
		{
			tarFile->resize(0);
			tarFile->close();

			progressDialog.close();

			return (false);
		}

		progressDialog.setValue(i);

		if (progressDialog.wasCanceled())
		{
			tarFile->resize(0);
			tarFile->close();

			progressDialog.close();

			return (false);
		}
	}

	if (!WriteTarEntry(firmwareInfo.GetPitFilename(), tarFile))
	{
		tarFile->resize(0);
		tarFile->close();

		return (false);
	}

	progressDialog.setValue(progressDialog.value() + 1);

	if (progressDialog.wasCanceled())
	{
		tarFile->resize(0);
		tarFile->close();

		progressDialog.close();

		return (false);
	}

	if (!WriteTarEntry(firmwareXmlFile.fileName(), tarFile, true))
	{
		tarFile->resize(0);
		tarFile->close();

		return (false);
	}

	progressDialog.setValue(progressDialog.value() + 1);
	progressDialog.close();

	// Write two empty blocks to signify the end of the archive.
	char emptyEntry[TarHeader::kBlockLength];
	memset(emptyEntry, 0, TarHeader::kBlockLength);

	tarFile->write(emptyEntry, TarHeader::kBlockLength);
	tarFile->write(emptyEntry, TarHeader::kBlockLength);

	tarFile->close();

	return (true);
}

bool Packaging::ExtractPackage(const QString& packagePath, PackageData *packageData)
{
	FILE *compressedPackageFile = fopen(packagePath.toStdString().c_str(), "rb");

	if (fopen == NULL)
	{
		// TODO: "Failed to open package \"%s\"."
		return (false);
	}

	fseek(compressedPackageFile, 0, SEEK_END);
	quint64 compressedFileSize = ftell(compressedPackageFile);
	rewind(compressedPackageFile);

	gzFile packageFile = gzdopen(fileno(compressedPackageFile), "rb");

	QTemporaryFile outputTar("XXXXXX.tar");

	if (!outputTar.open())
	{
		// TODO: "Error opening temporary TAR archive."
		gzclose(packageFile);

		return (false);
	}

	char buffer[kExtractBufferLength];
	int bytesRead;
	quint64 totalBytesRead = 0;

	QProgressDialog progressDialog("Decompressing package...", "Cancel", 0, compressedFileSize);
	progressDialog.setWindowModality(Qt::ApplicationModal);
	progressDialog.setWindowTitle("Heimdall Frontend");

	do
	{
		bytesRead = gzread(packageFile, buffer, kExtractBufferLength);

		if (bytesRead == -1)
		{
			// TODO: "Error decompressing archive."
			gzclose(packageFile);
			progressDialog.close();

			return (false);
		}

		outputTar.write(buffer, bytesRead);

		totalBytesRead += bytesRead;
		progressDialog.setValue(totalBytesRead);

		if (progressDialog.wasCanceled())
		{
			gzclose(packageFile);
			progressDialog.close();

			return (false);
		}
	} while (bytesRead > 0);

	progressDialog.close();

	outputTar.close();
	gzclose(packageFile); // Closes packageFile and compressedPackageFile

	if (!ExtractTar(outputTar, packageData))
		return (false);

	// Find and read firmware.xml
	for (int i = 0; i < packageData->GetFiles().length(); i++)
	{
		QTemporaryFile *file = packageData->GetFiles()[i];

		if (file->fileTemplate() == "XXXXXX-firmware.xml")
		{
			if (!packageData->ReadFirmwareInfo(file))
			{
				packageData->Clear();
				return (false);
			}

			return (true);
		}
	}

	return (false);
}

bool Packaging::BuildPackage(const QString& packagePath, const FirmwareInfo& firmwareInfo)
{
	FILE *compressedPackageFile = fopen(packagePath.toStdString().c_str(), "wb");

	if (fopen == NULL)
	{
		// TODO: "Failed to open package "%s"
		return (false);
	}

	QTemporaryFile tar("XXXXXX.tar");

	if (!CreateTar(firmwareInfo, &tar))
		return (false);

	if (!tar.open())
	{
		// TODO: "Failed to open temporary file "%s"
		fclose(compressedPackageFile);
		remove(packagePath.toStdString().c_str());
		return (false);
	}
	
	gzFile packageFile = gzdopen(fileno(compressedPackageFile), "wb");

	char buffer[kCompressBufferLength];
	qint64 totalBytesRead = 0;
	int bytesRead;

	QProgressDialog progressDialog("Compressing package...", "Cancel", 0, tar.size());
	progressDialog.setWindowModality(Qt::ApplicationModal);
	progressDialog.setWindowTitle("Heimdall Frontend");

	do
	{
		bytesRead = tar.read(buffer, kCompressBufferLength);

		if (bytesRead == -1)
		{
			// TODO: "Error reading temporary TAR file."
			gzclose(packageFile);
			remove(packagePath.toStdString().c_str());

			progressDialog.close();

			return (false);
		}

		if (gzwrite(packageFile, buffer, bytesRead) != bytesRead)
		{
			// TODO: "Error compressing package."
			gzclose(packageFile);
			remove(packagePath.toStdString().c_str());

			progressDialog.close();

			return (false);
		}

		totalBytesRead += bytesRead;
		progressDialog.setValue(totalBytesRead);

		if (progressDialog.wasCanceled())
		{
			gzclose(packageFile);
			remove(packagePath.toStdString().c_str());

			progressDialog.close();

			return (false);
		}
	} while (bytesRead > 0);

	progressDialog.close();

	gzclose(packageFile); // Closes packageFile and compressedPackageFile

	return (true);
}
