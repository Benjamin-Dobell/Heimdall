import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import "ArrayExtensions.js" as ArrayExtensions
import "FileUtils.js" as FileUtils
import HeimdallFrontend 1.0 as Native

DropFilesForm {
	id: background
	property var fileUrls: []
	signal nextPressed(var files)

	ListModel {
		id: fileModel
	}

	Native.PackageData {
		id: packageData
	}

	function setFileGridVisible(visible) {
		if (fileGridContainer.visible !== visible) {
			dropFilesContainer.visible = !visible;
			bottomButtonsContainer.visible = visible;
			fileGridContainer.visible = visible;

			if (visible) {
				fileGridView.forceActiveFocus();
			}
		}
	}

	function addFiles(urls) {
		var count = urls.length;
		if (count > 0) {
			for (var i = 0; i < count; i++) {
				if (FileUtils.isFile(urls[i])) {
					if (FileUtils.isArchive(urls[i])) {
						var packageData = Native.Firmware.extractArchive(urls[i]);
						packageData.filePaths.forEach(function(path) {
							var filename = FileUtils.filenameFromPath(path);
							fileModel.append({ icon: "drop_zone.svg", text: filename });
						});
						fileUrls.extend(packageData.filePaths);
					} else {
						var filename = FileUtils.filenameFromUrl(urls[i]);
						fileModel.append({ icon: "drop_zone.svg", text: filename });
						fileUrls.push(urls[i]);
					}
				}
			}

			setFileGridVisible(true);
		}
	}

	function removeFile(index) {
		if (index >= 0) {
			fileModel.remove(index);
			fileUrls.splice(index, 1);

			if (fileUrls.length == 0) {
				setFileGridVisible(false);
			}
		}
	}

	onKeyPressed: {
		if (event.key === Qt.Key_Delete || event.key === Qt.Key_Backspace) {
			event.accepted = true;
			removeFile(fileGridView.currentIndex);
		}
	}

	dropFilesArea {
		onDropped: {
			if ((drop.action == Qt.CopyAction || drop.action == Qt.MoveAction) && drop.urls.length > 0) {
				addFiles(drop.urls);
				drop.acceptProposedAction();
				drop.accept(Qt.CopyAction);
			}
		}
	}

	browseButton.onClicked: {
		browseDialog.open()
	}

	bottomBrowseButton.onClicked: {
		browseDialog.open()
	}

	nextButton {
		onClicked: nextPressed(fileUrls)
	}

	fileGridView {
		model: fileModel
	}

	FileDialog {
		id: browseDialog
		title: "Select firmware file(s)"
		selectMultiple: true
		selectFolder: false
		onAccepted: {
			addFiles(browseDialog.fileUrls);
		}
	}
}
