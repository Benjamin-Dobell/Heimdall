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

// Heimdall Frontend
#include "aboutform.h"
#include "ui_mainwindow.h"

// Qt
#include <QMainWindow>
#include <QProcess>

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

			bool IsArchive(QString path);

			QString PromptFileSelection(void);
			void UpdateStartButton(void);

		public:

			explicit MainWindow(QWidget *parent = 0);
			~MainWindow();

		public slots:

			void OpenDonationWebpage(void);
			void ShowAbout(void);

			void SelectPit(void);
			void SelectFactoryfs(void);
			void SelectKernel(void);
			void SelectParam(void);
			void SelectPrimaryBootloader(void);
			void SelectSecondaryBootloader(void);
			void SelectCache(void);
			void SelectDatabase(void);
			void SelectModem(void);
			void SelectRecovery(void);

			void SetRepartionEnabled(int enabled);
			void SetFactoryfsEnabled(int enabled);
			void SetKernelEnabled(int enabled);
			void SetParamEnabled(int enabled);
			void SetPrimaryBootloaderEnabled(int enabled);
			void SetSecondaryBootloaderEnabled(int enabled);
			void SetCacheEnabled(int enabled);
			void SetDatabaseEnabled(int enabled);
			void SetModemEnabled(int enabled);
			void SetRecoveryEnabled(int enabled);

			void StartFlash(void);

			void HandleHeimdallStdout(void);
			void HandleHeimdallReturned(int exitCode, QProcess::ExitStatus exitStatus);
			void HandleHeimdallError(QProcess::ProcessError error);
	};
}

#endif // MAINWINDOW_H
