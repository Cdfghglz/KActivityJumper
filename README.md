# KActivityJumper
A simple KDE application for fast navigation to favorite desktops across activities... and BACK!

## Use case
You know the situation when you **focus on a task** in an Activity populated with task specific content. And then **suddenly this email arrives** that you just have to reply to, this song that you just have to skip... So you want to **jump away** for a second and be right **BACK where you left**. Obviously KDE has a great hotkey support, but you do not want to compromise the flexibility of KDE Activities by setting a shortcut for every scenario.  

*You want to interrupt whatever you do by short, known tasks, and be brought back without cognitive effort.*

## Instructions

### Requirements
- CMake 3.5+
- KDE 5+
- Qt5+

### Configure
The *jumperDestinationConfig.txt* configuration file has the syntax:  
**dbus_method_arg &nbsp;&nbsp; activity_name &nbsp;&nbsp; desktop_number**  
e.g.  
`u KOM 1`  
`i KOM 2`  
`...`

A corresponding *myJumperShortcutExport.khotkeys* KDE hotkeys configuration file is included. This should be imported with right klick under *System Settings -> Shortcuts -> Custom Shortcuts*.  
Make sure to reflect the changes made in *jumperDestinationConfig.txt* on the imported *ActivityJumper* group Custom Shortcuts!  
**TODO**: work on automating this is in progress

### Install, run
To install, use `cmake` and `make` to build and run the executable with `./ActivityJumper`

### Use
The sample configuration files let you jump to any of 6 desktops of activity "KOM" with *'Super + u'*, *'Super + i'*... and then, in reverse order of the jumps, back to the initial position with *'Super + space'*.
