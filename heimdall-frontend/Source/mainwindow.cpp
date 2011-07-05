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

// Qt
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QProcess>
#include <QRegExp>
#include <QUrl>

// Heimdall Frontend
#include "mainwindow.h"
#include "Packaging.h"

using namespace HeimdallFrontend;

void MainWindow::UpdateUnusedPartitionIds(void)
{
	unusedPartitionIds.clear();

	// Initially populate unusedPartitionIds with all possible partition IDs. 
	for (unsigned int i = 0; i < currentPitData.GetEntryCount(); i++)
	{
		const PitEntry *pitEntry = currentPitData.GetEntry(i);

		if (!pitEntry->GetUnused() && strcmp(pitEntry->GetPartitionName(), "PIT") != 0)
			unusedPartitionIds.append(pitEntry->GetPartitionIdentifier());
	}

	// Remove any used partition IDs from unusedPartitionIds
	QList<FileInfo>& fileList = workingPackageData.GetFirmwareInfo().GetFileInfos();

	for (int i = 0; i < fileList.length(); i++)
		unusedPartitionIds.removeOne(fileList[i].GetPartitionId());
}

bool MainWindow::ReadPit(QFile *file)
{
	if(!file->open(QIODevice::ReadOnly))
		return (false);

	unsigned char *buffer = new unsigned char[file->size()];

	file->read(reinterpret_cast<char *>(buffer), file->size());
	file->close();

	bool success = currentPitData.Unpack(buffer);
	delete buffer;

	if (!success)
		currentPitData.Clear();

	return (success);
}

void MainWindow::UpdatePackageUserInterface(void)
{
	supportedDevicesListWidget->clear();
	includedFilesListWidget->clear();

	if (loadedPackageData.IsCleared())
	{
		// Package Interface
		firmwareNameLineEdit->clear();
		versionLineEdit->clear();

		developerNamesLineEdit->clear();

		platformLineEdit->clear();

		developerHomepageButton->setEnabled(false);
		developerDonateButton->setEnabled(false);
		
		repartitionRadioButton->setChecked(false);

		loadFirmwareButton->setEnabled(false);
	}
	else
	{
		firmwareNameLineEdit->setText(loadedPackageData.GetFirmwareInfo().GetName());
		versionLineEdit->setText(loadedPackageData.GetFirmwareInfo().GetVersion());

		QString developerNames;

		if (!loadedPackageData.GetFirmwareInfo().GetDevelopers().isEmpty())
		{
			developerNames = loadedPackageData.GetFirmwareInfo().GetDevelopers()[0];
			for (int i = 1; i < loadedPackageData.GetFirmwareInfo().GetDevelopers().length(); i++)
				developerNames += ", " + loadedPackageData.GetFirmwareInfo().GetDevelopers()[i];
		}

		developerNamesLineEdit->setText(developerNames);

		platformLineEdit->setText(loadedPackageData.GetFirmwareInfo().GetPlatformInfo().GetName() + " ("
			+ loadedPackageData.GetFirmwareInfo().GetPlatformInfo().GetVersion() + ")");

		if (!loadedPackageData.GetFirmwareInfo().GetUrl().isEmpty())
			developerHomepageButton->setEnabled(true);
		else
			developerHomepageButton->setEnabled(false);

		if (!loadedPackageData.GetFirmwareInfo().GetDonateUrl().isEmpty())
			developerDonateButton->setEnabled(true);
		else
			developerDonateButton->setEnabled(false);

		for (int i = 0; i < loadedPackageData.GetFirmwareInfo().GetDeviceInfos().length(); i++)
		{
			const DeviceInfo& deviceInfo = loadedPackageData.GetFirmwareInfo().GetDeviceInfos()[i];
			supportedDevicesListWidget->addItem(deviceInfo.GetManufacturer() + " " + deviceInfo.GetName() + " (" + deviceInfo.GetProduct() + ")");
		}

		for (int i = 0; i < loadedPackageData.GetFirmwareInfo().GetFileInfos().length(); i++)
		{
			const FileInfo& fileInfo = loadedPackageData.GetFirmwareInfo().GetFileInfos()[i];
			includedFilesListWidget->addItem(fileInfo.GetFilename());
		}

		repartitionRadioButton->setChecked(loadedPackageData.GetFirmwareInfo().GetRepartition());

		loadFirmwareButton->setEnabled(true);
	}
}

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

