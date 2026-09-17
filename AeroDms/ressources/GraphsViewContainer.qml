import QtQuick
import QtQuick.Layouts
import QtGraphs

Item {
    id: root
    property string chartTitle: ""
    property bool legendVisible: true
    property int titlePointSize: 20
    property string legendPosition: "right"
    property var legendEntries: []
    property var axisX: null
    property var axisY: null

    function rebuildLegend() {
        legendModel.clear()

        for (let i = 0; i < root.legendEntries.length; ++i)
            legendModel.append(root.legendEntries[i])
    }

    ListModel {
        id: legendModel
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Text {
            Layout.fillWidth: true
            visible: root.chartTitle.length > 0
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            font.family: "Arial"
            font.pointSize: root.titlePointSize
            font.bold: true
            text: root.chartTitle
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            GraphsView {
                id: graphView
                objectName: "graphView"
                axisX: root.axisX
                axisY: root.axisY
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                anchors.rightMargin: root.legendVisible && root.legendPosition === "right" ? legendPanel.width + 8 : 0
                anchors.bottomMargin: root.legendVisible && root.legendPosition === "bottom" ? legendFlow.implicitHeight + 8 : 0
            }

            Flickable {
                id: legendPanel
                anchors {
                    top: parent.top
                    right: parent.right
                    bottom: parent.bottom
                }
                width: root.legendVisible && root.legendPosition === "right" ? 280 : 0
                visible: root.legendVisible && root.legendPosition === "right"
                contentWidth: width
                contentHeight: legendColumn.implicitHeight
                clip: true

                Column {
                    id: legendColumn
                    width: parent.width
                    spacing: 8

                    Repeater {
                        model: legendModel

                        Row {
                            spacing: 8
                            width: parent.width

                            Rectangle {
                                width: 14
                                height: 14
                                radius: 2
                                color: legendColor
                                border.color: legendBorderColor
                                border.width: 1
                            }

                            Text {
                                width: parent.width - 22
                                wrapMode: Text.WordWrap
                                font.family: "Arial"
                                text: legendLabel
                            }
                        }
                    }
                }
            }

            Flow {
                id: legendFlow
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                visible: root.legendVisible && root.legendPosition === "bottom"
                spacing: 16

                Repeater {
                    model: legendModel

                    Row {
                        spacing: 8

                        Rectangle {
                            width: 14
                            height: 14
                            radius: 2
                            color: legendColor
                            border.color: legendBorderColor
                            border.width: 1
                        }

                        Text {
                            font.family: "Arial"
                            text: legendLabel
                        }
                    }
                }
            }
        }
    }
}
