import QtQuick
import QtQuick.Layouts

Item {
    id: settingsPanel

    required property var historyModel
    signal interacted()
    signal moveRequested(int dx, int dy)
    signal resetPositionRequested()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

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
                        height: 30
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
                    width: 30; height: 30; radius: 6
                    color: minusMouse.containsMouse ? "#3a3d42" : "#323438"
                    border.width: 1; border.color: "#44474e"
                    Text { anchors.centerIn: parent; text: "−"; color: "#eceff4"; font.pixelSize: 16 }
                    MouseArea {
                        id: minusMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        enabled: settingsPanel.historyModel.mode !== "off"
                        onClicked: { settingsPanel.historyModel.maxItems -= 1; settingsPanel.interacted() }
                    }
                }

                Text {
                    width: 36; height: 30
                    text: settingsPanel.historyModel.maxItems
                    color: "#eceff4"; font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }

                Rectangle {
                    width: 30; height: 30; radius: 6
                    color: plusMouse.containsMouse ? "#3a3d42" : "#323438"
                    border.width: 1; border.color: "#44474e"
                    Text { anchors.centerIn: parent; text: "+"; color: "#eceff4"; font.pixelSize: 16 }
                    MouseArea {
                        id: plusMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        enabled: settingsPanel.historyModel.mode !== "off"
                        onClicked: { settingsPanel.historyModel.maxItems += 1; settingsPanel.interacted() }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Launcher width"
                color: "#cfd4dc"
                font.pixelSize: 15
                Layout.fillWidth: true
            }

            Row {
                spacing: 8

                Rectangle {
                    width: 30; height: 30; radius: 6
                    color: wMinusMouse.containsMouse ? "#3a3d42" : "#323438"
                    border.width: 1; border.color: "#44474e"
                    Text { anchors.centerIn: parent; text: "−"; color: "#eceff4"; font.pixelSize: 16 }
                    MouseArea {
                        id: wMinusMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: { settingsPanel.historyModel.launcherWidth -= 40; settingsPanel.interacted() }
                    }
                }

                Text {
                    width: 56; height: 30
                    text: settingsPanel.historyModel.launcherWidth + "px"
                    color: "#eceff4"; font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }

                Rectangle {
                    width: 30; height: 30; radius: 6
                    color: wPlusMouse.containsMouse ? "#3a3d42" : "#323438"
                    border.width: 1; border.color: "#44474e"
                    Text { anchors.centerIn: parent; text: "+"; color: "#eceff4"; font.pixelSize: 16 }
                    MouseArea {
                        id: wPlusMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: { settingsPanel.historyModel.launcherWidth += 40; settingsPanel.interacted() }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Max visible result rows"
                color: "#cfd4dc"
                font.pixelSize: 15
                Layout.fillWidth: true
            }

            Row {
                spacing: 8

                Rectangle {
                    width: 30; height: 30; radius: 6
                    color: rMinusMouse.containsMouse ? "#3a3d42" : "#323438"
                    border.width: 1; border.color: "#44474e"
                    Text { anchors.centerIn: parent; text: "−"; color: "#eceff4"; font.pixelSize: 16 }
                    MouseArea {
                        id: rMinusMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: { settingsPanel.historyModel.maxVisibleRows -= 1; settingsPanel.interacted() }
                    }
                }

                Text {
                    width: 36; height: 30
                    text: settingsPanel.historyModel.maxVisibleRows
                    color: "#eceff4"; font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }

                Rectangle {
                    width: 30; height: 30; radius: 6
                    color: rPlusMouse.containsMouse ? "#3a3d42" : "#323438"
                    border.width: 1; border.color: "#44474e"
                    Text { anchors.centerIn: parent; text: "+"; color: "#eceff4"; font.pixelSize: 16 }
                    MouseArea {
                        id: rPlusMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: { settingsPanel.historyModel.maxVisibleRows += 1; settingsPanel.interacted() }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Horizontal position"
                color: "#cfd4dc"
                font.pixelSize: 15
                Layout.fillWidth: true
            }

            Row {
                spacing: 8

                Rectangle {
                    width: 30; height: 30; radius: 6
                    color: xLeftMouse.containsMouse ? "#3a3d42" : "#323438"
                    border.width: 1; border.color: "#44474e"
                    Text { anchors.centerIn: parent; text: "◀"; color: "#eceff4"; font.pixelSize: 12 }
                    MouseArea {
                        id: xLeftMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: { settingsPanel.moveRequested(-40, 0); settingsPanel.interacted() }
                    }
                }

                Text {
                    width: 64; height: 30
                    text: settingsPanel.historyModel.posX < 0 ? "Centered" : settingsPanel.historyModel.posX + "px"
                    color: "#eceff4"; font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }

                Rectangle {
                    width: 30; height: 30; radius: 6
                    color: xRightMouse.containsMouse ? "#3a3d42" : "#323438"
                    border.width: 1; border.color: "#44474e"
                    Text { anchors.centerIn: parent; text: "▶"; color: "#eceff4"; font.pixelSize: 12 }
                    MouseArea {
                        id: xRightMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: { settingsPanel.moveRequested(40, 0); settingsPanel.interacted() }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Vertical position"
                color: "#cfd4dc"
                font.pixelSize: 15
                Layout.fillWidth: true
            }

            Row {
                spacing: 8

                Rectangle {
                    width: 30; height: 30; radius: 6
                    color: yUpMouse.containsMouse ? "#3a3d42" : "#323438"
                    border.width: 1; border.color: "#44474e"
                    Text { anchors.centerIn: parent; text: "▲"; color: "#eceff4"; font.pixelSize: 12 }
                    MouseArea {
                        id: yUpMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: { settingsPanel.moveRequested(0, -40); settingsPanel.interacted() }
                    }
                }

                Text {
                    width: 64; height: 30
                    text: settingsPanel.historyModel.posY + "px"
                    color: "#eceff4"; font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }

                Rectangle {
                    width: 30; height: 30; radius: 6
                    color: yDownMouse.containsMouse ? "#3a3d42" : "#323438"
                    border.width: 1; border.color: "#44474e"
                    Text { anchors.centerIn: parent; text: "▼"; color: "#eceff4"; font.pixelSize: 12 }
                    MouseArea {
                        id: yDownMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: { settingsPanel.moveRequested(0, 40); settingsPanel.interacted() }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Drag panel to move · Drag right edge to resize width"
                color: "#7a808a"
                font.pixelSize: 12
                Layout.fillWidth: true
            }

            Rectangle {
                width: resetLabel.implicitWidth + 20
                height: 28
                radius: 6
                color: resetMouse.containsMouse ? "#3a3d42" : "#323438"
                border.width: 1
                border.color: "#44474e"

                Text {
                    id: resetLabel
                    anchors.centerIn: parent
                    text: "Center Position"
                    color: "#c5cad3"
                    font.pixelSize: 12
                }

                MouseArea {
                    id: resetMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        settingsPanel.resetPositionRequested()
                        settingsPanel.interacted()
                    }
                }
            }
        }
    }
}