QString MainWindow::PromptFileCreation(void)
{
	QString path = QFileDialog::getSaveFileName(this, "Save File", lastDirectory);

	if (path != "")
		lastDirectory = path.left(path.lastIndexOf('/') + 1);

	return (path);
}

void MainWindow::UpdatePartitionNamesInterface(void)
{
	populatingPartitionNames = true;

	partitionNameComboBox->clear();

	int partitionsListWidgetRow = partitionsListWidget->currentRow();

	if (partitionsListWidgetRow >= 0)
	{
		const FileInfo& partitionInfo = workingPackageData.GetFirmwareInfo().GetFileInfos()[partitionsListWidget->currentRow()];

		for (int i = 0; i < unusedPartitionIds.length(); i++)
			partitionNameComboBox->addItem(currentPitData.FindEntry(unusedPartitionIds[i])->GetPartitionName());

		partitionNameComboBox->addItem(currentPitData.FindEntry(partitionInfo.GetPartitionId())->GetPartitionName());
		partitionNameComboBox->setCurrentIndex(unusedPartitionIds.length());

		partitionNameComboBox->setEnabled(true);
	}
	else
	{
		partitionNameComboBox->setEnabled(false);
	}

	populatingPartitionNames = false;
}

void MainWindow::UpdateStartButton(void)
{
	if (heimdallRunning)
	{
		startFlashButton->setEnabled(false);
		return;
	}

	bool allPartitionsValid = true;

	QList<FileInfo>& fileList = workingPackageData.GetFirmwareInfo().GetFileInfos();

	for (int i = 0; i < fileList.length(); i++)
	{
		if (fileList[i].GetFilename().isEmpty())
		{
			allPartitionsValid = false;
			break;
		}
	}

	bool validSettings = allPartitionsValid && fileList.length() > 0;

	startFlashButton->setEnabled(validSettings);
	functionTabWidget->setTabEnabled(functionTabWidget->indexOf(createPackageTab), validSettings);
}

void MainWindow::UpdateBuildPackageButton(void)
{
	const FirmwareInfo& firmwareInfo = workingPackageData.GetFirmwareInfo();

	if (firmwareInfo.GetName().isEmpty() || firmwareInfo.GetVersion().isEmpty() || firmwareInfo.GetPlatformInfo().GetName().isEmpty()
		|| firmwareInfo.GetPlatformInfo().GetVersion().isEmpty() || firmwareInfo.GetDevelopers().isEmpty() || firmwareInfo.GetDeviceInfos().isEmpty())
	{
		buildPackageButton->setEnabled(false);
	}
	else
	{
		buildPackageButton->setEnabled(true);
	}
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUi(this);

	heimdallRunning = false;

	lastDirectory = QDir::toNativeSeparators(QApplication::applicationDirPath());

	populatingPartitionNames = false;

	functionTabWidget->setTabEnabled(functionTabWidget->indexOf(createPackageTab), false);

	QObject::connect(actionDonate, SIGNAL(triggered()), this, SLOT(OpenDonationWebpage()));
	QObject::connect(actionAboutHeimdall, SIGNAL(triggered()), this, SLOT(ShowAbout()));

	QObject::connect(browseFirmwarePackageButton, SIGNAL(clicked()), this, SLOT(SelectFirmwarePackage()));
	QObject::connect(developerHomepageButton, SIGNAL(clicked()), this, SLOT(OpenDeveloperHomepage()));
	QObject::connect(developerDonateButton, SIGNAL(clicked()), this, SLOT(OpenDeveloperDonationWebpage()));
	QObject::connect(loadFirmwareButton, SIGNAL(clicked()), this, SLOT(LoadFirmwarePackage()));

	QObject::connect(partitionsListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(SelectPartition(int)));
	QObject::connect(addPartitionButton, SIGNAL(clicked()), this, SLOT(AddPartition()));
	QObject::connect(removePartitionButton, SIGNAL(clicked()), this, SLOT(RemovePartition()));

	QObject::connect(partitionNameComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SelectPartitionName(int)));
	QObject::connect(partitionFileBrowseButton, SIGNAL(clicked()), this, SLOT(SelectPartitionFile()));

	QObject::connect(pitBrowseButton, SIGNAL(clicked()), this, SLOT(SelectPit()));
	QObject::connect(repartitionCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetRepartition(int)));
	
	QObject::connect(startFlashButton, SIGNAL(clicked()), this, SLOT(StartFlash()));

	QObject::connect(createFirmwareNameLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(FirmwareNameChanged(const QString&)));
	QObject::connect(createFirmwareVersionLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(FirmwareVersionChanged(const QString&)));
	QObject::connect(createPlatformNameLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(PlatformNameChanged(const QString&)));
	QObject::connect(createPlatformVersionLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(PlatformVersionChanged(const QString&)));

	QObject::connect(createHomepageLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(HomepageUrlChanged(const QString&)));
	QObject::connect(createDonateLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(DonateUrlChanged(const QString&)));

	QObject::connect(createDevelopersListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(SelectDeveloper(int)));
	QObject::connect(createDeveloperNameLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(DeveloperNameChanged(const QString&)));
	QObject::connect(addDeveloperButton, SIGNAL(clicked()), this, SLOT(AddDeveloper()));
	QObject::connect(removeDeveloperButton, SIGNAL(clicked()), this, SLOT(RemoveDeveloper()));

	QObject::connect(createDevicesListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(SelectDevice(int)));
	QObject::connect(deviceManufacturerLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(DeviceInfoChanged(const QString&)));
	QObject::connect(deviceNameLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(DeviceInfoChanged(const QString&)));
	QObject::connect(deviceProductCodeLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(DeviceInfoChanged(const QString&)));
	QObject::connect(addDeviceButton, SIGNAL(clicked()), this, SLOT(AddDevice()));
	QObject::connect(removeDeviceButton, SIGNAL(clicked()), this, SLOT(RemoveDevice()));
			
	QObject::connect(buildPackageButton, SIGNAL(clicked()), this, SLOT(BuildPackage()));

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

