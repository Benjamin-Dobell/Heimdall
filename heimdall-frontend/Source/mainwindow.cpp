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
#include "mainwindow.h"

// Qt
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QProcess>
#include <QRegExp>
#include <QUrl>

using namespace HeimdallFrontend;

bool MainWindow::IsArchive(QString path)
{
	// Not a real check but hopefully it gets the message across, don't flash archives!
	return (path.endsWith(".tar", Qt::CaseInsensitive) || path.endsWith(".gz", Qt::CaseInsensitive) || path.endsWith(".zip", Qt::CaseInsensitive)
		|| path.endsWith(".bz2", Qt::CaseInsensitive) || path.endsWith(".7z", Qt::CaseInsensitive) || path.endsWith(".rar", Qt::CaseInsensitive));
}

QString MainWindow::PromptFileSelection(void)
{
	QString path = QFileDialog::getOpenFileName(this, "Select File", lastDirectory);

	if (path != "")
		lastDirectory = path.left(path.lastIndexOf('/') + 1);

	return (path);
}

void MainWindow::UpdateStartButton(void)
{
	if (heimdallRunning)
	{
		startFlashButton->setEnabled(false);
		return;
	}

	if (repartitionCheckBox->isChecked())
	{
		if (!IsArchive(pitLineEdit->text()) && factoryfsCheckBox->isChecked() && !IsArchive(factoryfsLineEdit->text()) && kernelCheckBox->isChecked()
			&& !IsArchive(kernelLineEdit->text()) && paramCheckBox->isChecked() && !IsArchive(paramLineEdit->text())
			&& primaryBootCheckBox->isChecked() && !IsArchive(primaryBootLineEdit->text()) && secondaryBootCheckBox->isChecked()
			&& !IsArchive(secondaryBootLineEdit->text()) && modemCheckBox->isChecked() && !IsArchive(modemLineEdit->text()))
		{
			startFlashButton->setEnabled(true);
		}
		else
		{
			startFlashButton->setEnabled(false);
		}
	}
	else
	{
		bool atLeastOneFile = false;

		if (factoryfsCheckBox->isChecked())
		{
			atLeastOneFile = true;

			if (IsArchive(factoryfsLineEdit->text()))
			{
				startFlashButton->setEnabled(false);
				return;
			}
		}

		if (kernelCheckBox->isChecked())
		{
			atLeastOneFile = true;

			if (IsArchive(kernelLineEdit->text()))
			{
				startFlashButton->setEnabled(false);
				return;
			}
		}

		if (paramCheckBox->isChecked())
		{
			atLeastOneFile = true;

			if (IsArchive(paramLineEdit->text()))
			{
				startFlashButton->setEnabled(false);
				return;
			}
		}

		if (primaryBootCheckBox->isChecked())
		{
			atLeastOneFile = true;

			if (IsArchive(primaryBootLineEdit->text()))
			{
				startFlashButton->setEnabled(false);
				return;
			}
		}

		if (secondaryBootCheckBox->isChecked())
		{
			atLeastOneFile = true;

			if (IsArchive(secondaryBootLineEdit->text()))
			{
				startFlashButton->setEnabled(false);
				return;
			}
		}

		if (cacheCheckBox->isChecked())
		{
			atLeastOneFile = true;

			if (IsArchive(cacheLineEdit->text()))
			{
				startFlashButton->setEnabled(false);
				return;
			}
		}

		if (databaseCheckBox->isChecked())
		{
			atLeastOneFile = true;

			if (IsArchive(databaseLineEdit->text()))
			{
				startFlashButton->setEnabled(false);
				return;
			}
		}

		if (modemCheckBox->isChecked())
		{
			atLeastOneFile = true;

			if (IsArchive(modemLineEdit->text()))
			{
				startFlashButton->setEnabled(false);
				return;
			}
		}

		if (recoveryCheckBox->isChecked())
		{
			atLeastOneFile = true;

			if (IsArchive(recoveryLineEdit->text()))
			{
				startFlashButton->setEnabled(false);
				return;
			}
		}

		startFlashButton->setEnabled(atLeastOneFile);
	}
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUi(this);

	heimdallRunning = false;

	lastDirectory = QDir::toNativeSeparators(QApplication::applicationDirPath());

	QObject::connect(actionDonate, SIGNAL(triggered()), this, SLOT(OpenDonationWebpage()));
	QObject::connect(actionAboutHeimdall, SIGNAL(triggered()), this, SLOT(ShowAbout()));

	QObject::connect(pitBrowseButton, SIGNAL(clicked()), this, SLOT(SelectPit()));
	QObject::connect(factoryfsBrowseButton, SIGNAL(clicked()), this, SLOT(SelectFactoryfs()));
	QObject::connect(kernelBrowseButton, SIGNAL(clicked()), this, SLOT(SelectKernel()));
	QObject::connect(paramBrowseButton, SIGNAL(clicked()), this, SLOT(SelectParam()));
	QObject::connect(primaryBootBrowseButton, SIGNAL(clicked()), this, SLOT(SelectPrimaryBootloader()));
	QObject::connect(secondaryBootBrowseButton, SIGNAL(clicked()), this, SLOT(SelectSecondaryBootloader()));
	QObject::connect(cacheBrowseButton, SIGNAL(clicked()), this, SLOT(SelectCache()));
	QObject::connect(databaseBrowseButton, SIGNAL(clicked()), this, SLOT(SelectDatabase()));
	QObject::connect(modemBrowseButton, SIGNAL(clicked()), this, SLOT(SelectModem()));
	QObject::connect(recoveryBrowseButton, SIGNAL(clicked()), this, SLOT(SelectRecovery()));

	QObject::connect(repartitionCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetRepartionEnabled(int)));
	QObject::connect(factoryfsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetFactoryfsEnabled(int)));
	QObject::connect(kernelCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetKernelEnabled(int)));
	QObject::connect(paramCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetParamEnabled(int)));
	QObject::connect(primaryBootCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetPrimaryBootloaderEnabled(int)));
	QObject::connect(secondaryBootCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetSecondaryBootloaderEnabled(int)));
	QObject::connect(cacheCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetCacheEnabled(int)));
	QObject::connect(databaseCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetDatabaseEnabled(int)));
	QObject::connect(modemCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetModemEnabled(int)));
	QObject::connect(recoveryCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetRecoveryEnabled(int)));
	
	QObject::connect(startFlashButton, SIGNAL(clicked()), this, SLOT(StartFlash()));

	QObject::connect(&process, SIGNAL(readyRead()), this, SLOT(HandleHeimdallStdout()));
	QObject::connect(&process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(HandleHeimdallReturned(int, QProcess::ExitStatus)));
	QObject::connect(&process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(HandleHeimdallError(QProcess::ProcessError)));
}

