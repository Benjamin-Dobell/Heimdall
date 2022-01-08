/* Copyright (c) 2010-2022 Benjamin Dobell, Glass Echidna
 
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
#include <QTemporaryFile>

// Heimdall Frontend
#include "FirmwareInfo.h"

namespace HeimdallFrontend
{
	class PackageData
	{
		private:

			FirmwareInfo firmwareInfo;
			QList<QTemporaryFile *> files;

		public:

			PackageData();
			~PackageData();

			void Clear(void);
			bool ReadFirmwareInfo(QFile *file);

			bool IsCleared(void) const;

			const FirmwareInfo& GetFirmwareInfo(void) const
			{
				return (firmwareInfo);
			}

			FirmwareInfo& GetFirmwareInfo(void)
			{
				return (firmwareInfo);
			}

			const QList<QTemporaryFile *>& GetFiles(void) const
			{
				return (files);
			}

			QList<QTemporaryFile *>& GetFiles(void)
			{
				return (files);
			}

			// Simply clears the files list, it does delete/close any files.
			void RemoveAllFiles(void)
			{
				files.clear();
			}
	};
}

#endif
