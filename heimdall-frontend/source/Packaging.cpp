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
#include "Alerts.h"
#include "Packaging.h"

using namespace HeimdallFrontend;

const qint64 Packaging::kMaxFileSize = 8589934592ll;
const char *Packaging::ustarMagic = "ustar";

bool Packaging::ExtractTar(QTemporaryFile& tarFile, PackageData *packageData)
{
	TarHeader tarHeader;

	if (!tarFile.open())
	{
		Alerts::DisplayError(QString("Error opening temporary TAR archive:\n%1").arg(tarFile.fileName()));
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
			progressDialog.close();
			Alerts::DisplayError("Package's TAR archive is malformed.");

			tarFile.close();

			return (false);
		}

		progressDialog.setValue(tarFile.pos());

		if (progressDialog.wasCanceled())
		{
			tarFile.close();
			progressDialog.close();

			return (false);
		}

		//bool ustarFormat = strcmp(tarHeader.fields.magic, ustarMagic) == 0;
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
				progressDialog.close();
				Alerts::DisplayError("Tar header contained an invalid file size.");

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
					progressDialog.close();
					Alerts::DisplayError(QString("Failed to open output file: \n%1").arg(outputFile->fileName()));

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
						progressDialog.close();
						Alerts::DisplayError("Unexpected read error whilst extracting package files.");

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
				progressDialog.close();
				Alerts::DisplayError("Heimdall packages shouldn't contain links or directories.");

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

bool Packaging::WriteTarEntry(const QString& filePath, QTemporaryFile *tarFile, const QString& entryFilename)
{
	TarHeader tarHeader;
	memset(tarHeader.buffer, 0, TarHeader::kBlockLength);

	QFile file(filePath);

	if (!file.open(QFile::ReadOnly))
	{
		Alerts::DisplayError(QString("Failed to open file: \n%1").arg(file.fileName()));
		return (false);
	}

	if (file.size() > Packaging::kMaxFileSize)
	{
		Alerts::DisplayError(QString("File is too large to be packaged:\n%1").arg(file.fileName()));
		return (false);
	}

	QFileInfo qtFileInfo(file);
	QByteArray utfFilename;

	utfFilename = entryFilename.toUtf8();

	if (utfFilename.length() > 100)
	{
		Alerts::DisplayError(QString("File name is too long:\n%1").arg(qtFileInfo.fileName()));
		return (false);
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
	sprintf(tarHeader.fields.size, "%011llo", file.size());
	sprintf(tarHeader.fields.modifiedTime, "%u", qtFileInfo.lastModified().toTime_t());
		
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
			Alerts::DisplayError("Failed to write data to the temporary TAR file.");
			return (false);
		}

		if (dataRead % TarHeader::kBlockLength != 0)
		{
			int remainingBlockLength = TarHeader::kBlockLength - dataRead % TarHeader::kBlockLength;
			memset(buffer, 0, remainingBlockLength);

			if (tarFile->write(buffer, remainingBlockLength) != remainingBlockLength)
			{
				Alerts::DisplayError("Failed to write data to the temporary TAR file.");
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
		progressDialog.close();
		Alerts::DisplayError(QString("Failed to create temporary file: \n%1").arg(firmwareXmlFile.fileName()));

		return (false);
	}

	QXmlStreamWriter xml(&firmwareXmlFile);
	firmwareInfo.WriteXml(xml);
	firmwareXmlFile.close();

	if (!tarFile->open())
	{
		progressDialog.close();
		Alerts::DisplayError(QString("Failed to open file: \n%1").arg(tarFile->fileName()));

		return (false);
	}

	for (int i = 0; i < fileInfos.length(); i++)
	{
		// If the file was already compressed we don't compress it again.
		bool skip = false;

		for (int j = 0; j < i; j++)
		{
			if (fileInfos[i].GetFilename() == fileInfos[j].GetFilename())
			{
				skip = true;
				break;
			}
		}

		if (skip)
		{
			progressDialog.setValue(i);
			continue;
		}

		QString filename = ClashlessFilename(fileInfos, i);

		if (filename == "firmware.xml")
		{
			Alerts::DisplayError("You cannot name your partition files \"firmware.xml\".\nIt is a reserved name.");
			return (false);
		}

		if (!WriteTarEntry(fileInfos[i].GetFilename(), tarFile, filename))
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

	int lastSlash = firmwareInfo.GetPitFilename().lastIndexOf('/');

	if (lastSlash < 0)
		lastSlash = firmwareInfo.GetPitFilename().lastIndexOf('\\');

	QString pitFilename = ClashlessFilename(fileInfos, firmwareInfo.GetPitFilename().mid(lastSlash + 1));

	if (pitFilename == "firmware.xml")
	{
		Alerts::DisplayError("You cannot name your PIT file \"firmware.xml\".\nIt is a reserved name.");
		return (false);
	}

	if (!WriteTarEntry(firmwareInfo.GetPitFilename(), tarFile, pitFilename))
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

	if (!WriteTarEntry(firmwareXmlFile.fileName(), tarFile, "firmware.xml"))
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

	if (!compressedPackageFile)
	{
		Alerts::DisplayError(QString("Failed to open package:\n%1").arg(packagePath));
		return (false);
	}

	fseek(compressedPackageFile, 0, SEEK_END);
	quint64 compressedFileSize = ftell(compressedPackageFile);
	rewind(compressedPackageFile);

	gzFile packageFile = gzdopen(fileno(compressedPackageFile), "rb");

	QTemporaryFile outputTar("XXXXXX.tar");

	if (!outputTar.open())
	{
		Alerts::DisplayError("Failed to open temporary TAR archive.");
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
			progressDialog.close();
			Alerts::DisplayError("Error decompressing archive.");

			gzclose(packageFile);

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

	Alerts::DisplayError("firmware.xml is missing from the package.");
	return (false);
}

bool Packaging::BuildPackage(const QString& packagePath, const FirmwareInfo& firmwareInfo)
{
	FILE *compressedPackageFile = fopen(packagePath.toStdString().c_str(), "wb");

	if (!compressedPackageFile)
	{
		Alerts::DisplayError(QString("Failed to create package:\n%1").arg(packagePath));
		return (false);
	}

	QTemporaryFile tar("XXXXXX.tar");

	if (!CreateTar(firmwareInfo, &tar))
	{
		fclose(compressedPackageFile);
		remove(packagePath.toStdString().c_str());

		return (false);
	}

	if (!tar.open())
	{
		Alerts::DisplayError(QString("Failed to open temporary file: \n%1").arg(tar.fileName()));

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
			progressDialog.close();
			Alerts::DisplayError("Error reading temporary TAR file.");

			gzclose(packageFile);
			remove(packagePath.toStdString().c_str());

			return (false);
		}

		if (gzwrite(packageFile, buffer, bytesRead) != bytesRead)
		{
			progressDialog.close();
			Alerts::DisplayError("Error compressing package.");

			gzclose(packageFile);
			remove(packagePath.toStdString().c_str());

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

QString Packaging::ClashlessFilename(const QList<FileInfo>& fileInfos, int fileInfoIndex)
{
	int lastSlash = fileInfos[fileInfoIndex].GetFilename().lastIndexOf('/');

	if (lastSlash < 0)
		lastSlash = fileInfos[fileInfoIndex].GetFilename().lastIndexOf('\\');

	QString filename = fileInfos[fileInfoIndex].GetFilename().mid(lastSlash + 1);
	unsigned int renameIndex = 0;

	// Check for name clashes
	for (int i = 0; i < fileInfoIndex; i++)
	{
		lastSlash = fileInfos[i].GetFilename().lastIndexOf('/');

		if (lastSlash < 0)
			lastSlash = fileInfos[i].GetFilename().lastIndexOf('\\');

		QString otherFilename = fileInfos[i].GetFilename().mid(lastSlash + 1);

		// If the filenames are the same, but the files themselves aren't the same (i.e. not the same path), then rename.
		if (filename == otherFilename && fileInfos[i].GetFilename() != fileInfos[fileInfoIndex].GetFilename())
			renameIndex++;
	}

	if (renameIndex > 0)
	{
		int lastPeriodIndex = filename.lastIndexOf(QChar('.'));
		QString shortFilename;
		QString fileType;

		if (lastPeriodIndex >= 0)
		{
			shortFilename = filename.left(lastPeriodIndex);
			fileType = filename.mid(lastPeriodIndex);
		}
		else
		{
			shortFilename = filename;
		}

		unsigned int renameIndexOffset = 0;
		bool validIndexOffset = true;

		// Before we append a rename index we must ensure it doesn't produce further collisions.
		for (int i = 0; i < fileInfos.length(); i++)
		{
			int lastSlash = fileInfos[i].GetFilename().lastIndexOf('/');

			if (lastSlash < 0)
				lastSlash = fileInfos[i].GetFilename().lastIndexOf('\\');

			QString otherFilename = fileInfos[i].GetFilename().mid(lastSlash + 1);

			if (otherFilename.length() > filename.length() + 1)
			{
				QString trimmedOtherFilename = otherFilename.left(shortFilename.length());

				if (shortFilename == trimmedOtherFilename)
				{
					lastPeriodIndex = otherFilename.lastIndexOf(QChar('.'));
					QString shortOtherFilename;

					if (lastPeriodIndex >= 0)
						shortOtherFilename = otherFilename.left(lastPeriodIndex);
					else
						shortOtherFilename = otherFilename;

					QRegExp renameExp("-[0-9]+");
					
					if (renameExp.lastIndexIn(shortOtherFilename) == shortFilename.length())
					{
						unsigned int trailingInteger = shortOtherFilename.mid(shortFilename.length() + 1).toUInt(&validIndexOffset);

						if (!validIndexOffset)
							break;

						if (trailingInteger > renameIndexOffset)
							renameIndexOffset = trailingInteger;
					}
				}
			}
		}

		if (validIndexOffset)
		{
			// Ensure renaming won't involve integer overflow!
			if (renameIndex > static_cast<unsigned int>(-1) - renameIndexOffset)
				validIndexOffset = false;
		}

		if (validIndexOffset)
		{
			shortFilename.append(QChar('-'));
			shortFilename.append(QString::number(renameIndexOffset + renameIndex));

			return (shortFilename + fileType);
		}
		else
		{
			// Fallback behaviour... an absolutely terrible brute force implementation!
			QString filename;
			QString renamePrefix;

			for (;;)
			{
				renamePrefix.append(QChar('+'));

				for (unsigned int i = 0; i < static_cast<unsigned int>(-1); i++)
				{
					filename = shortFilename + renamePrefix + QString::number(i) + fileType;

					bool valid = true;

					for (int i = 0; i < fileInfos.length(); i++)
					{
						int lastSlash = fileInfos[i].GetFilename().lastIndexOf('/');

						if (lastSlash < 0)
							lastSlash = fileInfos[i].GetFilename().lastIndexOf('\\');

						if (filename == fileInfos[i].GetFilename().mid(lastSlash + 1))
						{
							valid = false;
							break;
						}
					}

					if (valid)
						return (filename);
				}
			}
		}
	}
	else
	{
		return (filename);
	}
}

QString Packaging::ClashlessFilename(const QList<FileInfo>& fileInfos, const QString& filename)
{
	unsigned int renameIndex = 0;

	// Check for name clashes
	for (int i = 0; i < fileInfos.length(); i++)
	{
		int lastSlash = fileInfos[i].GetFilename().lastIndexOf('/');

		if (lastSlash < 0)
			lastSlash = fileInfos[i].GetFilename().lastIndexOf('\\');

		QString otherFilename = fileInfos[i].GetFilename().mid(lastSlash + 1);

		if (filename == otherFilename)
			renameIndex++;
	}

	if (renameIndex > 0)
	{
		int lastPeriodIndex = filename.lastIndexOf(QChar('.'));
		QString shortFilename;
		QString fileType;

		if (lastPeriodIndex >= 0)
		{
			shortFilename = filename.left(lastPeriodIndex);
			fileType = filename.mid(lastPeriodIndex);
		}
		else
		{
			shortFilename = filename;
		}

		unsigned int renameIndexOffset = 0;
		bool validIndexOffset = true;

		// Before we append a rename index we must ensure it doesn't produce further collisions.
		for (int i = 0; i < fileInfos.length(); i++)
		{
			int lastSlash = fileInfos[i].GetFilename().lastIndexOf('/');

			if (lastSlash < 0)
				lastSlash = fileInfos[i].GetFilename().lastIndexOf('\\');

			QString otherFilename = fileInfos[i].GetFilename().mid(lastSlash + 1);

			if (otherFilename.length() > filename.length() + 1)
			{
				QString trimmedOtherFilename = otherFilename.left(filename.length());

				if (filename == trimmedOtherFilename)
				{
					lastPeriodIndex = otherFilename.lastIndexOf(QChar('.'));
					QString shortOtherFilename;

					if (lastPeriodIndex >= 0)
						shortOtherFilename = otherFilename.left(lastPeriodIndex);
					else
						shortOtherFilename = otherFilename;

					QRegExp renameExp("-[0-9]+");
					
					if (renameExp.lastIndexIn(shortOtherFilename) == shortFilename.length())
					{
						unsigned int trailingInteger = shortOtherFilename.mid(shortFilename.length() + 1).toUInt(&validIndexOffset);

						if (!validIndexOffset)
							break;

						if (trailingInteger > renameIndexOffset)
							renameIndexOffset = trailingInteger;
					}
				}
			}
		}

		if (validIndexOffset)
		{
			// Ensure renaming won't involve integer overflow!
			if (renameIndex > static_cast<unsigned int>(-1) - renameIndexOffset)
				validIndexOffset = false;
		}

		if (validIndexOffset)
		{
			shortFilename.append(QChar('-'));
			shortFilename.append(QString::number(renameIndexOffset + renameIndex));

			return (shortFilename + fileType);
		}
		else
		{
			// Fallback behaviour, brute-force/semi-random.
			bool valid;
			QString filename;

			do
			{
				valid = true;

				filename = shortFilename + "-";
				for (int i = 0; i < 8; i++)
					filename.append(QChar(qrand() % ('Z' - 'A' + 1) + 'A'));

				for (int i = 0; i < fileInfos.length(); i++)
				{
					int lastSlash = fileInfos[i].GetFilename().lastIndexOf('/');

					if (lastSlash < 0)
						lastSlash = fileInfos[i].GetFilename().lastIndexOf('\\');

					if (filename == fileInfos[i].GetFilename().mid(lastSlash + 1))
					{
						valid = false;
						break;
					}
				}
			} while (!valid);

			return (filename);
		}
	}
	else
	{
		return (filename);
	}
}
