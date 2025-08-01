# ActivityJumper
A simple KDE application for fast navigation to favorite desktops across activities... and back.

## Use case
If you enjoy the flexibility of Activities in KDE along with virtual desktops, it may become ineffective to cover all possible workflow scenarios with keyboard shortcuts. Normally there are **several main tasks** you work on, and a **handful of distractions** - an email that you just have to reply to, a song that you just have to skip... So you want to **jump away** for a second and be right **back where you left**. ActivityJumper helps you do just this. You can pin your current position, jump around to favourite ones and return with no cognitive effort.

## Contents

- [ActivityJumper](#activityjumper)
  - [Use case](#use-case)
  - [Contents](#contents)
  - [Instructions](#instructions)
    - [Requirements overview](#requirements-overview)
      - [Kubuntu 22.04](#kubuntu-2204)
      - [Kubuntu 20.04](#kubuntu-2004)
      - [Kubuntu 18.04](#kubuntu-1804)
      - [Kubuntu 16.04](#kubuntu-1604)
      - [KDE Neon](#kde-neon)
    - [Configure](#configure)
    - [Install](#install)
    - [Use](#use)
      - [GUI](#gui)
      - [Keys](#keys)
  - [TODO:](#todo)

## Instructions

### Requirements

Overview:

- CMake 3.5+
- KDE 5+
- Qt5+

#### Kubuntu 22.04
```
sudo apt update &&
sudo apt install \
  plasma-framework \
  plasma-workspace-dev \
  libkf5declarative-dev \
  libkf5globalaccel-dev \
  libkf5configwidgets-dev \
  libkf5xmlgui-dev \
  libkf5windowsystem-dev \
  libkf5doctools-dev \
  cmake \
  extra-cmake-modules \
  libkf5activities-dev
```

#### Kubuntu 20.04

`sudo apt install plasma-framework plasma-workspace-dev libkf5declarative-dev libkf5globalaccel-dev libkf5configwidgets-dev libkf5xmlgui-dev libkf5windowsystem-dev kdoctools-dev cmake extra-cmake-modules libkf5activities-dev`

#### Kubuntu 18.04

`sudo apt install plasma-framework plasma-workspace-dev libkf5declarative-dev libkf5globalaccel-dev libkf5configwidgets-dev libkf5xmlgui-dev libkf5windowsystem-dev kdoctools-dev cmake extra-cmake-modules libkf5activities-dev`

#### Kubuntu 16.04
`sudo apt install plasma-framework-dev plasma-workspace-dev libkf5declarative-dev libkf5globalaccel-dev libkf5configwidgets-dev libkf5xmlgui-dev libkf5windowsystem-dev kdoctools-dev cmake extra-cmake-modules kdelibs5-dev libkf5activities-dev`

*NOTE: please replace the `kf5-config` with `kde4-config` in the *install* file when installing on pre-KF5 system.

#### KDE Neon
THIS IS LIKELY NO LONGER WORKING:

`sudo apt install cmake extra-cmake-modules qtdeclarative5-dev libqt5x11extras5-dev libkf5iconthemes-dev libkf5plasma-dev libkf5windowsystem-dev libkf5declarative-dev libkf5xmlgui-dev libkf5activities-dev build-essential libxcb-util-dev libkf5wayland-dev git gettext libkf5archive-dev libkf5notifications-dev libxcb-util0-dev`

### Configure
1. Edit ***jumperDestinationConfig.config***  
File has the syntax `dbus_method_arg activity_name desktop_number\n`, e.g.:  
`u KOM 1`  
`i KOM 2`  
`...`  
Replace the *KOM* with desired and existing activity name, change the desktop numbers as needed. If the activity name inlcudes space character, wrap it with double quotes, e.g. *"Activity KOM"*.


2. Import ***myJumperShortcutExport.khotkeys***  
A corresponding *myJumperShortcutExport.khotkeys* KDE hotkeys configuration file is included. This should be imported with right klick under *System Settings -> Shortcuts -> Custom Shortcuts*.  
If needed, customize the shortcuts.

**NOTE**: Make sure to manually reflect any changes of `dbus_method_arg` in *jumperDestinationConfig.txt* on the imported *ActivityJumper* group of Custom Shortcuts! Work on automating this step is in progress.

### Install

`chmod +x install`

Run `./install`, the script will prompt a sudo password at the end.

Similarly you can uninstall with `sudo ./uninstall`

### Use

#### GUI

Add the ActivityJumper Plasma widget to where you like it most.

Click the *Pin* icon ![](package/icons/breeze/apps/16/ajumper-pin.svg) to change the pin-state of the current position, then use the *Back* ![](package/icons/breeze/apps/16/ajumper-back.svg) icon to jump to the pinned positions. When there are multiple pinned positions, the *Back* icon will circle through them endlessly.

#### Keys
Use the imported shortcuts to jump to desired activities desktops and back.

With the default configuration:
Use *'Super + u'*, *'Super + i'*... to jump to any of the 6 desktops of activity "KOM" and then, in reverse order, back to the pinned positions with *'Super + space'*.

## TODO:
 - GUI for shortcut editing
 - add icon layout flexibility
 - handlers for Activity events like Stop/Add/Delete...
 - localisation
 - synchronisation between multiple instances 