MainWindow::~MainWindow()
{
}

void MainWindow::OpenDonationWebpage(void)
{
	QDesktopServices::openUrl(QUrl("http://www.glassechidna.com.au/donate/", QUrl::StrictMode));
}

void MainWindow::ShowAbout(void)
{
	aboutForm.show();
}

void MainWindow::SelectPit(void)
{
	QString path = PromptFileSelection();
	pitLineEdit->setText(path);

	SetRepartionEnabled(path != "");
}

void MainWindow::SelectFactoryfs(void)
{
	QString path = PromptFileSelection();
	factoryfsLineEdit->setText(path);

	SetFactoryfsEnabled(path != "");
}

void MainWindow::SelectKernel(void)
{
	QString path = PromptFileSelection();
	kernelLineEdit->setText(path);

	SetKernelEnabled(path != "");
}

void MainWindow::SelectParam(void)
{
	QString path = PromptFileSelection();
	paramLineEdit->setText(path);

	SetParamEnabled(path != "");
}

void MainWindow::SelectPrimaryBootloader(void)
{
	QString path = PromptFileSelection();
	primaryBootLineEdit->setText(path);

	SetPrimaryBootloaderEnabled(path != "");
}

void MainWindow::SelectSecondaryBootloader(void)
{
	QString path = PromptFileSelection();
	secondaryBootLineEdit->setText(path);

	SetSecondaryBootloaderEnabled(path != "");
}

