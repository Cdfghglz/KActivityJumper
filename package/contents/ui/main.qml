
import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0

import org.kde.private.activityjumper 1.0

Item {
    id: main

    readonly property int minButtonSize: units.iconSizes.small
    readonly property int medButtonSize: units.iconSizes.medium
    readonly property int maxButtonSize: units.iconSizes.large
    
    Layout.minimumWidth: minButtonSize * gridItem.columns
    Layout.minimumHeight: minButtonSize * gridItem.rows

    Layout.maximumWidth: maxButtonSize * gridItem.columns
    Layout.maximumHeight: maxButtonSize * gridItem.rows
    
    readonly property int iconSize: {
        var value = 0
        if(plasmoid.formFactor != PlasmaCore.Types.Vertical){
            value = height / gridItem.rows
        }
        else {
            value = width / gridItem.columns
        }
        
        if(value < minButtonSize){
            value = minButtonSize
        }
        
        return value
    }

    Layout.preferredWidth: (iconSize * gridItem.columns)
    Layout.preferredHeight: (iconSize * gridItem.rows)
    
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    
    Plasmoid.icon: 'ajumper-back'

    ActivityJumper {
        id: activityJumper
        onIconSourceChanged: {
            updatePinIcon()
        }
    }

    function updatePinIcon() {
        iconTwo.source = activityJumper.iconSource
    }

    Grid {
        
        id: gridItem
        rows: 1
        columns: 2
        spacing: 0
        width: parent.width
        height: parent.height
        
        // sizes
            property int itemWidth: Math.floor(parent.width/columns)
            property int itemHeight: Math.floor(parent.height/rows)
            property int iconSize: Math.min(itemWidth, itemHeight)
        // sizes
        
        PlasmaCore.IconItem {
            property int state
            id: iconOne
            source: 'ajumper-back'
            visible: true
            
            width: gridItem.iconSize
            height: gridItem.iconSize
            
            MouseArea {
                id: mouseAreaOne
                hoverEnabled: true
                anchors.fill: parent

                onClicked: activityJumper.jumpBack()

                PlasmaCore.ToolTipArea {
                    anchors.fill: parent
                    mainText: "Jump back"
                    subText: "Jump to the previous activity and desktop."
               }
            }
        }
        
        PlasmaCore.IconItem {
            id: iconTwo
            source: 'ajumper-unpin'
            visible: true
            
            width: gridItem.iconSize
            height: gridItem.iconSize

            MouseArea {
                id: mouseAreaTwo
                hoverEnabled: true
                anchors.fill: parent

                onClicked: {
                    activityJumper.changePinState()
                    activityJumper.getPinState()
                }

                PlasmaCore.ToolTipArea {
                    anchors.fill: parent
//                    todo: change description based on state
                    mainText: "Pin the position"
                    subText: "Pin this desktop as return destination."
                }
            }
        }
    }
}
