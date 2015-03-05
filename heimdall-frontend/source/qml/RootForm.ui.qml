import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

Rectangle {
	id: rectangle1
	width: 620
	height: 460
	color: "#eeeeee"

	RowLayout {
		id: rowLayout1
		anchors.fill: parent

		Rectangle {
			id: rectangle6
			color: "#222222"
			width: 180
			anchors.top: parent.top
			anchors.topMargin: 0
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 0

			ListView {
				id: listView1
				anchors.fill: parent
				delegate: Item {
					anchors.right: parent.right
					anchors.rightMargin: 0
					anchors.left: parent.left
					anchors.leftMargin: 0
					height: 40
					Row {
						spacing: 8

						Rectangle {
							width: 40
							height: 40
							color: colorCode
						}

						Text {
							text: name
							color: '#eeeeee'
							anchors.verticalCenter: parent.verticalCenter
						}
					}
				}
				model: ListModel {
					ListElement {
						name: "Load Firmware"
						colorCode: "red"
					}

					ListElement {
						name: "Utilities"
						colorCode: "blue"
					}

					ListElement {
						name: "Configuration"
						colorCode: "green"
					}
				}
			}
		}
		DropFiles {
			id: dropFiles1
			Layout.fillWidth: true
		}
	}

}