void MainWindow::SelectCache(void)
{
	QString path = PromptFileSelection();
	cacheLineEdit->setText(path);

	SetCacheEnabled(path != "");
}

void MainWindow::SelectDatabase(void)
{
	QString path = PromptFileSelection();
	databaseLineEdit->setText(path);

	SetDatabaseEnabled(path != "");
}

void MainWindow::SelectModem(void)
{
	QString path = PromptFileSelection();
	modemLineEdit->setText(path);

	SetModemEnabled(path != "");
}

void MainWindow::SelectRecovery(void)
{
	QString path = PromptFileSelection();
	recoveryLineEdit->setText(path);

	SetRecoveryEnabled(path != "");
}

void MainWindow::SetRepartionEnabled(int enabled)
{
	if (repartitionCheckBox->isChecked() != (enabled != 0))
		repartitionCheckBox->setChecked(enabled);

	if (enabled)
	{
		repartitionCheckBox->setEnabled(true);
		pitLineEdit->setEnabled(true);
		repartitionCheckBox->setChecked(true);
	}
	else
	{
		repartitionCheckBox->setEnabled(pitLineEdit->text() != "");
		pitLineEdit->setEnabled(false);
	}
	
	UpdateStartButton();
}

void MainWindow::SetFactoryfsEnabled(int enabled)
{
	if (factoryfsCheckBox->isChecked() != (enabled != 0))
		factoryfsCheckBox->setChecked(enabled);

	if (enabled)
	{
		factoryfsCheckBox->setEnabled(true);
		factoryfsLineEdit->setEnabled(true);
		factoryfsCheckBox->setChecked(true);
	}
	else
	{
		factoryfsCheckBox->setEnabled(factoryfsLineEdit->text() != "");
		factoryfsLineEdit->setEnabled(false);
	}
	
	UpdateStartButton();
}

void MainWindow::SetKernelEnabled(int enabled)
{
	if (kernelCheckBox->isChecked() != (enabled != 0))
		kernelCheckBox->setChecked(enabled);

	if (enabled)
	{
		kernelCheckBox->setEnabled(true);
		kernelLineEdit->setEnabled(true);
		kernelCheckBox->setChecked(true);
	}
	else
	{
		kernelCheckBox->setEnabled(kernelLineEdit->text() != "");
		kernelLineEdit->setEnabled(false);
	}
	
	UpdateStartButton();
}

void MainWindow::SetParamEnabled(int enabled)
{
	if (paramCheckBox->isChecked() != (enabled != 0))
		paramCheckBox->setChecked(enabled);

	if (enabled)
	{
		paramCheckBox->setEnabled(true);
		paramLineEdit->setEnabled(true);
		paramCheckBox->setChecked(true);
	}
	else
	{
		paramCheckBox->setEnabled(paramLineEdit->text() != "");
		paramLineEdit->setEnabled(false);
	}
	
	UpdateStartButton();
}

void MainWindow::SetPrimaryBootloaderEnabled(int enabled)
{
	if (primaryBootCheckBox->isChecked() != (enabled != 0))
		primaryBootCheckBox->setChecked(enabled);

	if (enabled)
	{
		primaryBootCheckBox->setEnabled(true);
		primaryBootLineEdit->setEnabled(true);
		primaryBootCheckBox->setChecked(true);
	}
	else
	{
		primaryBootCheckBox->setEnabled(primaryBootLineEdit->text() != "");
		primaryBootLineEdit->setEnabled(false);
	}
	
	UpdateStartButton();
}

void MainWindow::SetSecondaryBootloaderEnabled(int enabled)
{
	if (secondaryBootCheckBox->isChecked() != (enabled != 0))
		secondaryBootCheckBox->setChecked(enabled);

	if (enabled)
	{
		secondaryBootCheckBox->setEnabled(true);
		secondaryBootLineEdit->setEnabled(true);
		secondaryBootCheckBox->setChecked(true);
	}
	else
	{
		secondaryBootCheckBox->setEnabled(secondaryBootLineEdit->text() != "");
		secondaryBootLineEdit->setEnabled(false);
	}
	
	UpdateStartButton();
}