void MainWindow::SelectFirmwarePackage(void)
{
	loadedPackageData.Clear();
	UpdatePackageUserInterface();

	QString path = PromptFileSelection();
	firmwarePackageLineEdit->setText(path);

	if (firmwarePackageLineEdit->text() != "")
	{
		if (Packaging::ExtractPackage(firmwarePackageLineEdit->text(), &loadedPackageData))
		{
			UpdatePackageUserInterface();
		}
		else
		{
			// TODO: Error?
			loadedPackageData.Clear();
		}
	}
}

void MainWindow::OpenDeveloperHomepage(void)
{
	QDesktopServices::openUrl(QUrl(loadedPackageData.GetFirmwareInfo().GetUrl(), QUrl::TolerantMode));
}

void MainWindow::OpenDeveloperDonationWebpage(void)
{
	QDesktopServices::openUrl(QUrl(loadedPackageData.GetFirmwareInfo().GetDonateUrl(), QUrl::TolerantMode));
}

void MainWindow::LoadFirmwarePackage(void)
{
	workingPackageData.Clear();
	currentPitData.Clear();
	
	// Make flashSettings responsible for the temporary files
	workingPackageData.GetFiles().append(loadedPackageData.GetFiles());
	loadedPackageData.RemoveAllFiles();

	const QList<FileInfo> packageFileInfos = loadedPackageData.GetFirmwareInfo().GetFileInfos();

	for (int i = 0; i < packageFileInfos.length(); i++)
	{
		for (int j = 0; j < workingPackageData.GetFiles().length(); j++)
		{
			if (workingPackageData.GetFiles()[j]->fileTemplate() == ("XXXXXX-" + packageFileInfos[i].GetFilename()))
			{
				FileInfo partitionInfo(packageFileInfos[i].GetPartitionId(), QDir::current().absoluteFilePath(workingPackageData.GetFiles()[j]->fileName()));
				workingPackageData.GetFirmwareInfo().GetFileInfos().append(partitionInfo);

				break;
			}
		}
	}

	// Find the PIT file and read it
	for (int i = 0; i < workingPackageData.GetFiles().length(); i++)
	{
		QTemporaryFile *file = workingPackageData.GetFiles()[i];

		if (file->fileTemplate() == ("XXXXXX-" + loadedPackageData.GetFirmwareInfo().GetPitFilename()))
		{
			workingPackageData.GetFirmwareInfo().SetPitFilename(QDir::current().absoluteFilePath(file->fileName()));

			if (!ReadPit(file))
			{
				// TODO: Error
				loadedPackageData.Clear();
				UpdatePackageUserInterface();

				workingPackageData.Clear();
				UpdateUnusedPartitionIds();
				return;
			}

			break;
		}
	}

	UpdateUnusedPartitionIds();
	workingPackageData.GetFirmwareInfo().SetRepartition(loadedPackageData.GetFirmwareInfo().GetRepartition());

	loadedPackageData.Clear();
	UpdatePackageUserInterface();
	firmwarePackageLineEdit->clear();

	partitionsListWidget->clear();

	// Populate partitionsListWidget with partition names (from the PIT file)
	for (int i = 0; i < workingPackageData.GetFirmwareInfo().GetFileInfos().length(); i++)
	{
		const FileInfo& partitionInfo = workingPackageData.GetFirmwareInfo().GetFileInfos()[i];

		const PitEntry *pitEntry = currentPitData.FindEntry(partitionInfo.GetPartitionId());

		if (pitEntry)
		{
			partitionsListWidget->addItem(pitEntry->GetPartitionName());
		}
		else
		{
			// TODO: "Firmware package includes invalid partition IDs."
			loadedPackageData.GetFirmwareInfo().Clear();
			currentPitData.Clear();
			UpdateUnusedPartitionIds();

			partitionsListWidget->clear();
			return;
		}
	}

	partitionNameComboBox->clear();
	partitionIdLineEdit->clear();
	partitionFileLineEdit->clear();
	partitionFileBrowseButton->setEnabled(false);

	repartitionCheckBox->setEnabled(true);
	repartitionCheckBox->setChecked(workingPackageData.GetFirmwareInfo().GetRepartition());
	partitionsListWidget->setEnabled(true);
	addPartitionButton->setEnabled(true);
	removePartitionButton->setEnabled(true && partitionsListWidget->currentRow() >= 0);

	pitLineEdit->setText(workingPackageData.GetFirmwareInfo().GetPitFilename());

	functionTabWidget->setCurrentWidget(flashTab);

	UpdateStartButton();
}

