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

// Qt
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QProcess>
#include <QRegExp>
#include <QUrl>

// Heimdall Frontend
#include "Alerts.h"
#include "mainwindow.h"
#include "Packaging.h"

#define UNUSED(x) (void)(x)

using namespace HeimdallFrontend;

void MainWindow::StartHeimdall(const QStringList& arguments)
{
	UpdateInterfaceAvailability();

	heimdallProcess.setReadChannel(QProcess::StandardOutput);
	
	heimdallProcess.start("heimdall", arguments);
	heimdallProcess.waitForStarted(3000);
	
	// OS X was playing up and not finding heimdall, so we're manually checking the PATH.
	if (heimdallFailed)
	{
		QStringList environment = QProcess::systemEnvironment();
		
		QStringList paths;

		// Ensure /usr/local/bin and /usr/bin are in PATH.
		for (int i = 0; i < environment.length(); i++)
		{
			if (environment[i].left(5) == "PATH=")
			{
				paths = environment[i].mid(5).split(':');
				
				if (!paths.contains("/usr/local/bin"))
					paths.prepend("/usr/local/bin");
				
				if (!paths.contains("/usr/bin"))
					paths.prepend("/usr/bin");
				
				break;
			}
		}
		
		int pathIndex = -1;

		while (heimdallFailed && ++pathIndex < paths.length())
		{
			QString heimdallPath = paths[pathIndex];
			
			if (heimdallPath.length() > 0)
			{
				utilityOutputPlainTextEdit->clear();
				heimdallFailed = false;
				
				if (heimdallPath[heimdallPath.length() - 1] != QDir::separator())
					heimdallPath += QDir::separator();
				
				heimdallPath += "heimdall";
				
				heimdallProcess.start(heimdallPath, arguments);
				heimdallProcess.waitForStarted(3000);
			}
		}
		
		if (heimdallFailed)
		{
			flashLabel->setText("Failed to start Heimdall!");
			
			heimdallState = HeimdallState::Stopped;
			UpdateInterfaceAvailability();
		}
	}
}

void MainWindow::UpdateUnusedPartitionIds(void)
{
	unusedPartitionIds.clear();

	// Initially populate unusedPartitionIds with all possible partition IDs. 
	for (unsigned int i = 0; i < currentPitData.GetEntryCount(); i++)
	{
		const PitEntry *pitEntry = currentPitData.GetEntry(i);

		if (pitEntry->IsFlashable() && strcmp(pitEntry->GetPartitionName(), "PIT") != 0 && strcmp(pitEntry->GetPartitionName(), "PT") != 0)
			unusedPartitionIds.append(pitEntry->GetIdentifier());
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
	delete[] buffer;

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
		
		repartitionRadioButton->setChecked(false);
		noRebootRadioButton->setChecked(false);
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

		for (int i = 0; i < loadedPackageData.GetFirmwareInfo().GetDeviceInfos().length(); i++)
		{
			const DeviceInfo& deviceInfo = loadedPackageData.GetFirmwareInfo().GetDeviceInfos()[i];
			supportedDevicesListWidget->addItem(deviceInfo.GetManufacturer() + " " + deviceInfo.GetName() + ": " + deviceInfo.GetProduct());
		}

		for (int i = 0; i < loadedPackageData.GetFirmwareInfo().GetFileInfos().length(); i++)
		{
			const FileInfo& fileInfo = loadedPackageData.GetFirmwareInfo().GetFileInfos()[i];
			includedFilesListWidget->addItem(fileInfo.GetFilename());
		}

		repartitionRadioButton->setChecked(loadedPackageData.GetFirmwareInfo().GetRepartition());
		noRebootRadioButton->setChecked(loadedPackageData.GetFirmwareInfo().GetNoReboot());
	}

	UpdateLoadPackageInterfaceAvailability();
}

bool MainWindow::IsArchive(QString path)
{
	// Not a real check but hopefully it gets the message across, don't directly flash archives!
	return (path.endsWith(".tar", Qt::CaseInsensitive) || path.endsWith(".gz", Qt::CaseInsensitive) || path.endsWith(".zip", Qt::CaseInsensitive)
		|| path.endsWith(".bz2", Qt::CaseInsensitive) || path.endsWith(".7z", Qt::CaseInsensitive) || path.endsWith(".rar", Qt::CaseInsensitive));
}

