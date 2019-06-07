import QtQuick 2.0
import QtQuick.Window 2.10

Canvas{

    property color backgroundColor: "white"
    property color blockColor: "light grey"
    property int sizeBlock: 20
    onPaint: {
        var ctx = getContext("2d")
        ctx.fillStyle = backgroundColor;
        ctx.fillRect(0, 0, width, height)

        ctx.fillStyle = blockColor;
        for (var i = 0; i < width; i += sizeBlock * 2) {
            for (var j = 0; j < height; j += sizeBlock * 2) {
                ctx.fillRect(i, j, sizeBlock, sizeBlock)
            }
        }
        for (var i = sizeBlock; i < width; i += sizeBlock * 2) {
            for (var j = sizeBlock; j < height; j += sizeBlock * 2) {
                ctx.fillRect(i, j, sizeBlock, sizeBlock)
            }
        }
    }
    width: childrenRect.x + childrenRect.width;
    height: childrenRect.y + childrenRect.height;
}
