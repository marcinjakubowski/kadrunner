import QtQuick

Window {
    id: root

    readonly property int launcherWidth: runnerBridge.historyModel.launcherWidth
    readonly property int searchBarHeight: 66
    readonly property int rowHeight: 68
    readonly property int maxVisibleRows: runnerBridge.historyModel.maxVisibleRows
    readonly property int settingsHeight: 316
    property bool settingsOpen: false
    property bool isDragging: false
    property bool snappedCenterX: false
    property bool snappedCenterY: false

    readonly property bool showingHistory: searchBar.text.length === 0
    readonly property int resultCount: settingsOpen ? 0 : resultsList.count
    readonly property int contentHeight: settingsOpen
        ? settingsHeight + 1
        : (resultCount > 0 ? Math.min(resultCount, maxVisibleRows) * rowHeight + 1 : 0)
    readonly property int panelHeight: searchBarHeight + contentHeight

    width: settingsOpen ? runnerBridge.activeScreenWidth() : launcherWidth
    height: settingsOpen ? runnerBridge.activeScreenHeight() : panelHeight
    visible: false
    color: "transparent"
    flags: Qt.FramelessWindowHint

    onSettingsOpenChanged: runnerBridge.setDesignMode(settingsOpen)

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
            searchBar.clearAndFocus()
            resultsList.currentIndex = 0
        }
    }

    // Click outside the launcher while settings are open to exit settings mode
    MouseArea {
        anchors.fill: parent
        visible: root.settingsOpen
        onClicked: {
            root.settingsOpen = false
            searchBar.focusInput()
        }
    }

    // Center snap guide lines (visible while dragging in design mode)
    Rectangle {
        visible: root.settingsOpen && root.isDragging && root.snappedCenterX
        x: Math.round(root.width / 2)
        y: 0
        width: 1
        height: root.height
        color: "#6194e8"
        opacity: 0.85
    }

    Rectangle {
        visible: root.settingsOpen && root.isDragging && root.snappedCenterY
        x: 0
        y: Math.round(root.height / 2)
        width: root.width
        height: 1
        color: "#6194e8"
        opacity: 0.85
    }

    Rectangle {
        id: frame
        x: root.settingsOpen ? Math.max(0, runnerBridge.historyModel.posX) : 0
        y: root.settingsOpen ? runnerBridge.historyModel.posY : 0
        width: root.launcherWidth
        height: root.panelHeight
        radius: 10
        color: "#2b2c2f"
        border.width: 1
        border.color: "#3f4146"
        clip: true

        // Prevent clicks inside frame from hitting the outside-click MouseArea
        MouseArea {
            anchors.fill: parent
            enabled: root.settingsOpen
            acceptedButtons: Qt.AllButtons
        }

        Column {
            anchors.fill: parent
            spacing: 0

            SearchBar {
                id: searchBar
                width: parent.width
                height: root.searchBarHeight
                settingsOpen: root.settingsOpen

                onTextChanged: {
                    if (root.settingsOpen && text.length > 0) {
                        root.settingsOpen = false
                    }
                    runnerBridge.model.queryString = text
                    resultsList.currentIndex = 0
                }

                onEscapePressed: {
                    if (root.settingsOpen) {
                        root.settingsOpen = false
                    } else {
                        runnerBridge.hide()
                    }
                }

                onMoveDown: {
                    if (!root.settingsOpen && resultsList.count > 0) {
                        resultsList.currentIndex = (resultsList.currentIndex + 1) % resultsList.count
                    }
                }

                onMoveUp: {
                    if (!root.settingsOpen && resultsList.count > 0) {
                        resultsList.currentIndex = (resultsList.currentIndex - 1 + resultsList.count) % resultsList.count
                    }
                }

                onAcceptCurrent: {
                    if (!root.settingsOpen && resultsList.count > 0) {
                        root.activateRow(Math.max(0, resultsList.currentIndex))
                    }
                }

                onActivateShortcut: function(idx) {
                    if (!root.settingsOpen && idx < resultsList.count) {
                        root.activateRow(idx)
                    }
                }

                onToggleSettings: root.settingsOpen = !root.settingsOpen
            }

            Rectangle {
                width: parent.width
                height: 1
                color: "#3a3c40"
                visible: root.contentHeight > 0
            }

            Item {
                width: parent.width
                height: root.settingsHeight
                visible: root.settingsOpen

                // Smooth drag anywhere on the settings panel background
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.SizeAllCursor
                    property real pressX: 0
                    property real pressY: 0
                    property int startPosX: 0
                    property int startPosY: 0
                    onPressed: function(mouse) {
                        const globalPt = mapToItem(null, mouse.x, mouse.y)
                        pressX = globalPt.x
                        pressY = globalPt.y
                        startPosX = frame.x
                        startPosY = frame.y
                        root.isDragging = true
                        root.snappedCenterX = false
                        root.snappedCenterY = false
                    }
                    onPositionChanged: function(mouse) {
                        if (!pressed) {
                            return
                        }
                        const globalPt = mapToItem(null, mouse.x, mouse.y)
                        const dx = Math.round(globalPt.x - pressX)
                        const dy = Math.round(globalPt.y - pressY)

                        const maxX = Math.max(0, root.width - frame.width)
                        const maxY = Math.max(0, root.height - frame.height)
                        const centerX = Math.round(maxX / 2)
                        const centerY = Math.round(maxY / 2)
                        const snapThreshold = 12

                        let nextX = Math.max(0, Math.min(maxX, startPosX + dx))
                        let nextY = Math.max(0, Math.min(maxY, startPosY + dy))

                        if (Math.abs(nextX - centerX) <= snapThreshold) {
                            nextX = centerX
                            root.snappedCenterX = true
                        } else {
                            root.snappedCenterX = false
                        }

                        if (Math.abs(nextY - centerY) <= snapThreshold) {
                            nextY = centerY
                            root.snappedCenterY = true
                        } else {
                            root.snappedCenterY = false
                        }

                        runnerBridge.historyModel.posX = nextX
                        runnerBridge.historyModel.posY = nextY
                    }
                    onReleased: {
                        root.isDragging = false
                        root.snappedCenterX = false
                        root.snappedCenterY = false
                        searchBar.focusInput()
                    }
                }

                SettingsPanel {
                    anchors.fill: parent
                    historyModel: runnerBridge.historyModel
                    onInteracted: searchBar.focusInput()
                    onMoveRequested: function(dx, dy) { runnerBridge.moveBy(dx, dy) }
                    onResetPositionRequested: {
                        runnerBridge.resetPosition()
                        const centeredX = Math.max(0, Math.round((root.width - frame.width) / 2))
                        runnerBridge.historyModel.posX = centeredX
                    }
                }
            }

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

                delegate: ResultDelegate {
                    width: resultsList.width
                    height: root.rowHeight
                    onHovered: function(row) { resultsList.currentIndex = row }
                    onActivated: function(row) { root.activateRow(row) }
                }
            }
        }

        // Right-edge resize handle when settings are open
        Rectangle {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: 8
            visible: root.settingsOpen
            color: resizeMouse.containsMouse || resizeMouse.pressed ? "#4b6eaf" : "transparent"

            MouseArea {
                id: resizeMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.SizeHorCursor
                property real pressX: 0
                property int startWidth: 0
                onPressed: function(mouse) {
                    const globalPt = mapToItem(null, mouse.x, mouse.y)
                    pressX = globalPt.x
                    startWidth = runnerBridge.historyModel.launcherWidth
                }
                onPositionChanged: function(mouse) {
                    if (pressed) {
                        const globalPt = mapToItem(null, mouse.x, mouse.y)
                        runnerBridge.historyModel.launcherWidth = startWidth + Math.round(globalPt.x - pressX)
                    }
                }
                onReleased: searchBar.focusInput()
            }
        }
    }
}