void MainWindow::SetCacheEnabled(int enabled)
{
	if (cacheCheckBox->isChecked() != (enabled != 0))
		cacheCheckBox->setChecked(enabled);

	if (enabled)
	{
		cacheCheckBox->setEnabled(true);
		cacheLineEdit->setEnabled(true);
		cacheCheckBox->setChecked(true);
	}
	else
	{
		cacheCheckBox->setEnabled(cacheLineEdit->text() != "");
		cacheLineEdit->setEnabled(false);
	}
	
	UpdateStartButton();
}

void MainWindow::SetDatabaseEnabled(int enabled)
{
	if (databaseCheckBox->isChecked() != (enabled != 0))
		databaseCheckBox->setChecked(enabled);

	if (enabled)
	{
		databaseCheckBox->setEnabled(true);
		databaseLineEdit->setEnabled(true);
		databaseCheckBox->setChecked(true);
	}
	else
	{
		databaseCheckBox->setEnabled(databaseLineEdit->text() != "");
		databaseLineEdit->setEnabled(false);
	}
	
	UpdateStartButton();
}

void MainWindow::SetModemEnabled(int enabled)
{
	if (modemCheckBox->isChecked() != (enabled != 0))
		modemCheckBox->setChecked(enabled);

	if (enabled)
	{
		modemCheckBox->setEnabled(true);
		modemLineEdit->setEnabled(true);
		modemCheckBox->setChecked(true);
	}
	else
	{
		modemCheckBox->setEnabled(databaseLineEdit->text() != "");
		modemLineEdit->setEnabled(false);
	}
	
	UpdateStartButton();
}

void MainWindow::SetRecoveryEnabled(int enabled)
{
	if (recoveryCheckBox->isChecked() != (enabled != 0))
		recoveryCheckBox->setChecked(enabled);

	if (enabled)
	{
		recoveryCheckBox->setEnabled(true);
		recoveryLineEdit->setEnabled(true);
		recoveryCheckBox->setChecked(true);
	}
	else
	{
		recoveryCheckBox->setEnabled(databaseLineEdit->text() != "");
		recoveryLineEdit->setEnabled(false);
	}
	
	UpdateStartButton();
}

void MainWindow::StartFlash(void)
{
	heimdallRunning = true;
	heimdallFailed = false;
	
	QStringList arguments;
	arguments.append("flash");

	if (repartitionCheckBox->isChecked())
	{
		arguments.append("--pit");
		arguments.append(pitLineEdit->text());
	}

	if (factoryfsCheckBox->isChecked())
	{
		arguments.append("--factoryfs");
		arguments.append(factoryfsLineEdit->text());
	}

	if (kernelCheckBox->isChecked())
	{
		arguments.append("--kernel");
		arguments.append(kernelLineEdit->text());
	}

	if (paramCheckBox->isChecked())
	{
		arguments.append("--param");
		arguments.append(paramLineEdit->text());
	}

	if (primaryBootCheckBox->isChecked())
	{
		arguments.append("--primary-boot");
		arguments.append(primaryBootLineEdit->text());
	}

	if (secondaryBootCheckBox->isChecked())
	{
		arguments.append("--secondary-boot");
		arguments.append(secondaryBootLineEdit->text());
	}

	if (cacheCheckBox->isChecked())
	{
		arguments.append("--cache");
		arguments.append(cacheLineEdit->text());
	}

	if (databaseCheckBox->isChecked())
	{
		arguments.append("--dbdata");
		arguments.append(databaseLineEdit->text());
	}

	if (modemCheckBox->isChecked())
	{
		arguments.append("--modem");
		arguments.append(modemLineEdit->text());
	}

	if (recoveryCheckBox->isChecked())
	{
		arguments.append("--recovery");
		arguments.append(recoveryLineEdit->text());
	}

	if (repartitionCheckBox->isChecked())
	{
		arguments.append("--repartition");
	}

	flashProgressBar->setEnabled(true);
	UpdateStartButton();
	
	int pathIndex = -1;
	process.setReadChannel(QProcess::StandardOutput);
	
	process.start("heimdall", arguments);
	process.waitForStarted(1000);
	
	// OS X was playing up and not finding heimdall, so we're manually checking the PATH.
	if (heimdallFailed)
	{
		QStringList environment = QProcess::systemEnvironment();
		
		QStringList paths;
		// Ensure /usr/local/bin is in PATH
		for (int i = 0; i < environment.length(); i++)
		{
			if (environment[i].left(5) == "PATH=")
			{
				paths = environment[i].mid(5).split(':');
				paths.prepend("/usr/local/bin");
				break;
			}
		}
		
		while (heimdallFailed && ++pathIndex < paths.length())
		{
			QString heimdallPath = paths[pathIndex];
			
			if (heimdallPath.length() > 0)
			{
				heimdallFailed = false;
				
				if (heimdallPath[heimdallPath.length() - 1] != QDir::separator())
					heimdallPath += QDir::separator();
				
				heimdallPath += "heimdall";
				
				process.start(heimdallPath, arguments);
				process.waitForStarted(1000);
			}
		}
		
		if (heimdallFailed)
		{
			flashLabel->setText("Failed to start Heimdall!");
			
			heimdallRunning = false;
			flashProgressBar->setEnabled(false);
			UpdateStartButton();
		}
	}
}

