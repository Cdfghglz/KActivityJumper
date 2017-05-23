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
1. Edit ***jumperDestinationConfig.config***  
File has the syntax `dbus_method_arg activity_name desktop_number\n`, e.g.:  
`u KOM 1`  
`i KOM 2`  
`...`  
Replace the *KOM* with desired and existing activity name, change the desktop numbers as needed.  

2. Import ***myJumperShortcutExport.khotkeys***  
A corresponding *myJumperShortcutExport.khotkeys* KDE hotkeys configuration file is included. This should be imported with right klick under *System Settings -> Shortcuts -> Custom Shortcuts*.  
If needed, customize the shortcuts.

**NOTE**: Make sure to manually reflect any changes of `dbus_method_arg` in *jumperDestinationConfig.txt* on the imported *ActivityJumper* group of Custom Shortcuts! Work on automating this step is in progress.

### Install
`mkdir build`  
`cd build/`  
`cmake ..`  
`sudo make install`

### Use

Use the imported shortcuts to jump to desired activities desktops and back.

With the default configuration:  

Use *'Super + u'*, *'Super + i'*... to jump to any of the 6 desktops of activity "KOM" and then, in reverse order, back to the initial position with *'Super + space'*.

