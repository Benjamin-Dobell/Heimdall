import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

Item {
	width: 640
	height: 460

	RowLayout {
		anchors.fill: parent
		spacing: 0

		Rectangle {
			color: "#222222"
			width: 172
			anchors.top: parent.top
			anchors.topMargin: 0
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 0

			ListView {
				id: list
				boundsBehavior: Flickable.StopAtBounds
				anchors.fill: parent
				focus: true

				highlight: Item {
					Rectangle {
						height: 40
						anchors.right: parent.right
						anchors.rightMargin: 0
						anchors.left: parent.left
						anchors.leftMargin: 0
						radius: 8
						color: "#333333"
					}
					Rectangle {
						height: 40
						anchors.right: parent.right
						anchors.rightMargin: 8
						anchors.left: parent.left
						anchors.leftMargin: 0
						color: "#333333"
					}
				}

				delegate: Item {
					height: 40
					anchors.right: parent.right
					anchors.rightMargin: 12
					anchors.left: parent.left
					anchors.leftMargin: 0

					Row {
						spacing: 8

						Rectangle {
							width: 40
							height: 40
							color: colorCode
						}

						Text {
							text: name
							color: '#ffffff'
							anchors.verticalCenter: parent.verticalCenter
						}
					}

					MouseArea {
						anchors.fill: parent
						hoverEnabled: false

						onClicked: {
							list.currentIndex = index
							list.forceActiveFocus()
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
						name: "Settings"
						colorCode: "green"
					}
				}
			}
		}

		Rectangle {
			Layout.fillWidth: true
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 0
			anchors.top: parent.top
			anchors.topMargin: 0

			DropFiles {
				id: dropFiles
				anchors.fill: parent
			}
		}
	}

}