void MainWindow::HandleHeimdallStdout(void)
{
	QString output = process.read(1024);

	// We often receive multiple lots of output from Heimdall at one time. So we use regular expressions
	// to ensure we don't miss out on any important information.
	QRegExp uploadingExp("Uploading [^\n]+\n");
	if (output.lastIndexOf(uploadingExp) > -1)
		flashLabel->setText(uploadingExp.cap().left(uploadingExp.cap().length() - 1));

	QRegExp percentExp("[\b\n][0-9]+%");
	if (output.lastIndexOf(percentExp) > -1)
	{
		QString percentString = percentExp.cap();
		flashProgressBar->setValue(percentString.mid(1, percentString.length() - 2).toInt());
	}

	/*// Handle other information

	int endOfLastLine = output.length() - 1;
	for (; endOfLastLine > -1; endOfLastLine--)
	{
		if (output[endOfLastLine] != '\n')
			break;
	}

	if (endOfLastLine < 0)
		return;	// Output was blank or just a bunch of new line characters.

	int startOfLastLine = endOfLastLine - 1;
	for (; startOfLastLine > -1; startOfLastLine--)
	{
		if (output[startOfLastLine] == '\n')
			break;
	}

	startOfLastLine++;

	// Just look at the last line of the output
	output = output.mid(startOfLastLine, endOfLastLine - startOfLastLine + 1);	// Work with the last line only

	percentExp.setPattern("[0-9]+%");
	
	// If the last line wasn't a uploading message or a percentage transferred then display it.
	if (output.lastIndexOf(uploadingExp) < 0 && output.lastIndexOf(percentExp) < 0)
		flashLabel->setText(output);*/
}

void MainWindow::HandleHeimdallReturned(int exitCode, QProcess::ExitStatus exitStatus)
{
	// This is a work-around for strange issues as a result of a exitCode being cast to
	// a unsigned char.
	char byteExitCode = exitCode;
	
	heimdallRunning = false;
	flashProgressBar->setEnabled(false);
	UpdateStartButton();

	if (exitStatus == QProcess::NormalExit && byteExitCode >= 0)
	{
		flashLabel->setText("Flash completed sucessfully!");
	}
	else
	{
		QString error = process.readAllStandardError();

		int firstNewLineChar = error.indexOf('\n');

		if (firstNewLineChar == 0)
			error = error.mid(1);
		else
			error = error.left(firstNewLineChar);

		flashLabel->setText(error);
	}
}

void MainWindow::HandleHeimdallError(QProcess::ProcessError error)
{	
	if (error == QProcess::FailedToStart || error == QProcess::Timedout)
	{
		heimdallFailed = true;
	}
	else if (error == QProcess::Crashed)
	{
		flashLabel->setText("Heimdall crashed!");
		
		heimdallRunning = false;
		flashProgressBar->setEnabled(false);
		UpdateStartButton();
	}
	else
	{
		flashLabel->setText("Heimdall reported an unknown error!");
		
		heimdallRunning = false;
		flashProgressBar->setEnabled(false);
		UpdateStartButton();
	}
}
