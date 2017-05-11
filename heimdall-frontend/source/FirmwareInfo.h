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

#ifndef FIRMWAREINFO_H
#define FIRMWAREINFO_H

// Qt
#include <QFile>
#include <QString>
#include <QXmlStreamReader>

namespace HeimdallFrontend
{
	class DeviceInfo
	{		
		private:

			QString manufacturer;
			QString product;
			QString name;

		public:

			DeviceInfo();
			DeviceInfo(const QString& manufacturer, const QString& product, const QString& name);

			bool ParseXml(QXmlStreamReader& xml);
			void WriteXml(QXmlStreamWriter& xml) const;

			const QString& GetManufacturer(void) const
			{
				return (manufacturer);
			}

			void SetManufacturer(const QString& manufacturer)
			{
				this->manufacturer = manufacturer;
			}

			const QString& GetProduct(void) const
			{
				return (product);
			}

			void SetProduct(const QString& product)
			{
				this->product = product;
			}

			const QString& GetName(void) const
			{
				return (name);
			}

			void SetName(const QString& name)
			{
				this->name = name;
			}
	};

	class PlatformInfo
	{
		private:

			QString name;
			QString version;

		public:

			PlatformInfo();

			void Clear(void);
			bool IsCleared(void) const;

			bool ParseXml(QXmlStreamReader& xml);
			void WriteXml(QXmlStreamWriter& xml) const;

			const QString& GetName(void) const
			{
				return (name);
			}

			void SetName(const QString& name)
			{
				this->name = name;
			}

			const QString& GetVersion(void) const
			{
				return (version);
			}

			void SetVersion(const QString& version)
			{
				this->version = version;
			}
	};

	class FileInfo
	{
		private:

			unsigned int partitionId;
			QString filename;

		public:

			FileInfo();
			FileInfo(unsigned int partitionId, const QString& filename);

			bool ParseXml(QXmlStreamReader& xml);
			void WriteXml(QXmlStreamWriter& xml, const QString& filename) const;

			unsigned int GetPartitionId(void) const
			{
				return (partitionId);
			}

			void SetPartitionId(unsigned int partitionId)
			{
				this->partitionId = partitionId;
			}

			const QString& GetFilename(void) const
			{
				return (filename);
			}

			void SetFilename(const QString& filename)
			{
				this->filename = filename;
			}
	};

	class FirmwareInfo
	{
		public:

			enum
			{
				kVersion = 1
			};

		private:

			QString name;
			QString version;
			PlatformInfo platformInfo;

			QList<QString> developers;
			QString url;
			QString donateUrl;

			QList<DeviceInfo> deviceInfos;

			QString pitFilename;
			bool repartition;

			bool noReboot;

			QList<FileInfo> fileInfos;

		public:

			FirmwareInfo();

			void Clear(void);
			bool IsCleared(void) const;

			bool ParseXml(QXmlStreamReader& xml);
			void WriteXml(QXmlStreamWriter& xml) const;

			const QString& GetName(void) const
			{
				return (name);
			}

			void SetName(const QString& name)
			{
				this->name = name;
			}

			const QString& GetVersion(void) const
			{
				return (version);
			}

			void SetVersion(const QString& version)
			{
				this->version = version;
			}

			const PlatformInfo& GetPlatformInfo(void) const
			{
				return (platformInfo);
			}

			PlatformInfo& GetPlatformInfo(void)
			{
				return (platformInfo);
			}

			const QList<QString>& GetDevelopers(void) const
			{
				return (developers);
			}

			QList<QString>& GetDevelopers(void)
			{
				return (developers);
			}

			const QString& GetUrl(void) const
			{
				return (url);
			}

			void SetUrl(const QString& url)
			{
				this->url = url;
			}

			const QString& GetDonateUrl(void) const
			{
				return (donateUrl);
			}

			void SetDonateUrl(const QString& donateUrl)
			{
				this->donateUrl = donateUrl;
			}

			const QList<DeviceInfo>& GetDeviceInfos(void) const
			{
				return (deviceInfos);
			}

			QList<DeviceInfo>& GetDeviceInfos(void)
			{
				return (deviceInfos);
			}

			const QString& GetPitFilename(void) const
			{
				return (pitFilename);
			}

			void SetPitFilename(const QString& pitFilename)
			{
				this->pitFilename = pitFilename;
			}

			bool GetRepartition(void) const
			{
				return (repartition);
			}

			void SetRepartition(bool repartition)
			{
				this->repartition = repartition;
			}

			bool GetNoReboot(void) const
			{
				return (noReboot);
			}

			void SetNoReboot(bool noReboot)
			{
				this->noReboot = noReboot;
			}

			const QList<FileInfo>& GetFileInfos(void) const
			{
				return (fileInfos);
			}

			QList<FileInfo>& GetFileInfos(void)
			{
				return (fileInfos);
			}
	};
}

#endif
