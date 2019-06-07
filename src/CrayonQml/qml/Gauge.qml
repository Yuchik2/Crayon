import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4
import Crayon 1.0



///123

QmlComponentNode{
    Module.id:"crayonqml/gauge"
    Module.version: "1.0.0"
    Module.path: "Indicators/Gauge"
    Module.description: "Gauge"
    Module.image: "gauge.png"

    name: "Gauge";
    flags: Node.EditableName | Node.ActiveEditor | Node.Out;

    RealNode{
        id: inNode
        name: qsTr("Input");
        flags: Node.In | Node.ActiveEditor;
        realValue:  0;
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

    Node {
        id:styleNode
        name: qsTr("Style");

        RealLimitedNode{
            id: minAngleNode
            name: qsTr("Min angle");
            flags: Node.ActiveEditor;
            min: -180
            max: maxAngleNode.realValue
             Component.onCompleted: realValue = -145;
        }

        RealLimitedNode{
            id: maxAngleNode
            name: qsTr("Max angle");
            flags: Node.ActiveEditor;
            min: minAngleNode.realValue
            max: 180
            Component.onCompleted: realValue = 145;
        }

        RealNode{
            id: labelStepNode
            name: qsTr("Label step");
            flags: Node.ActiveEditor;
            realValue:  10;
        }

        RealNode{
            id: tickmarkStepNode
            name: qsTr("Tickmark step");
            flags: Node.ActiveEditor;
            realValue:  10;
        }

        IntegerLimitedNode {
            id: minorTickmarkCountNode
            name: qsTr("Minor tickmark count");
            flags: Node.ActiveEditor;
            intValue:  4;
        }

        QmlComponentNode{
            id: backgroundNode
            name: qsTr("Background");
            flags: Node.In | Node.ActiveEditor;
            component: Rectangle {
                color: "red";
                opacity: 0.2;
            }
        }

        QmlComponentNode{
            id: foregroundNode
            name: qsTr("Foreground");
            flags: Node.In | Node.ActiveEditor;
            component: Item {
                Image {
                    source: "images/knob.png"
                    anchors.centerIn: parent
                    scale: {
                        var idealHeight = Math.min(parent.width, parent.height) * 0.1;
                        var originalImageHeight = sourceSize.height;
                        idealHeight / originalImageHeight;
                    }
                }
            }
        }

    }

    BooleanNode{
        id: animationNode
        name: qsTr("Animation");
        flags: Node.ActiveEditor;
        value: true;

        RealNode{
            id: dampingNode
            name: qsTr("Damping");
            flags: Node.ActiveEditor;
            realValue:  0.1;
        }

        RealNode{
            id: springNode
            name: qsTr("Spring");
            flags: Node.ActiveEditor;
            realValue:  2;
        }

        RealNode{
            id: velocityNode
            name: qsTr("Velocity");
            flags: Node.ActiveEditor;
            realValue:  0;
        }

        RealNode{
            id: massNode
            name: qsTr("Mass");
            flags: Node.ActiveEditor;
            realValue:  0;
        }
    }

    component:
        CircularGauge {
        id: gauge
        width: 200
        height: 200

        smooth: true
        antialiasing: true;

        minimumValue: minNode.realValue;
        maximumValue: maxNode.realValue;
        value: inNode.realValue;
        style : CircularGaugeStyle {
            minimumValueAngle: minAngleNode.realValue;
            maximumValueAngle: maxAngleNode.realValue;
            labelStepSize: labelStepNode.realValue;
            tickmarkStepSize: tickmarkStepNode.realValue;
            minorTickmarkCount: minorTickmarkCountNode.intValue

            background: backgroundNode.component;
            foreground: foregroundNode.component;
        }

        Behavior on value {
            enabled: animationNode.value;
            SpringAnimation {
                id: recanim
                damping: dampingNode.realValue
                spring: springNode.realValue
                velocity: velocityNode.realValue
                mass: massNode.realValue
                epsilon:0.25
            }
        }
    }
}