QString MainWindow::PromptFileSelection(const QString& caption, const QString& filter)
{
	QString path = QFileDialog::getOpenFileName(this, caption, lastDirectory, filter);

	if (path != "")
		lastDirectory = path.left(path.lastIndexOf('/') + 1);

	return (path);
}

QString MainWindow::PromptFileCreation(const QString& caption, const QString& filter)
{
	QString path = QFileDialog::getSaveFileName(this, caption, lastDirectory, filter);

	if (path != "")
		lastDirectory = path.left(path.lastIndexOf('/') + 1);

	return (path);
}

void MainWindow::UpdateLoadPackageInterfaceAvailability(void)
{
	if (loadedPackageData.IsCleared())
	{
		developerHomepageButton->setEnabled(false);
		developerDonateButton->setEnabled(false);
		loadFirmwareButton->setEnabled(false);
	}
	else
	{
		developerHomepageButton->setEnabled(!loadedPackageData.GetFirmwareInfo().GetUrl().isEmpty());
		developerDonateButton->setEnabled(!loadedPackageData.GetFirmwareInfo().GetDonateUrl().isEmpty());
		loadFirmwareButton->setEnabled(!!(heimdallState & HeimdallState::Stopped));
	}
}

void MainWindow::UpdateFlashInterfaceAvailability(void)
{
	if (!!(heimdallState & HeimdallState::Stopped))
	{
		partitionNameComboBox->setEnabled(partitionsListWidget->currentRow() >= 0);

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

		bool validFlashSettings = allPartitionsValid && fileList.length() > 0;
		
		flashProgressBar->setEnabled(false);
		optionsGroup->setEnabled(true);
		sessionGroup->setEnabled(true);
		startFlashButton->setEnabled(validFlashSettings);
		noRebootCheckBox->setEnabled(validFlashSettings);
		resumeCheckbox->setEnabled(validFlashSettings);
	}
	else
	{
		partitionNameComboBox->setEnabled(false);

		flashProgressBar->setEnabled(true);
		optionsGroup->setEnabled(false);
		sessionGroup->setEnabled(false);
	}
}

void MainWindow::UpdateCreatePackageInterfaceAvailability(void)
{
	if (!!(heimdallState & HeimdallState::Stopped))
	{
		const FirmwareInfo& firmwareInfo = workingPackageData.GetFirmwareInfo();

		bool fieldsPopulated = !(firmwareInfo.GetName().isEmpty() || firmwareInfo.GetVersion().isEmpty() || firmwareInfo.GetPlatformInfo().GetName().isEmpty()
			|| firmwareInfo.GetPlatformInfo().GetVersion().isEmpty() || firmwareInfo.GetDevelopers().isEmpty() || firmwareInfo.GetDeviceInfos().isEmpty());

		buildPackageButton->setEnabled(fieldsPopulated);
		addDeveloperButton->setEnabled(!addDeveloperButton->text().isEmpty());
		removeDeveloperButton->setEnabled(createDevelopersListWidget->currentRow() >= 0);
	}
	else
	{
		buildPackageButton->setEnabled(false);
	}
}

void MainWindow::UpdateUtilitiesInterfaceAvailability(void)
{
	if (!!(heimdallState & HeimdallState::Stopped))
	{
		detectDeviceButton->setEnabled(true);
		closePcScreenButton->setEnabled(true);
		pitSaveAsButton->setEnabled(true);

		downloadPitButton->setEnabled(!pitDestinationLineEdit->text().isEmpty());
		
		if (printPitDeviceRadioBox->isChecked())
		{
			// Device
			printLocalPitGroup->setEnabled(false);
			printPitButton->setEnabled(true);
		}
		else
		{
			// Local File
			printLocalPitGroup->setEnabled(true);
			printLocalPitLineEdit->setEnabled(true);
			printLocalPitBrowseButton->setEnabled(true);

			printPitButton->setEnabled(!printLocalPitLineEdit->text().isEmpty());
		}
	}
	else
	{
		detectDeviceButton->setEnabled(false);
		closePcScreenButton->setEnabled(false);
		pitSaveAsButton->setEnabled(false);
		downloadPitButton->setEnabled(false);

		printLocalPitGroup->setEnabled(false);
		printPitButton->setEnabled(false);
	}
}

