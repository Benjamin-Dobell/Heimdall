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

// Qt
#include "QRegExp"

// Heimdall Frontend
#include "Alerts.h"
#include "FirmwareInfo.h"
#include "Packaging.h"

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
					Alerts::DisplayError("Found multiple <manufacturer> elements in <device>.");
					return (false);
				}

				foundManufacturer = true;

				manufacturer = xml.readElementText();
			}
			else if (xml.name() == "product")
			{
				if (foundProduct)
				{
					Alerts::DisplayError("Found multiple <product> elements in <device>.");
					return (false);
				}

				foundProduct = true;

				product = xml.readElementText();
			}
			else if (xml.name() == "name")
			{
				if (foundName)
				{
					Alerts::DisplayError("Found multiple <name> elements in <device>.");
					return (false);
				}

				foundName = true;

				name = xml.readElementText();
			}
			else
			{
				Alerts::DisplayError(QString("<%1> is not a valid child of <device>.").arg(xml.name().toString()));
				return (false);
			}
		}
		else if (nextToken == QXmlStreamReader::EndElement)
		{
			if (xml.name() == "device")
			{
				if (foundManufacturer && foundProduct && foundName)
				{
					return (true);
				}
				else
				{
					Alerts::DisplayError("Required elements are missing from <device>.");
					return (false);
				}
			}
		}
		else
		{
			if (!(nextToken == QXmlStreamReader::Characters && xml.isWhitespace()))
			{
				Alerts::DisplayError("Unexpected token found in <device>.");
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
					Alerts::DisplayError("Found multiple <name> elements in <platform>.");
					return (false);
				}

				foundName = true;

				name = xml.readElementText();
			}
			else if (xml.name() == "version")
			{
				if (foundVersion)
				{
					Alerts::DisplayError("Found multiple <version> elements in <platform>.");
					return (false);
				}

				foundVersion = true;

				version = xml.readElementText();
			}
			else
			{
				Alerts::DisplayError(QString("<%1> is not a valid child of <platform>.").arg(xml.name().toString()));
				return (false);
			}
		}
		else if (nextToken == QXmlStreamReader::EndElement)
		{
			if (xml.name() == "platform")
			{
				if (foundName && foundVersion)
				{
					return (true);
				}
				else
				{
					Alerts::DisplayError("Required elements are missing from <platform>.");
					return (false);
				}
			}
		}
		else
		{
			if (!(nextToken == QXmlStreamReader::Characters && xml.isWhitespace()))
			{
				Alerts::DisplayError("Unexpected token found in <platform>.");
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
					Alerts::DisplayError("Found multiple <id> elements in <file>.");
					return (false);
				}

				foundId = true;

				partitionId = xml.readElementText().toInt();
			}
			else if (xml.name() == "filename")
			{
				if (foundFilename)
				{
					Alerts::DisplayError("Found multiple <filename> elements in <file>.");
					return (false);
				}

				foundFilename = true;

				filename = xml.readElementText();
			}
			else
			{
				Alerts::DisplayError(QString("<%1> is not a valid child of <file>.").arg(xml.name().toString()));
				return (false);
			}
		}
		else if (nextToken == QXmlStreamReader::EndElement)
		{
			if (xml.name() == "file")
			{
				if (foundId && foundFilename)
				{
					return (true);
				}
				else
				{
					Alerts::DisplayError("Required elements are missing from <file>.");
					return (false);
				}
			}
		}
		else
		{
			if (!(nextToken == QXmlStreamReader::Characters && xml.isWhitespace()))
			{
				Alerts::DisplayError("Unexpected token found in <file>.");
				return (false);
			}
		}
	}

	return (false);
}