void MainWindow::SelectPartitionName(int index)
{
	if (!populatingPartitionNames && index != -1 && index != unusedPartitionIds.length())
	{
		unsigned int newPartitionIndex = unusedPartitionIds[index];
		unusedPartitionIds.removeAt(index);

		FileInfo& partitionInfo = workingPackageData.GetFirmwareInfo().GetFileInfos()[partitionsListWidget->currentRow()];
		unusedPartitionIds.append(partitionInfo.GetPartitionId());
		partitionInfo.SetPartitionId(newPartitionIndex);

		partitionNameComboBox->clear();

		// Update interface
		UpdatePartitionNamesInterface();
		partitionIdLineEdit->setText(QString::number(newPartitionIndex));
		partitionsListWidget->currentItem()->setText(currentPitData.FindEntry(newPartitionIndex)->GetPartitionName());
	}
}

void MainWindow::SelectPartitionFile(void)
{
	QString path = PromptFileSelection();

	if (path != "")
	{
		workingPackageData.GetFirmwareInfo().GetFileInfos()[partitionsListWidget->currentRow()].SetFilename(path);
		partitionFileLineEdit->setText(path);

		pitBrowseButton->setEnabled(true);
		partitionsListWidget->setEnabled(true);
		UpdateStartButton();

		if (unusedPartitionIds.length() > 0)
			addPartitionButton->setEnabled(true);
	}
}

void MainWindow::SelectPartition(int row)
{
	if (row >= 0)
	{
		const FileInfo& partitionInfo = workingPackageData.GetFirmwareInfo().GetFileInfos()[row];

		UpdatePartitionNamesInterface();

		partitionIdLineEdit->setText(QString::number(partitionInfo.GetPartitionId()));
		partitionFileLineEdit->setText(partitionInfo.GetFilename());
		partitionFileBrowseButton->setEnabled(true);

		removePartitionButton->setEnabled(true);
	}
	else
	{
		UpdatePartitionNamesInterface();

		partitionIdLineEdit->clear();
		partitionFileLineEdit->clear();
		partitionFileBrowseButton->setEnabled(false);

		removePartitionButton->setEnabled(false);
	}
}

