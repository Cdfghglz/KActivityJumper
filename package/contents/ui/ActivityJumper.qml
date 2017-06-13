/*
    Copyright (c) 2016 Carlos López Sánchez <musikolo{AT}hotmail[DOT]com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.2

Item {
    id: activityJumperObj

    property var activityJumperPlugin: null
    property bool activityJumperFailedToInitialize: false

    function getActivityJumperPlugin() {
        
        if (activityJumperPlugin !== null) {
            return activityJumperPlugin
        }
        
        if (!activityJumperFailedToInitialize) {
            console.log('Initializing activityJumper plugin...')
            try {
                activityJumperPlugin = Qt.createQmlObject('import org.kde.private.activityjumper 1.0 as WW; WW.ActivityJumper {}', activityJumper, 'ActivityJumper')
                console.log('ActivityJumper plugin initialized successfully!')
            }catch (e) {
                console.exception('ERROR: ActivityJumper plugin FAILED to initialize -->', e)
                activityJumperFailedToInitialize = true
            }
        }
        
        return activityJumperPlugin
    }
    
    function jumpBack() {
        var plugin = getActivityJumperPlugin()
        if (plugin) {
            var result = plugin.jumpBack()
            if(result !=0){
                console.error("plugin.jumpBack() returned error code=", result)
            }
        } else {
            console.exception('ERROR: Jump back - ActivityJumper plugin not available')
        }
    }

    function changePinState() {
        var plugin = getActivityJumperPlugin()
        if (plugin) {
            var result = plugin.changePinState()
            if(result == -1){
                console.error("plugin.changePinState() returned error code=", result)
            }
        } else {
            console.exception('ERROR: Change pin status - ActivityJumper plugin not available')
        }
    }

    function getPinState() {
        var plugin = getActivityJumperPlugin()
        if (plugin) {
            var result = plugin.getPinState()
            if(result != -1){
                console.error("plugin.getPinState() returned error code=", result)
            }
        } else {
            console.exception('ERROR: Get pin status - ActivityJumper plugin not available')
        }
    }

}
 
