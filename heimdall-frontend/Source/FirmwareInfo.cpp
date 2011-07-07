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

// Heimdall Frontend
#include "FirmwareInfo.h"

using namespace HeimdallFrontend;

DeviceInfo::DeviceInfo()
{
}

DeviceInfo::DeviceInfo(const QString& manufacturer, const QString& product, const QString& name)
{
	this->manufacturer = manufacturer;
	this->product = product;
	this->name = name;
}

bool DeviceInfo::ParseXml(QXmlStreamReader& xml)
{
	bool foundManufacturer = false;
	bool foundProduct = false;
	bool foundName = false;

	while (!xml.atEnd())
	{
		QXmlStreamReader::TokenType nextToken = xml.readNext();

		if (nextToken == QXmlStreamReader::StartElement)
		{
			if (xml.name() == "manufacturer")
			{
				if (foundManufacturer)
				{
					// TODO: "Found multiple device manufacturers."
					return (false);
				}

				foundManufacturer = true;

				manufacturer = xml.readElementText();
			}
			else if (xml.name() == "product")
			{
				if (foundProduct)
				{
					// TODO: "Found multiple device product identifiers."
					return (false);
				}

				foundProduct = true;

				product = xml.readElementText();
			}
			else if (xml.name() == "name")
			{
				if (foundName)
				{
					// TODO: "Found multiple device names."));
					return (false);
				}

				foundName = true;

				name = xml.readElementText();
			}
		}
		else if (nextToken == QXmlStreamReader::EndElement)
		{
			if (xml.name() == "device")
				return (foundManufacturer && foundProduct && foundName);
		}
		else
		{
			if (!(nextToken == QXmlStreamReader::Characters && xml.isWhitespace()))
			{
				// TODO: "Unexpected token found in <device>"
				return (false);
			}
		}
	}

	return (false);
}

void DeviceInfo::WriteXml(QXmlStreamWriter& xml) const
{
	xml.writeStartElement("device");

	xml.writeStartElement("manufacturer");
	xml.writeCharacters(manufacturer);
	xml.writeEndElement();

	xml.writeStartElement("product");
	xml.writeCharacters(product);
	xml.writeEndElement();

	xml.writeStartElement("name");
	xml.writeCharacters(name);
	xml.writeEndElement();

	xml.writeEndElement();
}



PlatformInfo::PlatformInfo()
{
}

void PlatformInfo::Clear(void)
{
	name.clear();
	version.clear();
}

bool PlatformInfo::IsCleared(void) const
{
	return (name.isEmpty() && version.isEmpty());
}

bool PlatformInfo::ParseXml(QXmlStreamReader& xml)
{
	Clear();

	bool foundName = false;
	bool foundVersion = false;

	while (!xml.atEnd())
	{
		QXmlStreamReader::TokenType nextToken = xml.readNext();

		if (nextToken == QXmlStreamReader::StartElement)
		{
			if (xml.name() == "name")
			{
				if (foundName)
				{
					// TODO: "Found multiple platform names."
					return (false);
				}

				foundName = true;

				name = xml.readElementText();
			}
			else if (xml.name() == "version")
			{
				if (foundVersion)
				{
					// TODO: "Found multiple platform versions."
					return (false);
				}

				foundVersion = true;

				version = xml.readElementText();
			}
			else
			{
				// TODO: "found unknown <platform> sub-element <" + xml.name() + ">."
				return (false);
			}
		}
		else if (nextToken == QXmlStreamReader::EndElement)
		{
			if (xml.name() == "platform")
				return (foundName && foundVersion);
		}
		else
		{
			if (!(nextToken == QXmlStreamReader::Characters && xml.isWhitespace()))
			{
				// TODO: "Unexpected token found in <platform>"
				return (false);
			}
		}
	}

	return (false);
}

