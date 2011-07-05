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
					// TODO: "found multiple device manufacturers."
					return (false);
				}

				foundManufacturer = true;

				manufacturer = xml.readElementText();
			}
			else if (xml.name() == "product")
			{
				if (foundProduct)
				{
					// TODO: "found multiple device product identifiers."
					return (false);
				}

				foundProduct = true;

				product = xml.readElementText();
			}
			else if (xml.name() == "name")
			{
				if (foundName)
				{
					// TODO: "found multiple device names."));
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
					// TODO: "found multiple platform names."
					return (false);
				}

				foundName = true;

				name = xml.readElementText();
			}
			else if (xml.name() == "version")
			{
				if (foundVersion)
				{
					// TODO: "found multiple platform versions."
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
					// TODO: "found multiple file IDs."
					return (false);
				}

				foundId = true;

				partitionId = xml.readElementText().toInt();
			}
			else if (xml.name() == "filename")
			{
				if (foundFilename)
				{
					// TODO: "found multiple file filenames."
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



FirmwareInfo::FirmwareInfo()
{
	repartition = false;
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

	fileInfos.clear();
}

bool FirmwareInfo::IsCleared(void) const
{
	return (name.isEmpty() && version.isEmpty() && platformInfo.IsCleared() && developers.isEmpty() && url.isEmpty() && url.isEmpty() && donateUrl.isEmpty()
		&& deviceInfos.isEmpty() && pitFilename.isEmpty() && !repartition && fileInfos.isEmpty());
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
					// TODO: "found multiple firmware names."
					return (false);
				}

				foundName = true;
				name = xml.readElementText();
			}
			else if (xml.name() == "version")
			{
				if (foundVersion)
				{
					// TODO: "found multiple firmware versions."
					return (false);
				}

				foundVersion = true;
				version = xml.readElementText();
			}
			else if (xml.name() == "platform")
			{
				if (foundPlatform)
				{
					// TODO: "found multiple firmware platforms."
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
					// TODO: "found multiple sets of firmware developers."
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
					// TODO: "found multiple firmware URLs."
					return (false);
				}

				foundUrl = true;

				url = xml.readElementText();
			}
			else if (xml.name() == "donateurl")
			{
				if (foundDonateUrl)
				{
					// TODO: "found multiple firmware donate URLs."
					return (false);
				}

				foundDonateUrl = true;

				donateUrl = xml.readElementText();
			}
			else if (xml.name() == "devices")
			{
				if (foundDevices)
				{
					// TODO: "found multiple sets of firmware devices."
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
					// TODO: "found multiple firmware PIT files."
					return (false);
				}

				foundPit = true;

				pitFilename = xml.readElementText();
			}
			else if (xml.name() == "repartition")
			{
				if (foundRepartition)
				{
					// TODO: "found multiple firmware repartition values."
					return (false);
				}

				foundRepartition = true;

				repartition = (xml.readElementText().toInt() != 0);
			}
			else if (xml.name() == "files")
			{
				if (foundFiles)
				{
					// TODO: "found multiple sets of firmware files."
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
				if (!(foundName && foundVersion && foundPlatform && foundDevelopers && foundDevices && foundPit && foundRepartition && foundFiles))
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
