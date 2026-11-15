import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Window {
    id: root

    readonly property int launcherWidth: 760
    readonly property int searchBarHeight: 66
    readonly property int rowHeight: 68
    readonly property int maxVisibleRows: 8
    readonly property int settingsHeight: 148
    property bool settingsOpen: false

    readonly property bool showingHistory: searchInput.text.length === 0
    readonly property int resultCount: settingsOpen ? 0 : resultsList.count
    readonly property int contentHeight: settingsOpen
        ? settingsHeight + 1
        : (resultCount > 0 ? Math.min(resultCount, maxVisibleRows) * rowHeight + 1 : 0)

    width: launcherWidth
    height: searchBarHeight + contentHeight
    visible: false
    color: "transparent"
    flags: Qt.FramelessWindowHint

    function activateRow(idx) {
        if (showingHistory) {
            runnerBridge.runHistoryRow(idx)
        } else {
            runnerBridge.runRow(idx)
        }
    }

    onActiveChanged: {
        if (!active && visible) {
            runnerBridge.hide()
        }
    }

    Connections {
        target: runnerBridge
        function onShown() {
            root.settingsOpen = false
            searchInput.text = ""
            resultsList.currentIndex = 0
            searchInput.forceActiveFocus()
        }
    }

    Rectangle {
        id: frame
        anchors.fill: parent
        radius: 10
        color: "#2b2c2f"
        border.width: 1
        border.color: "#3f4146"
        clip: true

        Column {
            anchors.fill: parent
            spacing: 0

            // Search input bar
            Item {
                width: parent.width
                height: root.searchBarHeight

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 22
                    anchors.rightMargin: 16
                    spacing: 12

                    TextField {
                        id: searchInput
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        background: null
                        color: "#eceff4"
                        placeholderText: "Type to search..."
                        placeholderTextColor: "#787e88"
                        font.pixelSize: 24
                        verticalAlignment: TextInput.AlignVCenter
                        selectByMouse: true
                        focus: true

                        onTextChanged: {
                            if (root.settingsOpen && text.length > 0) {
                                root.settingsOpen = false
                            }
                            runnerBridge.model.queryString = text
                            resultsList.currentIndex = 0
                        }

                        Keys.onPressed: function(event) {
                            if (event.key === Qt.Key_Escape) {
                                if (root.settingsOpen) {
                                    root.settingsOpen = false
                                } else {
                                    runnerBridge.hide()
                                }
                                event.accepted = true
                            } else if (event.key === Qt.Key_Down || (event.key === Qt.Key_N && (event.modifiers & Qt.ControlModifier))) {
                                if (!root.settingsOpen && resultsList.count > 0) {
                                    resultsList.currentIndex = (resultsList.currentIndex + 1) % resultsList.count
                                }
                                event.accepted = true
                            } else if (event.key === Qt.Key_Up || (event.key === Qt.Key_P && (event.modifiers & Qt.ControlModifier))) {
                                if (!root.settingsOpen && resultsList.count > 0) {
                                    resultsList.currentIndex = (resultsList.currentIndex - 1 + resultsList.count) % resultsList.count
                                }
                                event.accepted = true
                            } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                                if (!root.settingsOpen && resultsList.count > 0) {
                                    root.activateRow(Math.max(0, resultsList.currentIndex))
                                }
                                event.accepted = true
                            } else if ((event.modifiers & Qt.AltModifier) && event.key >= Qt.Key_1 && event.key <= Qt.Key_9) {
                                const idx = event.key - Qt.Key_1
                                if (!root.settingsOpen && idx < resultsList.count) {
                                    root.activateRow(idx)
                                    event.accepted = true
                                }
                            }
                        }
                    }

                    // Subtle gear icon matching Ulauncher
                    Item {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        Layout.alignment: Qt.AlignVCenter

                        Kirigami.Icon {
                            anchors.centerIn: parent
                            width: 18
                            height: 18
                            source: "configure"
                            color: root.settingsOpen || gearMouse.containsMouse ? "#9aa0a6" : "#565a63"
                            isMask: true
                        }

                        MouseArea {
                            id: gearMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.settingsOpen = !root.settingsOpen
                                searchInput.forceActiveFocus()
                            }
                        }
                    }
                }
            }

            // Divider
            Rectangle {
                width: parent.width
                height: 1
                color: "#3a3c40"
                visible: root.contentHeight > 0
            }

            // Inline settings panel
            Item {
                width: parent.width
                height: root.settingsHeight
                visible: root.settingsOpen

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
                                    readonly property bool selected: runnerBridge.historyModel.mode === modelData.value
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
                                            runnerBridge.historyModel.mode = modelData.value
                                            searchInput.forceActiveFocus()
                                        }
                                    }
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        opacity: runnerBridge.historyModel.mode === "off" ? 0.45 : 1.0

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
                                    enabled: runnerBridge.historyModel.mode !== "off"
                                    onClicked: {
                                        runnerBridge.historyModel.maxItems = runnerBridge.historyModel.maxItems - 1
                                        searchInput.forceActiveFocus()
                                    }
                                }
                            }

                            Text {
                                width: 28
                                height: 32
                                text: runnerBridge.historyModel.maxItems
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
                                    enabled: runnerBridge.historyModel.mode !== "off"
                                    onClicked: {
                                        runnerBridge.historyModel.maxItems = runnerBridge.historyModel.maxItems + 1
                                        searchInput.forceActiveFocus()
                                    }
                                }
                            }
                        }
                    }

                    Text {
                        text: "Hotkey: System Settings → Keyboard → Shortcuts → KadRunner (default Meta+/)"
                        color: "#7a808a"
                        font.pixelSize: 12
                    }
                }
            }

            // Results list
            ListView {
                id: resultsList
                width: parent.width
                height: Math.min(root.resultCount, root.maxVisibleRows) * root.rowHeight
                visible: !root.settingsOpen && root.resultCount > 0
                clip: true
                model: root.showingHistory ? runnerBridge.historyModel : runnerBridge.model
                currentIndex: 0
                boundsBehavior: Flickable.StopAtBounds
                highlightMoveDuration: 0

                onCountChanged: {
                    if (currentIndex < 0 || currentIndex >= count) {
                        currentIndex = 0
                    }
                }

                delegate: Rectangle {
                    id: rowDelegate
                    width: resultsList.width
                    height: root.rowHeight
                    color: ListView.isCurrentItem ? "#3d3f44" : (mouseArea.containsMouse ? "#333539" : "transparent")

                    required property int index
                    required property var decoration
                    required property string display
                    required property string subtext
                    required property string category

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 20
                        anchors.rightMargin: 20
                        anchors.topMargin: 10
                        anchors.bottomMargin: 10
                        spacing: 16

                        Kirigami.Icon {
                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 48
                            Layout.alignment: Qt.AlignVCenter
                            source: rowDelegate.decoration
                            fallback: "application-x-executable"
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 2

                            Text {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignTop
                                text: rowDelegate.display
                                color: "#eceff4"
                                font.pixelSize: 18
                                font.weight: Font.Normal
                                elide: Text.ElideRight
                            }

                            Item { Layout.fillHeight: true }

                            Text {
                                readonly property string typeText: rowDelegate.subtext.length > 0
                                    ? (rowDelegate.category.length > 0 ? rowDelegate.category + " · " + rowDelegate.subtext : rowDelegate.subtext)
                                    : rowDelegate.category
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignBottom
                                visible: typeText.length > 0
                                text: typeText
                                color: "#8b919a"
                                font.pixelSize: 12
                                elide: Text.ElideRight
                            }
                        }

                        Text {
                            visible: rowDelegate.index < 9
                            text: "Alt+" + (rowDelegate.index + 1)
                            color: ListView.isCurrentItem ? "#b4bac2" : "#777c85"
                            font.pixelSize: 13
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: resultsList.currentIndex = rowDelegate.index
                        onClicked: root.activateRow(rowDelegate.index)
                    }
                }
            }
        }
    }
}
