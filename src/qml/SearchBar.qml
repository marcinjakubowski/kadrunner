import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: searchBar

    property alias text: input.text
    property bool settingsOpen: false

    signal moveDown()
    signal moveUp()
    signal acceptCurrent()
    signal activateShortcut(int index)
    signal escapePressed()
    signal toggleSettings()

    function focusInput() {
        input.forceActiveFocus()
    }

    function clearAndFocus() {
        input.text = ""
        input.forceActiveFocus()
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 22
        anchors.rightMargin: 16
        spacing: 12

        TextField {
            id: input
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

            Keys.onPressed: function(event) {
                if (event.key === Qt.Key_Escape) {
                    searchBar.escapePressed()
                    event.accepted = true
                } else if (event.key === Qt.Key_Down || (event.key === Qt.Key_N && (event.modifiers & Qt.ControlModifier))) {
                    searchBar.moveDown()
                    event.accepted = true
                } else if (event.key === Qt.Key_Up || (event.key === Qt.Key_P && (event.modifiers & Qt.ControlModifier))) {
                    searchBar.moveUp()
                    event.accepted = true
                } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                    searchBar.acceptCurrent()
                    event.accepted = true
                } else if ((event.modifiers & Qt.AltModifier) && event.key >= Qt.Key_1 && event.key <= Qt.Key_9) {
                    searchBar.activateShortcut(event.key - Qt.Key_1)
                    event.accepted = true
                }
            }
        }

        Item {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignVCenter

            Kirigami.Icon {
                anchors.centerIn: parent
                width: 18
                height: 18
                source: "configure"
                color: searchBar.settingsOpen || gearMouse.containsMouse ? "#9aa0a6" : "#565a63"
                isMask: true
            }

            MouseArea {
                id: gearMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    searchBar.toggleSettings()
                    input.forceActiveFocus()
                }
            }
        }
    }
}
