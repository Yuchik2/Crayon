import QtQuick 2.9
import QtQuick.Controls 2.2 as Controls
import Crayon 1.0



QmlComponentNode{

    Module.id:"crayonqml/slider"
    Module.version: "1.0.0"
    Module.path: "Controls/Slider"
    Module.description: "Slider"
    Module.image: "slider.png"

    name: "Slider";
    flags: Node.EditableName | Node.ActiveEditor | Node.Out;

    RealNode{
        id: outNode
        name: qsTr("Output");
        flags: Node.Out;
        realValue: 0
    }

    RealNode{
        id: minNode
        name: qsTr("Minimum");
        flags: Node.ActiveEditor;
        realValue:  0;
    }

    RealNode{
        id: maxNode
        name: qsTr("Maximum");
        flags: Node.ActiveEditor;
        realValue:  100;
    }

    component: Controls.Slider{
        width: 300;
        height: 100;

        from: minNode.realValue
        to: maxNode.realValue
        value: 0
        onValueChanged: {
            outNode.realValue = value;
        }
    }
}



