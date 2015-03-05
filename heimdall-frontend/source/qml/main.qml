import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

ApplicationWindow {
	title: qsTr("Heimdall Frontend")
	minimumWidth: 620
	minimumHeight: 460
	width: 640
	height: 480
	visible: true

	menuBar: MenuBar {
		Menu {
			title: qsTr("&File")
			MenuItem {
				text: qsTr("&Open")
				onTriggered: messageDialog.show(qsTr("Open action triggered"));
			}
			MenuItem {
				text: qsTr("E&xit")
				onTriggered: Qt.quit();
			}
		}
	}

	StackView {
		id: stack
		initialItem: root
		anchors.fill: parent

		Root {
			id: root
			anchors.fill: parent
		}

		function transitionFinished(properties)
		{
			properties.exitItem.x = 0
		}

		delegate: StackViewDelegate {
			pushTransition: StackViewTransition {
				PropertyAnimation {
					target: enterItem
					property: "x"
					from: target.width
					to: 0
					duration: 300
				}
				PropertyAnimation {
					target: exitItem
					property: "x"
					from: 0
					to: target.width
					duration: 300
				}
			}
		}
	}
}
