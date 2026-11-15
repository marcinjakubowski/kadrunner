import QtQuick

Window {
    id: root

    readonly property int launcherWidth: 760
    readonly property int searchBarHeight: 66
    readonly property int rowHeight: 68
    readonly property int maxVisibleRows: 8
    readonly property int settingsHeight: 148
    property bool settingsOpen: false

    readonly property bool showingHistory: searchBar.text.length === 0
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
            searchBar.clearAndFocus()
            resultsList.currentIndex = 0
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: 10
        color: "#2b2c2f"
        border.width: 1
        border.color: "#3f4146"
        clip: true

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

            SettingsPanel {
                width: parent.width
                height: root.settingsHeight
                visible: root.settingsOpen
                historyModel: runnerBridge.historyModel
                onInteracted: searchBar.focusInput()
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
    }
}
