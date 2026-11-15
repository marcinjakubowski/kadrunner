import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Rectangle {
    id: rowDelegate

    required property int index
    required property var decoration
    required property string display
    required property string subtext
    required property string category

    signal hovered(int row)
    signal activated(int row)

    color: ListView.isCurrentItem ? "#3d3f44" : (mouseArea.containsMouse ? "#333539" : "transparent")

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
        onEntered: rowDelegate.hovered(rowDelegate.index)
        onClicked: rowDelegate.activated(rowDelegate.index)
    }
}