void MainWindow::AddPartition(void)
{
	FileInfo partitionInfo(unusedPartitionIds.first(), "");
	workingPackageData.GetFirmwareInfo().GetFileInfos().append(partitionInfo);
	UpdateUnusedPartitionIds();

	pitBrowseButton->setEnabled(false);
	addPartitionButton->setEnabled(false);

	partitionsListWidget->addItem(currentPitData.FindEntry(partitionInfo.GetPartitionId())->GetPartitionName());
	partitionsListWidget->setCurrentRow(partitionsListWidget->count() - 1);
	partitionsListWidget->setEnabled(false);
	UpdateStartButton();
}

void MainWindow::RemovePartition(void)
{
	workingPackageData.GetFirmwareInfo().GetFileInfos().removeAt(partitionsListWidget->currentRow());
	UpdateUnusedPartitionIds();

	QListWidgetItem *item = partitionsListWidget->currentItem();
	partitionsListWidget->setCurrentRow(-1);
	delete item;

	pitBrowseButton->setEnabled(true);
	addPartitionButton->setEnabled(true);
	partitionsListWidget->setEnabled(true);
	UpdateStartButton();
}

void MainWindow::SelectPit(void)
{
	QString path = PromptFileSelection();
	bool validPit = path != "";

	// In order to map files in the old PIT to file in the new one, we first must use partition names instead of IDs.
	QList<FileInfo> fileInfos = workingPackageData.GetFirmwareInfo().GetFileInfos();

	int partitionNamesCount = fileInfos.length();
	QString *partitionNames = new QString[fileInfos.length()];
	for (int i = 0; i < fileInfos.length(); i++)
		partitionNames[i] = currentPitData.FindEntry(fileInfos[i].GetPartitionId())->GetPartitionName();

	currentPitData.Clear();

	if (validPit)
	{
		QFile pitFile(path);

		if (ReadPit(&pitFile))
		{
			workingPackageData.GetFirmwareInfo().SetPitFilename(path);

			partitionsListWidget->clear();
			int partitionInfoIndex = 0;

			for (int i = 0; i < partitionNamesCount; i++)
			{
				const PitEntry *pitEntry = currentPitData.FindEntry(partitionNames[i].toAscii().constData());
				
				if (pitEntry)
				{
					fileInfos[partitionInfoIndex++].SetPartitionId(pitEntry->GetPartitionIdentifier());
					partitionsListWidget->addItem(pitEntry->GetPartitionName());
				}
				else
				{
					fileInfos.removeAt(partitionInfoIndex);
				}
			}
		}
		else
		{
			validPit = false;
		}
	}
	
	// If the selected PIT was invalid, attempt to reload the old one.
	if (!validPit)
	{
		// TODO: "The file selected was not a valid PIT file."
		QFile originalPitFile(workingPackageData.GetFirmwareInfo().GetPitFilename());

		if (ReadPit(&originalPitFile))
		{
			validPit = true;
		}
		else
		{
			// TODO: "Failed to reload working PIT data."
			workingPackageData.Clear();
			partitionsListWidget->clear();
		}
	}

	UpdateUnusedPartitionIds();

	delete [] partitionNames;

	pitLineEdit->setText(workingPackageData.GetFirmwareInfo().GetPitFilename());

	repartitionCheckBox->setEnabled(validPit);
	partitionsListWidget->setEnabled(validPit);

	addPartitionButton->setEnabled(validPit);
	removePartitionButton->setEnabled(validPit && partitionsListWidget->currentRow() >= 0);

	UpdateStartButton();
}

void MainWindow::SetRepartition(int enabled)
{
	workingPackageData.GetFirmwareInfo().SetRepartition(enabled);
}

