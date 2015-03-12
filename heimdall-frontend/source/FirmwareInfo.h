/* Copyright (c) 2010-2014 Benjamin Dobell, Glass Echidna

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
#include <QQmlListProperty>
#include <QString>
#include <QXmlStreamReader>

namespace HeimdallFrontend
{
	class DeviceInfo : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(QString manufacturer READ GetManufacturer)
		Q_PROPERTY(QString product READ GetProduct)
		Q_PROPERTY(QString name READ GetName)

		private:

			QString manufacturer;
			QString product;
			QString name;

		public:

			static void Register(void);

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

	class PlatformInfo : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(QString name READ GetName)
		Q_PROPERTY(QString version READ GetVersion)

		private:

			QString name;
			QString version;

		public:

			static void Register(void);

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

	class FileInfo : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(int partitionId READ GetPartitionId)
		Q_PROPERTY(QString filename READ GetFilename)

		private:

			unsigned int partitionId;
			QString filename;

		public:

			static void Register(void);

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

	class FirmwareInfo : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(QString name READ GetName)
		Q_PROPERTY(QString version READ GetVersion)
		Q_PROPERTY(HeimdallFrontend::PlatformInfo *platformInfo READ GetPlatformInfo)

		Q_PROPERTY(QList<QString> developers READ GetDevelopers)
		Q_PROPERTY(QString url READ GetUrl)
		Q_PROPERTY(QString donateUrl READ GetDonateUrl)

		Q_PROPERTY(QQmlListProperty<HeimdallFrontend::DeviceInfo> deviceInfos READ GetDeviceInfosListProperty)

		Q_PROPERTY(QString pitFilename READ GetPitFilename)
		Q_PROPERTY(bool repartition READ GetRepartition)

		Q_PROPERTY(bool noReboot READ GetNoReboot)

		Q_PROPERTY(QQmlListProperty<HeimdallFrontend::FileInfo> fileInfos READ GetFileInfosListProperty)

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

			QList<DeviceInfo *> deviceInfos;

			QString pitFilename;
			bool repartition;

			bool noReboot;

			QList<FileInfo *> fileInfos;

			static void DeviceInfoAppend(QQmlListProperty<DeviceInfo> *property, DeviceInfo *deviceInfo)
			{
				qobject_cast<FirmwareInfo *>(property->object)->deviceInfos.append(deviceInfo);
			}

			static int DeviceInfoCount(QQmlListProperty<DeviceInfo> *property)
			{
				return qobject_cast<FirmwareInfo *>(property->object)->deviceInfos.length();
			}

			static DeviceInfo *DeviceInfoAtIndex(QQmlListProperty<DeviceInfo> *property, int index)
			{
				return qobject_cast<FirmwareInfo *>(property->object)->deviceInfos[index];
			}

			static void DeviceInfoClearAll(QQmlListProperty<DeviceInfo> *property)
			{
				qobject_cast<FirmwareInfo *>(property->object)->deviceInfos.clear();
			}

			static void FileInfoAppend(QQmlListProperty<FileInfo> *property, FileInfo *fileInfo)
			{
				qobject_cast<FirmwareInfo *>(property->object)->fileInfos.append(fileInfo);
			}

			static int FileInfoCount(QQmlListProperty<FileInfo> *property)
			{
				return qobject_cast<FirmwareInfo *>(property->object)->fileInfos.length();
			}

			static FileInfo *FileInfoAtIndex(QQmlListProperty<FileInfo> *property, int index)
			{
				return qobject_cast<FirmwareInfo *>(property->object)->fileInfos[index];
			}

			static void FileInfoClearAll(QQmlListProperty<FileInfo> *property)
			{
				qobject_cast<FirmwareInfo *>(property->object)->fileInfos.clear();
			}

		public:

			static void Register(void);

			FirmwareInfo();
			~FirmwareInfo();

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

			const PlatformInfo *GetPlatformInfo(void) const
			{
				return (&platformInfo);
			}

			PlatformInfo *GetPlatformInfo(void)
			{
				return (&platformInfo);
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

			const QList<DeviceInfo *>& GetDeviceInfos(void) const
			{
				return (deviceInfos);
			}

			QList<DeviceInfo *>& GetDeviceInfos(void)
			{
				return (deviceInfos);
			}

			QQmlListProperty<DeviceInfo> GetDeviceInfosListProperty(void)
			{
				return QQmlListProperty<DeviceInfo>{this, nullptr, &FirmwareInfo::DeviceInfoAppend, &FirmwareInfo::DeviceInfoCount,
					&FirmwareInfo::DeviceInfoAtIndex, &FirmwareInfo::DeviceInfoClearAll};
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

			const QList<FileInfo *>& GetFileInfos(void) const
			{
				return (fileInfos);
			}

			QList<FileInfo *>& GetFileInfos(void)
			{
				return (fileInfos);
			}

			QQmlListProperty<FileInfo> GetFileInfosListProperty(void)
			{
				return QQmlListProperty<FileInfo>{this, nullptr, &FirmwareInfo::FileInfoAppend, &FirmwareInfo::FileInfoCount,
					&FirmwareInfo::FileInfoAtIndex, &FirmwareInfo::FileInfoClearAll};
			}
	};
}

Q_DECLARE_METATYPE(HeimdallFrontend::DeviceInfo *)
Q_DECLARE_METATYPE(HeimdallFrontend::PlatformInfo *)
Q_DECLARE_METATYPE(HeimdallFrontend::FileInfo *)

#endif