void MainWindow::UpdateInterfaceAvailability(void)
{
	UpdateLoadPackageInterfaceAvailability();
	UpdateFlashInterfaceAvailability();
	UpdateCreatePackageInterfaceAvailability();
	UpdateUtilitiesInterfaceAvailability();

	if (!!(heimdallState & HeimdallState::Stopped))
	{		
		// Enable/disable tabs

		for (int i = 0; i < functionTabWidget->count(); i++)
			functionTabWidget->setTabEnabled(i, true);

		functionTabWidget->setTabEnabled(functionTabWidget->indexOf(createPackageTab), startFlashButton->isEnabled());
	}
	else
	{
		// Disable non-current tabs

		for (int i = 0; i < functionTabWidget->count(); i++)
		{
			if (i == functionTabWidget->currentIndex())
				functionTabWidget->setTabEnabled(i, true);
			else
				functionTabWidget->setTabEnabled(i, false);
		}
	}
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
	}

	populatingPartitionNames = false;

	UpdateFlashInterfaceAvailability();
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUi(this);

	heimdallState = HeimdallState::Stopped;

	lastDirectory = QDir::toNativeSeparators(QApplication::applicationDirPath());

	populatingPartitionNames = false;

	verboseOutput = false;
	resume = false;

	tabIndex = functionTabWidget->currentIndex();
	functionTabWidget->setTabEnabled(functionTabWidget->indexOf(createPackageTab), false);

	QObject::connect(functionTabWidget, SIGNAL(currentChanged(int)), this, SLOT(FunctionTabChanged(int)));
	
	// Menu
	QObject::connect(actionDonate, SIGNAL(triggered()), this, SLOT(OpenDonationWebpage()));
	QObject::connect(actionVerboseOutput, SIGNAL(toggled(bool)), this, SLOT(SetVerboseOutput(bool)));
	QObject::connect(actionResumeConnection, SIGNAL(toggled(bool)), this, SLOT(SetResume(bool)));
	QObject::connect(actionAboutHeimdall, SIGNAL(triggered()), this, SLOT(ShowAbout()));

	// Load Package Tab
	QObject::connect(browseFirmwarePackageButton, SIGNAL(clicked()), this, SLOT(SelectFirmwarePackage()));
	QObject::connect(developerHomepageButton, SIGNAL(clicked()), this, SLOT(OpenDeveloperHomepage()));
	QObject::connect(developerDonateButton, SIGNAL(clicked()), this, SLOT(OpenDeveloperDonationWebpage()));
	QObject::connect(loadFirmwareButton, SIGNAL(clicked()), this, SLOT(LoadFirmwarePackage()));

	QObject::connect(partitionsListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(SelectPartition(int)));
	QObject::connect(addPartitionButton, SIGNAL(clicked()), this, SLOT(AddPartition()));
	QObject::connect(removePartitionButton, SIGNAL(clicked()), this, SLOT(RemovePartition()));

	// Flash Tab
	QObject::connect(partitionNameComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SelectPartitionName(int)));
	QObject::connect(partitionFileBrowseButton, SIGNAL(clicked()), this, SLOT(SelectPartitionFile()));

	QObject::connect(pitBrowseButton, SIGNAL(clicked()), this, SLOT(SelectPit()));

	QObject::connect(repartitionCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetRepartition(int)));

	QObject::connect(noRebootCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetNoReboot(int)));
	QObject::connect(resumeCheckbox, SIGNAL(stateChanged(int)), this, SLOT(SetResume(int)));
	
	QObject::connect(startFlashButton, SIGNAL(clicked()), this, SLOT(StartFlash()));

	// Create Package Tab
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

	// Utilities Tab
	QObject::connect(detectDeviceButton, SIGNAL(clicked()), this, SLOT(DetectDevice()));

	QObject::connect(closePcScreenButton, SIGNAL(clicked()), this, SLOT(ClosePcScreen()));

	QObject::connect(printPitDeviceRadioBox, SIGNAL(toggled(bool)), this, SLOT(DevicePrintPitToggled(bool)));
	QObject::connect(printPitLocalFileRadioBox, SIGNAL(toggled(bool)), this, SLOT(LocalFilePrintPitToggled(bool)));
	QObject::connect(printLocalPitBrowseButton, SIGNAL(clicked()), this, SLOT(SelectPrintPitFile()));
	QObject::connect(printPitButton, SIGNAL(clicked()), this, SLOT(PrintPit()));

	QObject::connect(pitSaveAsButton, SIGNAL(clicked()), this, SLOT(SelectPitDestination()));
	QObject::connect(downloadPitButton, SIGNAL(clicked()), this, SLOT(DownloadPit()));

	// Heimdall Command Line
	QObject::connect(&heimdallProcess, SIGNAL(readyRead()), this, SLOT(HandleHeimdallStdout()));
	QObject::connect(&heimdallProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(HandleHeimdallReturned(int, QProcess::ExitStatus)));
	QObject::connect(&heimdallProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(HandleHeimdallError(QProcess::ProcessError)));
}

