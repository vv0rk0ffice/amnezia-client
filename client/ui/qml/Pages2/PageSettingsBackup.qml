import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import PageEnum 1.0

import "./"
import "../Controls2"
import "../Config"
import "../Controls2/TextTypes"

PageType {
    id: root

    Connections {
        target: SettingsController

        function onChangeSettingsErrorOccurred(errorMessage) {
            PageController.showErrorMessage(errorMessage)
        }

        function onRestoreBackupFinished() {
            PageController.showNotificationMessage(qsTr("Settings restored from backup file"))
            goToStartPage()
            PageController.goToPageHome()
        }
    }

    BackButtonType {
        id: backButton

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 20
    }

    FlickableType {
        id: fl
        anchors.top: backButton.bottom
        anchors.bottom: parent.bottom
        contentHeight: content.height

        ColumnLayout {
            id: content

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 16
            anchors.rightMargin: 16

            spacing: 16

            HeaderType {
                Layout.fillWidth: true

                headerText: qsTr("Backup")
            }

            ListItemTitleType {
                Layout.fillWidth: true
                Layout.topMargin: 10

                text: qsTr("Configuration backup")
            }

            CaptionTextType {
                Layout.fillWidth: true
                Layout.topMargin: -12

                text: qsTr("It will help you instantly restore connection settings at the next installation")
                color: "#878B91"
            }

            BasicButtonType {
                Layout.fillWidth: true
                Layout.topMargin: 14

                text: qsTr("Make a backup")

                onClicked: {
                    PageController.showBusyIndicator(true)
                    SettingsController.backupAppConfig()
                    PageController.showBusyIndicator(false)
                }
            }

            BasicButtonType {
                Layout.fillWidth: true
                Layout.topMargin: -8

                defaultColor: "transparent"
                hoveredColor: Qt.rgba(1, 1, 1, 0.08)
                pressedColor: Qt.rgba(1, 1, 1, 0.12)
                disabledColor: "#878B91"
                textColor: "#D7D8DB"
                borderWidth: 1

                text: qsTr("Restore from backup")

                onClicked: {
                    PageController.showBusyIndicator(true)
                    SettingsController.restoreAppConfig()
                    PageController.showBusyIndicator(false)
                }
            }
        }
    }
}