void PlatformInfo::WriteXml(QXmlStreamWriter& xml) const
{
	xml.writeStartElement("platform");

	xml.writeStartElement("name");
	xml.writeCharacters(name);
	xml.writeEndElement();

	xml.writeStartElement("version");
	xml.writeCharacters(version);
	xml.writeEndElement();

	xml.writeEndElement();
}



FileInfo::FileInfo()
{
}

FileInfo::FileInfo(unsigned int partitionId, const QString& filename)
{
	this->partitionId = partitionId;
	this->filename = filename;
}

bool FileInfo::ParseXml(QXmlStreamReader& xml)
{
	bool foundId = false;
	bool foundFilename = false;

	while (!xml.atEnd())
	{
		QXmlStreamReader::TokenType nextToken = xml.readNext();

		if (nextToken == QXmlStreamReader::StartElement)
		{
			if (xml.name() == "id")
			{
				if (foundId)
				{
					// TODO: "Found multiple file IDs."
					return (false);
				}

				foundId = true;

				partitionId = xml.readElementText().toInt();
			}
			else if (xml.name() == "filename")
			{
				if (foundFilename)
				{
					// TODO: "Found multiple file filenames."
					return (false);
				}

				foundFilename = true;

				filename = xml.readElementText();
			}
		}
		else if (nextToken == QXmlStreamReader::EndElement)
		{
			if (xml.name() == "file")
				return (foundId && foundFilename);
		}
		else
		{
			if (!(nextToken == QXmlStreamReader::Characters && xml.isWhitespace()))
			{
				// TODO: "Unexpected token found in <file>"
				return (false);
			}
		}
	}

	return (false);
}

void FileInfo::WriteXml(QXmlStreamWriter& xml) const
{
	xml.writeStartElement("file");

	xml.writeStartElement("id");
	xml.writeCharacters(QString::number(partitionId));
	xml.writeEndElement();

	xml.writeStartElement("filename");

	int lastSlash = filename.lastIndexOf('/');

	if (lastSlash < 0)
		lastSlash = filename.lastIndexOf('\\');

	xml.writeCharacters(filename.mid(lastSlash + 1));

	xml.writeEndElement();

	xml.writeEndElement();
}



FirmwareInfo::FirmwareInfo()
{
	repartition = false;
	noReboot = false;
}

void FirmwareInfo::Clear(void)
{
	name = "";
	version = "";
	platformInfo.Clear();

	developers.clear();
	url.clear();
	donateUrl.clear();

	deviceInfos.clear();

	pitFilename.clear();
	repartition = false;

	noReboot = false;

	fileInfos.clear();
}

bool FirmwareInfo::IsCleared(void) const
{
	return (name.isEmpty() && version.isEmpty() && platformInfo.IsCleared() && developers.isEmpty() && url.isEmpty() && url.isEmpty() && donateUrl.isEmpty()
		&& deviceInfos.isEmpty() && pitFilename.isEmpty() && !repartition && !noReboot && fileInfos.isEmpty());
}