void MainWindow::StartFlash(void)
{
	heimdallRunning = true;
	heimdallFailed = false;
	
	QStringList arguments;
	arguments.append("flash");

	if (repartitionCheckBox->isChecked())
	{
		arguments.append("--repartition");

		arguments.append("--pit");
		arguments.append(pitLineEdit->text());
	}

	// TODO: Loop through partitions and append them.

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
		// Ensure /usr/bin is in PATH
		for (int i = 0; i < environment.length(); i++)
		{
			if (environment[i].left(5) == "PATH=")
			{
				paths = environment[i].mid(5).split(':');
				paths.prepend("/usr/bin");
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

void MainWindow::FirmwareNameChanged(const QString& text)
{
	workingPackageData.GetFirmwareInfo().SetName(text);
	UpdateBuildPackageButton();
}

void MainWindow::FirmwareVersionChanged(const QString& text)
{
	workingPackageData.GetFirmwareInfo().SetVersion(text);
	UpdateBuildPackageButton();
}

void MainWindow::PlatformNameChanged(const QString& text)
{
	workingPackageData.GetFirmwareInfo().GetPlatformInfo().SetName(text);
	UpdateBuildPackageButton();
}

void MainWindow::PlatformVersionChanged(const QString& text)
{
	workingPackageData.GetFirmwareInfo().GetPlatformInfo().SetVersion(text);
	UpdateBuildPackageButton();
}

void MainWindow::HomepageUrlChanged(const QString& text)
{
	workingPackageData.GetFirmwareInfo().SetUrl(text);
}

void MainWindow::DonateUrlChanged(const QString& text)
{
	workingPackageData.GetFirmwareInfo().SetDonateUrl(text);
}

void MainWindow::DeveloperNameChanged(const QString& text)
{
	if (text.isEmpty())
		addDeveloperButton->setEnabled(false);
	else
		addDeveloperButton->setEnabled(true);
}

void MainWindow::SelectDeveloper(int row)
{
	if (row >= 0)
		removeDeveloperButton->setEnabled(true);
	else
		removeDeveloperButton->setEnabled(false);
}

void MainWindow::AddDeveloper(void)
{
	workingPackageData.GetFirmwareInfo().GetDevelopers().append(createDeveloperNameLineEdit->text());

	createDevelopersListWidget->addItem(createDeveloperNameLineEdit->text());
	createDeveloperNameLineEdit->clear();

	UpdateBuildPackageButton();
}

void MainWindow::RemoveDeveloper(void)
{
	workingPackageData.GetFirmwareInfo().GetDevelopers().removeAt(createDevelopersListWidget->currentRow());

	QListWidgetItem *item = createDevelopersListWidget->currentItem();
	createDevelopersListWidget->setCurrentRow(-1);
	delete item;

	removeDeveloperButton->setEnabled(false);

	UpdateBuildPackageButton();
}

void MainWindow::DeviceInfoChanged(const QString& text)
{
	if (deviceManufacturerLineEdit->text().isEmpty() || deviceNameLineEdit->text().isEmpty() || deviceProductCodeLineEdit->text().isEmpty())
		addDeviceButton->setEnabled(false);
	else
		addDeviceButton->setEnabled(true);
}

void MainWindow::SelectDevice(int row)
{
	if (row >= 0)
		removeDeviceButton->setEnabled(true);
	else
		removeDeviceButton->setEnabled(false);
}

void MainWindow::AddDevice(void)
{
	workingPackageData.GetFirmwareInfo().GetDeviceInfos().append(DeviceInfo(deviceManufacturerLineEdit->text(), deviceNameLineEdit->text(),
		deviceProductCodeLineEdit->text()));

	createDevicesListWidget->addItem(deviceManufacturerLineEdit->text() + " " + deviceNameLineEdit->text() + " (" + deviceProductCodeLineEdit->text() + ")");
	deviceManufacturerLineEdit->clear();
	deviceNameLineEdit->clear();
	deviceProductCodeLineEdit->clear();

	UpdateBuildPackageButton();
}

void MainWindow::RemoveDevice(void)
{
	workingPackageData.GetFirmwareInfo().GetDeviceInfos().removeAt(createDevicesListWidget->currentRow());

	QListWidgetItem *item = createDevicesListWidget->currentItem();
	createDevicesListWidget->setCurrentRow(-1);
	delete item;

	removeDeviceButton->setEnabled(false);

	UpdateBuildPackageButton();
}
			
void MainWindow::BuildPackage(void)
{
	QString packagePath = PromptFileCreation();

	if (!packagePath.endsWith(".tar.gz", Qt::CaseInsensitive))
	{
		if (packagePath.endsWith(".tar", Qt::CaseInsensitive))
			packagePath.append(".gz");
		else if (packagePath.endsWith(".gz", Qt::CaseInsensitive))
			packagePath.replace(packagePath.length() - 3, ".tar.gz");
		else if (packagePath.endsWith(".tgz", Qt::CaseInsensitive))
			packagePath.replace(packagePath.length() - 4, ".tar.gz");
		else
			packagePath.append(".tar.gz");
	}

	Packaging::BuildPackage(packagePath, workingPackageData);
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
