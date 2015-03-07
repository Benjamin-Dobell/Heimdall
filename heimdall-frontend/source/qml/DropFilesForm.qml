import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

Rectangle {
	id: dropFilesForm
	color: "#eeeeee"

	property alias dropFilesArea: dropFilesArea
	property alias fileGridView: fileGridView
	property alias fileGridContainer: fileGridContainer
	property alias browseButton: browseButton
	property alias bottomBrowseButton: bottomBrowseButton
	property alias nextButton: nextButton
	property alias dropFilesContainer: dropFilesColumn
	property alias bottomButtonsContainer: bottomButtonsContainer

	signal keyPressed(var event)

	DropArea {
		id: dropFilesArea
		anchors.fill: parent

		onEntered: {
			background.color = "#cccccc";
		}
		onDropped: {
			background.color = "#eeeeee";
		}
		onExited: {
			background.color = "#eeeeee";
		}
	}

	Column {
		id: dropFilesColumn
		spacing: 12
		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenter: parent.horizontalCenter

		Text {
			id: dragFilesText
			text: qsTr("Drop firmware files here")
			anchors.horizontalCenter: parent.horizontalCenter
			wrapMode: Text.WordWrap
			horizontalAlignment: Text.AlignLeft
			font.pixelSize: 18
		}

		Image {
			width: 100
			height: 100
			anchors.horizontalCenter: parent.horizontalCenter
			source: "drop_zone.svg"
		}

		Button {
			id: browseButton
			text: qsTr("Browse")
			isDefault: true
			anchors.horizontalCenter: parent.horizontalCenter
		}
	}

	Item {
		id: fileGridContainer
		visible: false
		anchors.fill: parent
		anchors.bottomMargin: 36

		ScrollView {
			anchors.fill: parent

			GridView {
				id: fileGridView
				flow: GridView.FlowLeftToRight
				cellWidth: 148
				cellHeight: 120
				boundsBehavior: Flickable.StopAtBounds
				focus: true
				anchors {
					fill: parent
					margins: 4
				}

				highlight: Rectangle {
					color: "#ccddff"
					radius: 8
				}

				delegate: Item {
					width: 148
					height: 120

					Column {
						spacing: 8
						anchors.centerIn: parent

						Image {
							width: 80
							anchors.horizontalCenter: parent.horizontalCenter
							fillMode: Image.PreserveAspectFit
							source: model.icon
						}

						Text {
							width: 140
							anchors.horizontalCenter: parent.horizontalCenter
							wrapMode: Text.NoWrap
							horizontalAlignment: Text.AlignHCenter
							text: model.text
							elide: Text.ElideMiddle
						}
					}

					MouseArea {
						anchors.fill: parent

						onClicked: {
							fileGridView.currentIndex = index
							fileGridView.forceActiveFocus()
						}
					}
				}

				Keys.onPressed: {
					var columns = Math.floor(fileGridView.width / fileGridView.cellWidth);
					var index = fileGridView.currentIndex;

					switch (event.key) {
						case Qt.Key_Left:
							index--;
							break;
						case Qt.Key_Right:
							index++;
							break;
						case Qt.Key_Up:
							index -= columns;
							break;
						case Qt.Key_Down:
							index += columns;
							break;
					}

					var upperBound = fileGridView.model.count - 1;
					var partialRowCount = fileGridView.model.count % columns;

					if (partialRowCount > 0) {
						upperBound += columns - partialRowCount;
					}

					if (index >= 0 && index <= upperBound) {
						index = Math.min(fileGridView.model.count - 1, index);
						fileGridView.currentIndex = index;
					}

					dropFilesForm.keyPressed(event);
				}
			}
		}
	}

	Rectangle {
		id: bottomButtonsContainer
		height: 36
		color: "#dddddd"
		visible: false

		anchors {
			bottom: parent.bottom
			left: parent.left
			right: parent.right
		}

		Button {
			id: bottomBrowseButton
			text: qsTr("Browse")
			anchors {
				bottom: parent.bottom
				bottomMargin: 4
				left: parent.left
				leftMargin: 8
				verticalCenter: parent.verticalCenter
			}
		}

		Button {
			id: nextButton
			text: qsTr("Next")
			isDefault: true
			anchors {
				bottom: parent.bottom
				bottomMargin: 4
				right: parent.right
				rightMargin: 8
				verticalCenter: parent.verticalCenter
			}
		}
	}
}