void FileInfo::WriteXml(QXmlStreamWriter& xml, const QString& filename) const
{
	xml.writeStartElement("file");

	xml.writeStartElement("id");
	xml.writeCharacters(QString::number(partitionId));
	xml.writeEndElement();

	xml.writeStartElement("filename");
	xml.writeCharacters(filename);
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
		Alerts::DisplayError("Failed to find <firmware> element.");
		return (false);
	}

	if (xml.name() != "firmware")
	{
		Alerts::DisplayError(QString("Expected <firmware> element but found <%1>.").arg(xml.name().toString()));
		return (false);
	}

	QString formatVersionString;
	formatVersionString += xml.attributes().value("version");

	if (formatVersionString.isEmpty())
	{
		Alerts::DisplayError("<firmware> is missing the version attribute.");
		return (false);
	}

	bool parsedVersion = false;
	int formatVersion = formatVersionString.toInt(&parsedVersion);

	if (!parsedVersion)
	{
		Alerts::DisplayError("<firmware> contains a malformed version.");
		return (false);
	}

	if (formatVersion > kVersion)
	{
		Alerts::DisplayError("Package is for a newer version of Heimdall Frontend.\nPlease download the latest version of Heimdall Frontend.");
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
					Alerts::DisplayError("Found multiple <name> elements in <firmware>.");
					return (false);
				}

				foundName = true;
				name = xml.readElementText();
			}
			else if (xml.name() == "version")
			{
				if (foundVersion)
				{
					Alerts::DisplayError("Found multiple <version> elements in <firmware>.");
					return (false);
				}

				foundVersion = true;
				version = xml.readElementText();
			}
			else if (xml.name() == "platform")
			{
				if (foundPlatform)
				{
					Alerts::DisplayError("Found multiple <platform> elements in <firmware>.");
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
					Alerts::DisplayError("Found multiple <developers> elements in <firmware>.");
					return (false);
				}

				foundDevelopers = true;

				while (!xml.atEnd())
				{
					nextToken = xml.readNext();

					if (nextToken == QXmlStreamReader::StartElement)
					{
						if (xml.name() == "name")
						{
							developers.append(xml.readElementText());
						}
						else
						{
							Alerts::DisplayError(QString("<%1> is not a valid child of <developers>.").arg(xml.name().toString()));
							return (false);
						}
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
							Alerts::DisplayError("Unexpected token found in <developers>.");
							return (false);
						}
					}
				}
			}
			else if (xml.name() == "url")
			{
				if (foundUrl)
				{
					Alerts::DisplayError("Found multiple <url> elements in <firmware>.");
					return (false);
				}

				foundUrl = true;

				url = xml.readElementText();
			}
			else if (xml.name() == "donateurl")
			{
				if (foundDonateUrl)
				{
					Alerts::DisplayError("Found multiple <donateurl> elements in <firmware>.");
					return (false);
				}

				foundDonateUrl = true;

				donateUrl = xml.readElementText();
			}
			else if (xml.name() == "devices")
			{
				if (foundDevices)
				{
					Alerts::DisplayError("Found multiple <devices> elements in <firmware>.");
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
						else
						{
							Alerts::DisplayError(QString("<%1> is not a valid child of <devices>.").arg(xml.name().toString()));
							return (false);
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
							Alerts::DisplayError("Unexpected token found in <devices>.");
							return (false);
						}
					}
				}
			}
			else if (xml.name() == "pit")
			{
				if (foundPit)
				{
					Alerts::DisplayError("Found multiple <pit> elements in <firmware>.");
					return (false);
				}

				foundPit = true;

				pitFilename = xml.readElementText();
			}
			else if (xml.name() == "repartition")
			{
				if (foundRepartition)
				{
					Alerts::DisplayError("Found multiple <repartition> elements in <firmware>.");
					return (false);
				}

				foundRepartition = true;

				repartition = (xml.readElementText().toInt() != 0);
			}
			else if (xml.name() == "noreboot")
			{
				if (foundNoReboot)
				{
					Alerts::DisplayError("Found multiple <noreboot> elements in <firmware>.");
					return (false);
				}

				foundNoReboot = true;

				noReboot = (xml.readElementText().toInt() != 0);
			}
			else if (xml.name() == "files")
			{
				if (foundFiles)
				{
					Alerts::DisplayError("Found multiple <files> elements in <firmware>.");
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
						else
						{
							Alerts::DisplayError(QString("<%1> is not a valid child of <files>.").arg(xml.name().toString()));
							return (false);
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
							Alerts::DisplayError("Unexpected token found in <devices>.");
							return (false);
						}
					}
				}
			}
			else
			{
				Alerts::DisplayError(QString("<%1> is not a valid child of <firmware>.").arg(xml.name().toString()));
				return (false);
			}
		}
		else if (nextToken == QXmlStreamReader::EndElement)
		{
			if (xml.name() == "firmware")
			{
				if (!(foundName && foundVersion && foundPlatform && foundDevelopers && foundDevices && foundPit && foundRepartition && foundNoReboot && foundFiles))
				{
					Alerts::DisplayError("Required elements are missing from <firmware>.");
					return (false);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			if (!(nextToken == QXmlStreamReader::Characters && xml.isWhitespace()))
			{
				Alerts::DisplayError("Unexpected token found in <firmware>.");
				return (false);
			}
		}
	}

	// Read whitespaces at the end of the file (if there are any)
	xml.readNext();

	if (!xml.atEnd())
	{
		Alerts::DisplayError("Found data after </firmware>.");
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
	{
		fileInfos[i].WriteXml(xml, Packaging::ClashlessFilename(fileInfos, i));
	}

	xml.writeEndElement();

	xml.writeEndElement();
	xml.writeEndDocument();
}