bool FirmwareInfo::ParseXml(QXmlStreamReader& xml)
{
	Clear();

	bool foundName = false;
	bool foundVersion = false;
	bool foundPlatform = false;
	bool foundDevelopers = false;
	bool foundUrl = false;
	bool foundDonateUrl = false;
	bool foundDevices = false;
	bool foundPit = false;
	bool foundRepartition = false;
	bool foundNoReboot = false;
	bool foundFiles = false;

	if (!xml.readNextStartElement())
	{
		// TODO: "Failed to find <firmware> element."
		return (false);
	}

	if (xml.name() != "firmware")
	{
		// TODO: "Expected <firmware> element but found <%s>"
		return (false);
	}

	QString formatVersionString;
	formatVersionString += xml.attributes().value("version");

	if (formatVersionString.isEmpty())
	{
		// TODO: <firmware> is missing a version."
		return (false);
	}

	bool parsedVersion = false;
	int formatVersion = formatVersionString.toInt(&parsedVersion);

	if (!parsedVersion)
	{
		// TODO: "<firmware> contains a malformed version."
		return (false);
	}

	if (formatVersion > kVersion)
	{
		// TODO: "Package is for a newer version of Heimdall Frontend. Please download the latest version of Heimdall Frontend."
		return (false);
	}

	while (!xml.atEnd())
	{
		QXmlStreamReader::TokenType nextToken = xml.readNext();

		if (nextToken == QXmlStreamReader::StartElement)
		{
			if (xml.name() == "name")
			{
				if (foundName)
				{
					// TODO: "Found multiple firmware names."
					return (false);
				}

				foundName = true;
				name = xml.readElementText();
			}
			else if (xml.name() == "version")
			{
				if (foundVersion)
				{
					// TODO: "Found multiple firmware versions."
					return (false);
				}

				foundVersion = true;
				version = xml.readElementText();
			}
			else if (xml.name() == "platform")
			{
				if (foundPlatform)
				{
					// TODO: "Found multiple firmware platforms."
					return (false);
				}

				foundPlatform = true;

				if (!platformInfo.ParseXml(xml))
					return (false);
			}
			else if (xml.name() == "developers")
			{
				if (foundDevelopers)
				{
					// TODO: "Found multiple sets of firmware developers."
					return (false);
				}

				foundDevelopers = true;

				while (!xml.atEnd())
				{
					nextToken = xml.readNext();

					if (nextToken == QXmlStreamReader::StartElement)
					{
						if (xml.name() == "name")
							developers.append(xml.readElementText());
					}
					else if (nextToken == QXmlStreamReader::EndElement)
					{
						if (xml.name() == "developers")
							break;
					}
					else
					{
						if (!(nextToken == QXmlStreamReader::Characters && xml.isWhitespace()))
						{
							// TODO: "Unexpected token found in <developers>"
							return (false);
						}
					}
				}
			}
			else if (xml.name() == "url")
			{
				if (foundUrl)
				{
					// TODO: "Found multiple firmware URLs."
					return (false);
				}

				foundUrl = true;

				url = xml.readElementText();
			}
			else if (xml.name() == "donateurl")
			{
				if (foundDonateUrl)
				{
					// TODO: "Found multiple firmware donate URLs."
					return (false);
				}

				foundDonateUrl = true;

				donateUrl = xml.readElementText();
			}
			else if (xml.name() == "devices")
			{
				if (foundDevices)
				{
					// TODO: "Found multiple sets of firmware devices."
					return (false);
				}

				foundDevices = true;

				while (!xml.atEnd())
				{
					nextToken = xml.readNext();

					if (nextToken == QXmlStreamReader::StartElement)
					{
						if (xml.name() == "device")
						{
							DeviceInfo deviceInfo;

							if (!deviceInfo.ParseXml(xml))
								return (false);

							deviceInfos.append(deviceInfo);
						}
					}
					else if (nextToken == QXmlStreamReader::EndElement)
					{
						if (xml.name() == "devices")
							break;
					}
					else
					{
						if (!(nextToken == QXmlStreamReader::Characters && xml.isWhitespace()))
						{
							// TODO: "Unexpected token found in <devices>"
							return (false);
						}
					}
				}
			}
			else if (xml.name() == "pit")
			{
				if (foundPit)
				{
					// TODO: "Found multiple firmware PIT files."
					return (false);
				}

				foundPit = true;

				pitFilename = xml.readElementText();
			}
			else if (xml.name() == "repartition")
			{
				if (foundRepartition)
				{
					// TODO: "Found multiple firmware repartition values."
					return (false);
				}

				foundRepartition = true;

				repartition = (xml.readElementText().toInt() != 0);
			}
			else if (xml.name() == "noreboot")
			{
				if (foundNoReboot)
				{
					// TODO: "Found multiple firmware noreboot values."
					return (false);
				}

				foundNoReboot = true;

				noReboot = (xml.readElementText().toInt() != 0);
			}
			else if (xml.name() == "files")
			{
				if (foundFiles)
				{
					// TODO: "Found multiple sets of firmware files."
					return (false);
				}

				foundFiles = true;

				while (!xml.atEnd())
				{
					nextToken = xml.readNext();

					if (nextToken == QXmlStreamReader::StartElement)
					{
						if (xml.name() == "file")
						{
							FileInfo fileInfo;

							if (!fileInfo.ParseXml(xml))
								return (false);

							fileInfos.append(fileInfo);
						}
					}
					else if (nextToken == QXmlStreamReader::EndElement)
					{
						if (xml.name() == "files")
							break;
					}
					else
					{
						if (!(nextToken == QXmlStreamReader::Characters && xml.isWhitespace()))
						{
							// TODO: "Unexpected token found in <devices>"
							return (false);
						}
					}
				}
			}
			else
			{
				// TODO: "unknown <firmware> sub-element <" + xml.name() + ">."
				return (false);
			}
		}
		else if (nextToken == QXmlStreamReader::EndElement)
		{
			if (xml.name() == "firmware")
			{
				if (!(foundName && foundVersion && foundPlatform && foundDevelopers && foundDevices && foundPit && foundRepartition && foundNoReboot && foundFiles))
					return (false);
				else
					break;
			}
		}
		else
		{
			if (!(nextToken == QXmlStreamReader::Characters && xml.isWhitespace()))
			{
				// TODO: "Unexpected token found in <firmware>"
				return (false);
			}
		}
	}

	// Read whitespaces at the end of the file (if there are any)
	xml.readNext();

	if (!xml.atEnd())
	{
		// TODO: "Found data after </firmware>"
		return (false);
	}
	
	return (true);
}

