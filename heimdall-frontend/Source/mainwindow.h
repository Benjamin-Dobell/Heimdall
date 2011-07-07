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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt
#include <QList>
#include <QMainWindow>
#include <QProcess>
#include <QTemporaryFile>

// libpit
#include "libpit.h"

// Heimdall Frontend
#include "aboutform.h"
#include "ui_mainwindow.h"
#include "PackageData.h"

using namespace libpit;

namespace HeimdallFrontend
{
	class MainWindow : public QMainWindow, public Ui::MainWindow
	{
		Q_OBJECT

		private:

			AboutForm aboutForm;
		
			QString lastDirectory;

			bool heimdallFailed;
			bool heimdallRunning;
			QProcess process;

			PackageData loadedPackageData;
			
			PitData currentPitData;
			PackageData workingPackageData;

			bool populatingPartitionNames;
			QList<unsigned int> unusedPartitionIds;

			void UpdateUnusedPartitionIds(void);
			bool ReadPit(QFile *file);

			void UpdatePackageUserInterface(void);

			bool IsArchive(QString path);

			QString PromptFileSelection(void);
			QString PromptFileCreation(void);

			void UpdatePartitionNamesInterface(void);
			void UpdateStartButton(void);

			void UpdateBuildPackageButton(void);

		public:

			explicit MainWindow(QWidget *parent = 0);
			~MainWindow();

		public slots:

			void OpenDonationWebpage(void);
			void ShowAbout(void);

			void SelectFirmwarePackage(void);
			void OpenDeveloperHomepage(void);
			void OpenDeveloperDonationWebpage(void);
			void LoadFirmwarePackage(void);

			void SelectPartitionName(int index);
			void SelectPartitionFile(void);

			void SelectPartition(int row);
			void AddPartition(void);
			void RemovePartition(void);

			void SelectPit(void);

			void SetRepartition(int enabled);
			void SetNoReboot(int enabled);

			void StartFlash(void);

			void FirmwareNameChanged(const QString& text);
			void FirmwareVersionChanged(const QString& text);
			void PlatformNameChanged(const QString& text);
			void PlatformVersionChanged(const QString& text);

			void HomepageUrlChanged(const QString& text);
			void DonateUrlChanged(const QString& text);

			void DeveloperNameChanged(const QString& text);
			void SelectDeveloper(int row);
			void AddDeveloper(void);
			void RemoveDeveloper(void);

			void DeviceInfoChanged(const QString& text);
			void SelectDevice(int row);
			void AddDevice(void);
			void RemoveDevice(void);
			
			void BuildPackage(void);

			void HandleHeimdallStdout(void);
			void HandleHeimdallReturned(int exitCode, QProcess::ExitStatus exitStatus);
			void HandleHeimdallError(QProcess::ProcessError error);
	};
}

#endif // MAINWINDOW_H
