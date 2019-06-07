import QtQuick 2.0
import CrayonQml 1.0

CrNode {
    nodeName: "Test Qml";
    nodeFlags: CrNode.In | CrNode.EditableName | CrNode.EditableValue;
    value: 5
    property var tst: function () {return 15;}

    CrNode{
        nodeName: "Child";
        nodeFlags: CrNode.In | CrNode.EditableName | CrNode.EditableValue;
        value: "25x25";
    }
    CrNode{
        id: editor
        nodeName: "Editor";
        nodeFlags: CrNode.In | CrNode.EditableName | CrNode.EditableValue;
        value: 10;
    }
    CrNode{
        nodeName: "Child";
        nodeFlags: CrNode.In | CrNode.EditableName | CrNode.EditableValue;
        value: editor.value * 2;
    }
    property Component view: Item{
        property string nn: editor.nodeName;
        implicitHeight: 200;
        implicitWidth: 200;
        Text {
            id: text
            anchors.fill: parent
            text: nn
        }
    }
}