void FirmwareInfo::WriteXml(QXmlStreamWriter& xml) const
{
	xml.writeStartDocument();
	xml.writeStartElement("firmware");
	xml.writeAttribute("version", QString::number(FirmwareInfo::kVersion));

	xml.writeStartElement("name");
	xml.writeCharacters(name);
	xml.writeEndElement();

	xml.writeStartElement("version");
	xml.writeCharacters(version);
	xml.writeEndElement();

	platformInfo.WriteXml(xml);

	xml.writeStartElement("developers");

	for (int i = 0; i < developers.length(); i++)
	{
		xml.writeStartElement("name");
		xml.writeCharacters(developers[i]);
		xml.writeEndElement();
	}

	xml.writeEndElement();

	if (!url.isEmpty())
	{
		xml.writeStartElement("url");
		xml.writeCharacters(url);
		xml.writeEndElement();
	}

	if (!donateUrl.isEmpty())
	{
		xml.writeStartElement("donateurl");
		xml.writeCharacters(donateUrl);
		xml.writeEndElement();
	}

	xml.writeStartElement("devices");

	for (int i = 0; i < deviceInfos.length(); i++)
		deviceInfos[i].WriteXml(xml);

	xml.writeEndElement();

	xml.writeStartElement("pit");

	int lastSlash = pitFilename.lastIndexOf('/');

	if (lastSlash < 0)
		lastSlash = pitFilename.lastIndexOf('\\');

	xml.writeCharacters(pitFilename.mid(lastSlash + 1));

	xml.writeEndElement();

	xml.writeStartElement("repartition");
	xml.writeCharacters((repartition) ? "1" : "0");
	xml.writeEndElement();

	xml.writeStartElement("noreboot");
	xml.writeCharacters((noReboot) ? "1" : "0");
	xml.writeEndElement();

	xml.writeStartElement("files");

	for (int i = 0; i < fileInfos.length(); i++)
		fileInfos[i].WriteXml(xml);

	xml.writeEndElement();

	xml.writeEndElement();
	xml.writeEndDocument();
}
