godot-portaudio
===
module for godot

## Setup:
### via submodule
- open a terminal inside the godot source folder and run the following commands:  
`cd modules`  
`git submodule add https://github.com/sebastian-heinz/portaudio.git`
`git submodule update --init --recursive`  

### via download 
Download the zip (https://github.com/sebastian-heinz/portaudio/archive/master.zip) and extract it into the godot/modules-folder

## About
- `PortAudio` is a singleton class, its purpose is to provide wrapper methods for all PortAudio calls (PA_*) with a godot type friendly interface. For direct access from anywhere
- `PortAudioNode` is a godot node. Its primary purpose is to be extended via GDScript, so that it is possible to have the PortAudio-Audio-Callback available in GDScript.
- If working with c++ one can simply import the `#include "portaudio/include/portaudio.h"` PortAudio-Header and directly work with PortAudio's API.

## Status
Currenlty only tested ASIO driver under windows.

## TODO / ISSUES
- Add build switches to specify which host apis should be build. Currently if ASIO driver are not found the build will abort.
- The GDScript bindings need to be improoved, so might need to create more wrapper classes to expose functionality to GDScript.
- FInd out if there is a way to have callbacks working in GDScript, so it can also benefit from directly invoking the `PortAudio` singleton, instead of extending `PortAudioNode`
- [WIN] [WASAPI] clashes with godots WASAPI imports.
- [WIN] [WDMKS] also seems to clash with godot imports. (error LNK2005: KSDATAFORMAT_SUBTYPE_MIDI already defined in dxguid.lib(dxguid.obj))
- [LINUX] build pipeline untested.
- [OSX] build pipeline untested.

## Links
Portaudio:  
https://app.assembla.com/spaces/portaudio/git/source  
http://www.portaudio.com/  
  
Godot:  
https://github.com/godotengine/godot  
  
ASIO SDK 2.0:  
https://www.steinberg.net/en/company/developers.html  

Asio4All Driver:  
http://www.asio4all.org/
