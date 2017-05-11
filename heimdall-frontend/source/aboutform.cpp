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
#include <QDir>
#include <QProcess>

// Heimdall Frontend
#include "aboutform.h"

#include <QStringList>

#define UNUSED(x) (void)(x)

using namespace HeimdallFrontend;

AboutForm::AboutForm(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	
	// Heimdall Command Line
	QObject::connect(&heimdallProcess, SIGNAL(readyRead()), this, SLOT(HandleHeimdallStdout()));
	QObject::connect(&heimdallProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(HandleHeimdallReturned(int, QProcess::ExitStatus)));
	QObject::connect(&heimdallProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(HandleHeimdallError(QProcess::ProcessError)));

	heimdallFailed = false;

	RetrieveHeimdallVersion();
}

void AboutForm::RetrieveHeimdallVersion(void)
{
	heimdallProcess.setReadChannel(QProcess::StandardOutput);
	
	heimdallProcess.start("heimdall", QStringList("version"));
	heimdallProcess.waitForFinished(350);
	
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
		
		int pathIndex = -1;
		
		while (heimdallFailed && ++pathIndex < paths.length())
		{
			QString heimdallPath = paths[pathIndex];
			
			if (heimdallPath.length() > 0)
			{
				heimdallFailed = false;
				
				if (heimdallPath[heimdallPath.length() - 1] != QDir::separator())
					heimdallPath += QDir::separator();
				
				heimdallPath += "heimdall";
				
				heimdallProcess.start(heimdallPath, QStringList("version"));
				heimdallProcess.waitForFinished(350);
			}
		}
		
		if (heimdallFailed)
			versionCopyrightLabel->setText(versionCopyrightLabel->text().replace("%HEIMDALL-VERSION%", ""));
	}
}

void AboutForm::HandleHeimdallStdout(void)
{
	QString version = heimdallProcess.readAll();

	if (version.length() > 0)
	{
		if (version.at(0) == QChar('v'))
			version = version.mid(1);

		versionCopyrightLabel->setText(versionCopyrightLabel->text().replace("%HEIMDALL-VERSION%", "Version " + version + "<br />"));
	}
}

void AboutForm::HandleHeimdallReturned(int exitCode, QProcess::ExitStatus exitStatus)
{
	UNUSED(exitCode);
	UNUSED(exitStatus);

	// If for some reason %HEIMDALL-VERSION% hasn't been replaced yet, we'll replace it with an empty string.
	versionCopyrightLabel->setText(versionCopyrightLabel->text().replace("%HEIMDALL-VERSION%", ""));
}

void AboutForm::HandleHeimdallError(QProcess::ProcessError error)
{
	UNUSED(error);

	heimdallFailed = true;
}

