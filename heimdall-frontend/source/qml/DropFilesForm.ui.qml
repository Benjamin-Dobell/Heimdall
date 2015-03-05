import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

Rectangle {
	Layout.preferredWidth: 420
	Layout.preferredHeight: 440
	color: "#eeeeee"

	DropArea {
		id: dropFilesArea
	}

	Column {
		id: dropFilesColumn
		spacing: 12
		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenter: parent.horizontalCenter

		Text {
			id: dragFilesText
			text: qsTr("Drop firmware files here")
			opacity: 1
			anchors.horizontalCenter: parent.horizontalCenter
			wrapMode: Text.WordWrap
			horizontalAlignment: Text.AlignLeft
			font.pixelSize: 18
		}

		Image {
			id: image1
			width: 100
			height: 100
			opacity: 1
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

 Button {
	 id: flashButton
	 x: 556
	 y: 606
	 text: qsTr("Next")
	 visible: false
	 enabled: true
	 isDefault: true
	 anchors.bottom: parent.bottom
	 anchors.bottomMargin: 8
	 anchors.right: parent.right
	 anchors.rightMargin: 8
 }
}
