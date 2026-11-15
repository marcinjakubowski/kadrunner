import QtQuick
import QtQuick.Layouts

Item {
    id: settingsPanel

    required property var historyModel
    signal interacted()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Default items when empty"
                color: "#cfd4dc"
                font.pixelSize: 15
                Layout.fillWidth: true
            }

            Row {
                spacing: 6
                Repeater {
                    model: [
                        { label: "Most Often", value: "often" },
                        { label: "Most Recent", value: "recent" },
                        { label: "Off", value: "off" }
                    ]
                    delegate: Rectangle {
                        required property var modelData
                        readonly property bool selected: settingsPanel.historyModel.mode === modelData.value
                        width: modeLabel.implicitWidth + 20
                        height: 32
                        radius: 6
                        color: selected ? "#4b6eaf" : (modeMouse.containsMouse ? "#3a3d42" : "#323438")
                        border.width: 1
                        border.color: selected ? "#6286c9" : "#44474e"

                        Text {
                            id: modeLabel
                            anchors.centerIn: parent
                            text: modelData.label
                            color: selected ? "#ffffff" : "#c5cad3"
                            font.pixelSize: 13
                        }

                        MouseArea {
                            id: modeMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                settingsPanel.historyModel.mode = modelData.value
                                settingsPanel.interacted()
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            opacity: settingsPanel.historyModel.mode === "off" ? 0.45 : 1.0

            Text {
                text: "Number of default items"
                color: "#cfd4dc"
                font.pixelSize: 15
                Layout.fillWidth: true
            }

            Row {
                spacing: 8

                Rectangle {
                    width: 32
                    height: 32
                    radius: 6
                    color: minusMouse.containsMouse ? "#3a3d42" : "#323438"
                    border.width: 1
                    border.color: "#44474e"
                    Text {
                        anchors.centerIn: parent
                        text: "−"
                        color: "#eceff4"
                        font.pixelSize: 16
                    }
                    MouseArea {
                        id: minusMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        enabled: settingsPanel.historyModel.mode !== "off"
                        onClicked: {
                            settingsPanel.historyModel.maxItems -= 1
                            settingsPanel.interacted()
                        }
                    }
                }

                Text {
                    width: 28
                    height: 32
                    text: settingsPanel.historyModel.maxItems
                    color: "#eceff4"
                    font.pixelSize: 15
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Rectangle {
                    width: 32
                    height: 32
                    radius: 6
                    color: plusMouse.containsMouse ? "#3a3d42" : "#323438"
                    border.width: 1
                    border.color: "#44474e"
                    Text {
                        anchors.centerIn: parent
                        text: "+"
                        color: "#eceff4"
                        font.pixelSize: 16
                    }
                    MouseArea {
                        id: plusMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        enabled: settingsPanel.historyModel.mode !== "off"
                        onClicked: {
                            settingsPanel.historyModel.maxItems += 1
                            settingsPanel.interacted()
                        }
                    }
                }
            }
        }

        Text {
            text: "Hotkey: System Settings → Keyboard → Shortcuts → KadRunner (default Meta / Meta+/)"
            color: "#7a808a"
            font.pixelSize: 12
        }
    }
}
