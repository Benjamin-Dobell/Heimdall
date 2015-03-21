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

#ifndef PACKAGEDATA_H
#define PACKAGEDATA_H

// Qt
#include <QDir>

// Heimdall Frontend
#include "FirmwareInfo.h"

namespace HeimdallFrontend
{
	class PackageData : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(HeimdallFrontend::FirmwareInfo *firmwareInfo READ GetFirmwareInfo)
		Q_PROPERTY(QList<QString> filePaths READ GetFilePaths)

		private:

			FirmwareInfo firmwareInfo;
			QList<QString> filePaths;
			QList<QDir> ownedDirectories;

			QString findPit(void) const
			{
				QString pitIdentifier = firmwareInfo.GetPitFilename();

				if (pitIdentifier.length() == 0)
				{
					pitIdentifier = ".pit";
				}

				for (const QString& path : filePaths)
				{
					if (path.endsWith(pitIdentifier))
					{
						return path;
					}
				}

				return QLatin1String("");
			}

		public:

			static void Register(void);

			PackageData();
			~PackageData();

			void Clear(void);
			bool ReadFirmwareInfo(const QString& path);

			bool IsCleared(void) const;

			const FirmwareInfo *GetFirmwareInfo(void) const
			{
				return (&firmwareInfo);
			}

			FirmwareInfo *GetFirmwareInfo(void)
			{
				return (&firmwareInfo);
			}

			const QList<QString>& GetFilePaths(void) const
			{
				return (filePaths);
			}

			QList<QString>& GetFilePaths(void)
			{
				return (filePaths);
			}

			const QList<QDir>& GetOwnedDirectories(void) const
			{
				return (ownedDirectories);
			}

			QList<QDir>& GetOwnedDirectories(void)
			{
				return (ownedDirectories);
			}

			bool TakeFrom(PackageData& other, QString& failureReason, bool dryRun = false)
			{
				if (!firmwareInfo.IsCleared() && !other.firmwareInfo.IsCleared())
				{
					failureReason = QLatin1String("Only one firmware.xml can be used at a time");
					return (false);
				}

				if (findPit().length() > 0 && other.findPit().length() > 0)
				{
					failureReason = QLatin1String("Only one PIT file can be used at a time");
					return (false);
				}

				if (!dryRun)
				{
					if (firmwareInfo.IsCleared())
					{
						firmwareInfo.CopyFrom(other.firmwareInfo);
					}

					filePaths.append(other.filePaths);
					ownedDirectories.append(other.ownedDirectories);

					other.ownedDirectories.clear();

					firmwareInfo.CopyFrom(other.firmwareInfo);
				}

				return (true);
			}
	};
}

Q_DECLARE_METATYPE(HeimdallFrontend::PackageData *)

#endif