MainWindow::~MainWindow()
{
}

void MainWindow::OpenDonationWebpage(void)
{
	QDesktopServices::openUrl(QUrl("http://www.glassechidna.com.au/donate/", QUrl::StrictMode));
}

void MainWindow::SetVerboseOutput(bool enabled)
{
	verboseOutput = enabled;
}

void MainWindow::ShowAbout(void)
{
	aboutForm.show();
}

void MainWindow::FunctionTabChanged(int index)
{
	tabIndex = index;
	deviceDetectedRadioButton->setChecked(false);
}

void MainWindow::SelectFirmwarePackage(void)
{
	loadedPackageData.Clear();
	UpdatePackageUserInterface();

	QString path = PromptFileSelection("Select Package", "Firmware Package (*.gz)");
	firmwarePackageLineEdit->setText(path);

	if (firmwarePackageLineEdit->text() != "")
	{
		if (Packaging::ExtractPackage(firmwarePackageLineEdit->text(), &loadedPackageData))
			UpdatePackageUserInterface();
		else
			loadedPackageData.Clear();
	}
}

void MainWindow::OpenDeveloperHomepage(void)
{
	if(!QDesktopServices::openUrl(QUrl(loadedPackageData.GetFirmwareInfo().GetUrl(), QUrl::TolerantMode)))
		Alerts::DisplayWarning(QString("Cannot open invalid URL:\n%1").arg(loadedPackageData.GetFirmwareInfo().GetUrl()));
}

void MainWindow::OpenDeveloperDonationWebpage(void)
{
	if (!QDesktopServices::openUrl(QUrl(loadedPackageData.GetFirmwareInfo().GetDonateUrl(), QUrl::TolerantMode)))
		Alerts::DisplayWarning(QString("Cannot open invalid URL:\n%1").arg(loadedPackageData.GetFirmwareInfo().GetDonateUrl()));
}

