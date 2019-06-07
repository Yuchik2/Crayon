import QtQuick 2.0

Rectangle{
    width: 200;
    height: 200;
    border.color: "black";
    border.width: 2;
    Image {
        anchors.centerIn: parent;
        width: Math.min(parent.width, parent.height) / 2;
        height: width;
        source: "qrc:/CrayonQmlPluginResources/null_item.svg";
    }
}

