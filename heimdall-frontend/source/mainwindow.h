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
	enum class HeimdallState : int
	{
		Stopped = 1,
		Flashing = (int)Stopped << 1,
		DetectingDevice = (int)Flashing << 1,
		ClosingPcScreen = (int)DetectingDevice << 1,
		PrintingPit = (int)ClosingPcScreen << 1,
		DownloadingPit = (int)PrintingPit << 1,
		NoReboot = (int)DownloadingPit << 1
	};

	inline HeimdallState operator|(HeimdallState lhs, HeimdallState rhs)
	{
		return (HeimdallState)((int)lhs | (int)rhs);
	}

	inline HeimdallState& operator|=(HeimdallState& lhs, HeimdallState rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	inline HeimdallState operator&(HeimdallState lhs, HeimdallState rhs)
	{
		lhs = (HeimdallState)((int)lhs & (int)rhs);
		return lhs;
	}

	inline bool operator!(HeimdallState state)
	{
		return (int)state == 0;
	}

	class MainWindow : public QMainWindow, public Ui::MainWindow
	{
		Q_OBJECT

		private:

			enum
			{
				kPrintPitSourceDevice = 0,
				kPrintPitSourceLocalFile
			};

			AboutForm aboutForm;
		
			QString lastDirectory;

			int tabIndex;

			bool heimdallFailed;
			HeimdallState heimdallState;
			QProcess heimdallProcess;

			PackageData loadedPackageData;
			
			PitData currentPitData;
			PackageData workingPackageData;

			bool populatingPartitionNames;
			QList<unsigned int> unusedPartitionIds;

			bool verboseOutput;
			bool resume;


			void StartHeimdall(const QStringList& arguments);

			void UpdateUnusedPartitionIds(void);
			bool ReadPit(QFile *file);

			void UpdatePackageUserInterface(void);

			bool IsArchive(QString path);

			QString PromptFileSelection(const QString& caption = QString("Select File"), const QString& filter = QString());
			QString PromptFileCreation(const QString& caption = QString("Save File"), const QString& filter = QString());
			
			void UpdateLoadPackageInterfaceAvailability(void);
			void UpdateFlashInterfaceAvailability(void);
			void UpdateCreatePackageInterfaceAvailability(void);
			void UpdateUtilitiesInterfaceAvailability(void);
			void UpdateInterfaceAvailability(void);

			void UpdatePartitionNamesInterface(void);

		public:

			explicit MainWindow(QWidget *parent = 0);
			~MainWindow();

		public slots:

			void OpenDonationWebpage(void);
			void SetVerboseOutput(bool enabled);
			void ShowAbout(void);

			void FunctionTabChanged(int index);

			// Load Package Tab
			void SelectFirmwarePackage(void);
			void OpenDeveloperHomepage(void);
			void OpenDeveloperDonationWebpage(void);
			void LoadFirmwarePackage(void);

			// Flash Tab
			void SelectPartitionName(int index);
			void SelectPartitionFile(void);

			void SelectPartition(int row);
			void AddPartition(void);
			void RemovePartition(void);

			void SelectPit(void);

			void SetRepartition(int enabled);

			void SetNoReboot(int enabled);
			void SetResume(bool enabled);
			void SetResume(int enabled);

			void StartFlash(void);

			// Create Package Tab
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

			// Utilities Tab
			void DetectDevice(void);
			void ClosePcScreen(void);

			void SelectPitDestination(void);
			void DownloadPit(void);

			void DevicePrintPitToggled(bool checked);
			void LocalFilePrintPitToggled(bool checked);
			void SelectPrintPitFile(void);
			void PrintPit(void);

			// Heimdall Command Line
			void HandleHeimdallStdout(void);
			void HandleHeimdallReturned(int exitCode, QProcess::ExitStatus exitStatus);
			void HandleHeimdallError(QProcess::ProcessError error);
	};
}

#endif // MAINWINDOW_H