void MainWindow::LoadFirmwarePackage(void)
{
	workingPackageData.Clear();
	currentPitData.Clear();
	
	workingPackageData.GetFiles().append(loadedPackageData.GetFiles());
	loadedPackageData.RemoveAllFiles();

	const QList<FileInfo> packageFileInfos = loadedPackageData.GetFirmwareInfo().GetFileInfos();

	for (int i = 0; i < packageFileInfos.length(); i++)
	{
		bool fileFound = false;

		for (int j = 0; j < workingPackageData.GetFiles().length(); j++)
		{
			if (workingPackageData.GetFiles()[j]->fileTemplate() == ("XXXXXX-" + packageFileInfos[i].GetFilename()))
			{
				FileInfo partitionInfo(packageFileInfos[i].GetPartitionId(), QDir::current().absoluteFilePath(workingPackageData.GetFiles()[j]->fileName()));
				workingPackageData.GetFirmwareInfo().GetFileInfos().append(partitionInfo);

				fileFound = true;
				break;
			}
		}

		if (!fileFound)
			Alerts::DisplayWarning(QString("%1 is missing from the package.").arg(packageFileInfos[i].GetFilename()));
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
				Alerts::DisplayError("Failed to read PIT file.");

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
	workingPackageData.GetFirmwareInfo().SetNoReboot(loadedPackageData.GetFirmwareInfo().GetNoReboot());

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
			Alerts::DisplayError("Firmware package includes invalid partition IDs.");

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
	noRebootCheckBox->setEnabled(true);
	noRebootCheckBox->setChecked(workingPackageData.GetFirmwareInfo().GetNoReboot());

	partitionsListWidget->setEnabled(true);
	addPartitionButton->setEnabled(true);
	removePartitionButton->setEnabled(partitionsListWidget->currentRow() >= 0);

	pitLineEdit->setText(workingPackageData.GetFirmwareInfo().GetPitFilename());

	functionTabWidget->setCurrentWidget(flashTab);

	UpdateInterfaceAvailability();
}

void MainWindow::SelectPartitionName(int index)
{
	if (!populatingPartitionNames && index != -1 && index != unusedPartitionIds.length())
	{
		unsigned int newPartitionIndex = unusedPartitionIds[index];
		unusedPartitionIds.removeAt(index);

		FileInfo& fileInfo = workingPackageData.GetFirmwareInfo().GetFileInfos()[partitionsListWidget->currentRow()];
		unusedPartitionIds.append(fileInfo.GetPartitionId());
		fileInfo.SetPartitionId(newPartitionIndex);

		PitEntry *pitEntry = currentPitData.FindEntry(newPartitionIndex);

		QString title("File");

		if (pitEntry && strlen(pitEntry->GetFlashFilename()) > 0)
			title += " (" + QString(pitEntry->GetFlashFilename()) + ")";

		partitionFileGroup->setTitle(title);

		if (pitEntry && !fileInfo.GetFilename().isEmpty())
		{
			QString partitionFilename = pitEntry->GetFlashFilename();
			int lastPeriod = partitionFilename.lastIndexOf(QChar('.'));

			if (lastPeriod >= 0)
			{
				QString partitionFileExtension = partitionFilename.mid(lastPeriod + 1);

				lastPeriod = fileInfo.GetFilename().lastIndexOf(QChar('.'));

				if (lastPeriod < 0 || fileInfo.GetFilename().mid(lastPeriod + 1) != partitionFileExtension)
					Alerts::DisplayWarning(QString("%1 partition expects files with file extension \"%2\".").arg(pitEntry->GetPartitionName(), partitionFileExtension));
			}
		}

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
		FileInfo& fileInfo = workingPackageData.GetFirmwareInfo().GetFileInfos()[partitionsListWidget->currentRow()];
		PitEntry *pitEntry = currentPitData.FindEntry(fileInfo.GetPartitionId());

		QString partitionFilename = pitEntry->GetFlashFilename();
		int lastPeriod = partitionFilename.lastIndexOf(QChar('.'));

		if (lastPeriod >= 0)
		{
			QString partitionFileExtension = partitionFilename.mid(lastPeriod + 1);

			lastPeriod = path.lastIndexOf(QChar('.'));

			if (lastPeriod < 0 || path.mid(lastPeriod + 1) != partitionFileExtension)
				Alerts::DisplayWarning(QString("%1 partition expects files with file extension \"%2\".").arg(pitEntry->GetPartitionName(), partitionFileExtension));
		}

		fileInfo.SetFilename(path);
		partitionFileLineEdit->setText(path);

		pitBrowseButton->setEnabled(true);
		partitionsListWidget->setEnabled(true);
		UpdateInterfaceAvailability();

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

		QString title("File");

		PitEntry *pitEntry = currentPitData.FindEntry(partitionInfo.GetPartitionId());

		if (pitEntry && strlen(pitEntry->GetFlashFilename()) > 0)
			title += " (" + QString(pitEntry->GetFlashFilename()) + ")";

		partitionFileGroup->setTitle(title);
	}
	else
	{
		UpdatePartitionNamesInterface();

		partitionIdLineEdit->clear();
		partitionFileLineEdit->clear();
		partitionFileBrowseButton->setEnabled(false);

		removePartitionButton->setEnabled(false);

		partitionFileGroup->setTitle("File");
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

	UpdateInterfaceAvailability();
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
	UpdateInterfaceAvailability();
}

void MainWindow::SelectPit(void)
{
	QString path = PromptFileSelection("Select PIT", "*.pit");
	bool validPit = path != "";

	if (validPit)
	{
		// In order to map files in the old PIT to file in the new one, we first must use partition names instead of IDs.
		QList<FileInfo> fileInfos = workingPackageData.GetFirmwareInfo().GetFileInfos();

		int partitionNamesCount = fileInfos.length();
		QString *partitionNames = new QString[fileInfos.length()];
		for (int i = 0; i < fileInfos.length(); i++)
			partitionNames[i] = currentPitData.FindEntry(fileInfos[i].GetPartitionId())->GetPartitionName();

		currentPitData.Clear();

		QFile pitFile(path);

		if (ReadPit(&pitFile))
		{
			workingPackageData.GetFirmwareInfo().SetPitFilename(path);

			partitionsListWidget->clear();
			int partitionInfoIndex = 0;

			for (int i = 0; i < partitionNamesCount; i++)
			{
				const PitEntry *pitEntry = currentPitData.FindEntry(partitionNames[i].toLatin1().constData());
				
				if (pitEntry)
				{
					fileInfos[partitionInfoIndex++].SetPartitionId(pitEntry->GetIdentifier());
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

		// If the selected PIT was invalid, attempt to reload the old one.
		if (!validPit)
		{
			Alerts::DisplayError("The file selected was not a valid PIT file.");

			if (!workingPackageData.GetFirmwareInfo().GetPitFilename().isEmpty())
			{
				QFile originalPitFile(workingPackageData.GetFirmwareInfo().GetPitFilename());

				if (ReadPit(&originalPitFile))
				{
					validPit = true;
				}
				else
				{
					Alerts::DisplayError("Failed to reload working PIT data.");

					workingPackageData.Clear();
					partitionsListWidget->clear();
				}
			}
		}

		UpdateUnusedPartitionIds();

		delete [] partitionNames;

		pitLineEdit->setText(workingPackageData.GetFirmwareInfo().GetPitFilename());

		repartitionCheckBox->setEnabled(validPit);
		noRebootCheckBox->setEnabled(validPit);
		partitionsListWidget->setEnabled(validPit);

		addPartitionButton->setEnabled(validPit);
		removePartitionButton->setEnabled(validPit && partitionsListWidget->currentRow() >= 0);

		UpdateInterfaceAvailability();
	}
}


void MainWindow::SetRepartition(int enabled)
{
	workingPackageData.GetFirmwareInfo().SetRepartition(enabled);

	repartitionCheckBox->setChecked(enabled);
}

void MainWindow::SetNoReboot(int enabled)
{
	workingPackageData.GetFirmwareInfo().SetNoReboot(enabled);

	noRebootCheckBox->setChecked(enabled);
}

void MainWindow::SetResume(bool enabled)
{
	resume = enabled;

	actionResumeConnection->setChecked(enabled);
	resumeCheckbox->setChecked(enabled);
}

void MainWindow::SetResume(int enabled)
{
	SetResume(enabled != 0);
}

void MainWindow::StartFlash(void)
{
	outputPlainTextEdit->clear();

	heimdallState = HeimdallState::Flashing;
	heimdallFailed = false;

	const FirmwareInfo& firmwareInfo = workingPackageData.GetFirmwareInfo();
	const QList<FileInfo>& fileInfos = firmwareInfo.GetFileInfos();
	
	QStringList arguments;
	arguments.append("flash");

	if (firmwareInfo.GetRepartition())
		arguments.append("--repartition");

	arguments.append("--pit");
	arguments.append(firmwareInfo.GetPitFilename());

	for (int i = 0; i < fileInfos.length(); i++)
	{
		QString flag;
		flag.sprintf("--%u", fileInfos[i].GetPartitionId());

		arguments.append(flag);
		arguments.append(fileInfos[i].GetFilename());
	}

	if (firmwareInfo.GetNoReboot())
	{
		arguments.append("--no-reboot");
		heimdallState |= HeimdallState::NoReboot;
	}

	if (resume)
		arguments.append("--resume");

	if (verboseOutput)
		arguments.append("--verbose");

	arguments.append("--stdout-errors");

	StartHeimdall(arguments);
}

void MainWindow::FirmwareNameChanged(const QString& text)
{
	workingPackageData.GetFirmwareInfo().SetName(text);
	UpdateInterfaceAvailability();
}

void MainWindow::FirmwareVersionChanged(const QString& text)
{
	workingPackageData.GetFirmwareInfo().SetVersion(text);
	UpdateInterfaceAvailability();
}

void MainWindow::PlatformNameChanged(const QString& text)
{
	workingPackageData.GetFirmwareInfo().GetPlatformInfo().SetName(text);
	UpdateInterfaceAvailability();
}

void MainWindow::PlatformVersionChanged(const QString& text)
{
	workingPackageData.GetFirmwareInfo().GetPlatformInfo().SetVersion(text);
	UpdateInterfaceAvailability();
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
	UNUSED(text);

	UpdateCreatePackageInterfaceAvailability();
}

void MainWindow::SelectDeveloper(int row)
{
	UNUSED(row);

	UpdateCreatePackageInterfaceAvailability();
}

void MainWindow::AddDeveloper(void)
{
	workingPackageData.GetFirmwareInfo().GetDevelopers().append(createDeveloperNameLineEdit->text());

	createDevelopersListWidget->addItem(createDeveloperNameLineEdit->text());
	createDeveloperNameLineEdit->clear();
	
	UpdateCreatePackageInterfaceAvailability();
}

void MainWindow::RemoveDeveloper(void)
{
	workingPackageData.GetFirmwareInfo().GetDevelopers().removeAt(createDevelopersListWidget->currentRow());

	QListWidgetItem *item = createDevelopersListWidget->currentItem();
	createDevelopersListWidget->setCurrentRow(-1);
	delete item;

	removeDeveloperButton->setEnabled(false);
	
	UpdateInterfaceAvailability();
}

void MainWindow::DeviceInfoChanged(const QString& text)
{
	UNUSED(text);

	if (deviceManufacturerLineEdit->text().isEmpty() || deviceNameLineEdit->text().isEmpty() || deviceProductCodeLineEdit->text().isEmpty())
		addDeviceButton->setEnabled(false);
	else
		addDeviceButton->setEnabled(true);
}

void MainWindow::SelectDevice(int row)
{
	removeDeviceButton->setEnabled(row >= 0);
}

void MainWindow::AddDevice(void)
{
	workingPackageData.GetFirmwareInfo().GetDeviceInfos().append(DeviceInfo(deviceManufacturerLineEdit->text(), deviceProductCodeLineEdit->text(),
		deviceNameLineEdit->text()));

	createDevicesListWidget->addItem(deviceManufacturerLineEdit->text() + " " + deviceNameLineEdit->text() + ": " + deviceProductCodeLineEdit->text());
	deviceManufacturerLineEdit->clear();
	deviceNameLineEdit->clear();
	deviceProductCodeLineEdit->clear();
	
	UpdateInterfaceAvailability();
}

void MainWindow::RemoveDevice(void)
{
	workingPackageData.GetFirmwareInfo().GetDeviceInfos().removeAt(createDevicesListWidget->currentRow());

	QListWidgetItem *item = createDevicesListWidget->currentItem();
	createDevicesListWidget->setCurrentRow(-1);
	delete item;

	removeDeviceButton->setEnabled(false);
	
	UpdateInterfaceAvailability();
}
			
void MainWindow::BuildPackage(void)
{
	QString packagePath = PromptFileCreation("Save Package", "Firmware Package (*.gz)");

	if (!packagePath.isEmpty())
	{
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

		Packaging::BuildPackage(packagePath, workingPackageData.GetFirmwareInfo());
	}
}

void MainWindow::DetectDevice(void)
{
	deviceDetectedRadioButton->setChecked(false);
	utilityOutputPlainTextEdit->clear();

	heimdallState = HeimdallState::DetectingDevice;
	heimdallFailed = false;
	
	QStringList arguments;
	arguments.append("detect");

	if (verboseOutput)
		arguments.append("--verbose");

	arguments.append("--stdout-errors");

	StartHeimdall(arguments);
}

void MainWindow::ClosePcScreen(void)
{
	utilityOutputPlainTextEdit->clear();

	heimdallState = HeimdallState::ClosingPcScreen;
	heimdallFailed = false;
	
	QStringList arguments;
	arguments.append("close-pc-screen");
	
	if (resume)
		arguments.append("--resume");

	if (verboseOutput)
		arguments.append("--verbose");

	arguments.append("--stdout-errors");

	StartHeimdall(arguments);
}

void MainWindow::SelectPitDestination(void)
{
	QString path = PromptFileCreation("Save PIT", "*.pit");

	if (path != "")
	{
		if (!path.endsWith(".pit"))
			path.append(".pit");

		pitDestinationLineEdit->setText(path);

		UpdateInterfaceAvailability();
	}
}

void MainWindow::DownloadPit(void)
{
	deviceDetectedRadioButton->setChecked(false);
	utilityOutputPlainTextEdit->clear();

	heimdallState = HeimdallState::DownloadingPit | HeimdallState::NoReboot;
	heimdallFailed = false;
	
	QStringList arguments;
	arguments.append("download-pit");

	arguments.append("--output");
	arguments.append(pitDestinationLineEdit->text());

	arguments.append("--no-reboot");

	if (resume)
		arguments.append("--resume");

	if (verboseOutput)
		arguments.append("--verbose");

	arguments.append("--stdout-errors");

	StartHeimdall(arguments);
}

void MainWindow::DevicePrintPitToggled(bool checked)
{
	if (checked)
	{
		if (printPitLocalFileRadioBox->isChecked())
			printPitLocalFileRadioBox->setChecked(false);
	}

	UpdateUtilitiesInterfaceAvailability();
}

void MainWindow::LocalFilePrintPitToggled(bool checked)
{
	if (checked)
	{
		if (printPitDeviceRadioBox->isChecked())
			printPitDeviceRadioBox->setChecked(false);
	}

	UpdateUtilitiesInterfaceAvailability();
}

void MainWindow::SelectPrintPitFile(void)
{
	QString path = PromptFileSelection("Select PIT", "*.pit");

	if (path.length() > 0)
	{
		printLocalPitLineEdit->setText(path);
		printPitButton->setEnabled(true);
	}
	else
	{
		printPitButton->setEnabled(false);
	}
}

void MainWindow::PrintPit(void)
{
	utilityOutputPlainTextEdit->clear();

	heimdallState = HeimdallState::PrintingPit | HeimdallState::NoReboot;
	heimdallFailed = false;
	
	QStringList arguments;
	arguments.append("print-pit");

	if (printPitLocalFileRadioBox->isChecked())
	{
		arguments.append("--file");
		arguments.append(printLocalPitLineEdit->text());
	}

	arguments.append("--stdout-errors");
	arguments.append("--no-reboot");
	
	if (resume)
		arguments.append("--resume");

	if (verboseOutput)
		arguments.append("--verbose");

	StartHeimdall(arguments);
}

void MainWindow::HandleHeimdallStdout(void)
{
	QString output = heimdallProcess.readAll();

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

	output.remove(QChar('\b'));
	output.replace(QChar('%'), QString("%\n"));

	if (!!(heimdallState & HeimdallState::Flashing))
	{
		outputPlainTextEdit->insertPlainText(output);
		outputPlainTextEdit->ensureCursorVisible();
	}
	else
	{
		utilityOutputPlainTextEdit->insertPlainText(output);
		utilityOutputPlainTextEdit->ensureCursorVisible();
	}
}

void MainWindow::HandleHeimdallReturned(int exitCode, QProcess::ExitStatus exitStatus)
{
	HandleHeimdallStdout();

	if (exitStatus == QProcess::NormalExit && exitCode == 0)
	{
		SetResume(!!(heimdallState & HeimdallState::NoReboot));

		if (!!(heimdallState & HeimdallState::Flashing))
		{
			flashLabel->setText("Flash completed successfully!");
		}
		else if (!!(heimdallState & HeimdallState::DetectingDevice))
		{
			deviceDetectedRadioButton->setChecked(true);
		}
	}
	else
	{
		if (!!(heimdallState & HeimdallState::Flashing))
		{
			QString error = heimdallProcess.readAllStandardError();

			int lastNewLineChar = error.lastIndexOf('\n');

			if (lastNewLineChar == 0)
				error = error.mid(1).remove("ERROR: ");
			else
				error = error.left(lastNewLineChar).remove("ERROR: ");

			flashLabel->setText(error);
		}
		else if (!!(heimdallState & HeimdallState::DetectingDevice))
		{
			deviceDetectedRadioButton->setChecked(false);
		}
	}

	heimdallState = HeimdallState::Stopped;
	flashProgressBar->setValue(0);
	flashProgressBar->setEnabled(false);
	UpdateInterfaceAvailability();
}

void MainWindow::HandleHeimdallError(QProcess::ProcessError error)
{	
	if (error == QProcess::FailedToStart || error == QProcess::Timedout)
	{
		if (!!(heimdallState & HeimdallState::Flashing))
		{
			flashLabel->setText("Failed to start Heimdall!");
			flashProgressBar->setEnabled(false);
		}
		else
		{
			utilityOutputPlainTextEdit->setPlainText("\nFRONTEND ERROR: Failed to start Heimdall!");
		}

		heimdallFailed = true;
		heimdallState = HeimdallState::Stopped;
		UpdateInterfaceAvailability();
	}
	else if (error == QProcess::Crashed)
	{
		if (!!(heimdallState & HeimdallState::Flashing))
		{
			flashLabel->setText("Heimdall crashed!");
			flashProgressBar->setEnabled(false);
		}
		else
		{
			utilityOutputPlainTextEdit->appendPlainText("\nFRONTEND ERROR: Heimdall crashed!");
		}
		
		heimdallState = HeimdallState::Stopped;
		UpdateInterfaceAvailability();
	}
	else
	{
		if (!!(heimdallState & HeimdallState::Flashing))
		{
			flashLabel->setText("Heimdall reported an unknown error!");
			flashProgressBar->setEnabled(false);
		}
		else
		{
			utilityOutputPlainTextEdit->appendPlainText("\nFRONTEND ERROR: Heimdall reported an unknown error!");
		}
		
		heimdallState = HeimdallState::Stopped;
		UpdateInterfaceAvailability();
	}
}
