godot-portaudio
===
module for godot

## Setup:
### via submodule
- open a terminal inside the godot source folder and run the following commands:  
`cd modules`  
`git clone https://github.com/sebastian-heinz/portaudio.git`  
`git submodule update --init --recursive`  

### via download 
Download the zip (https://github.com/sebastian-heinz/portaudio/archive/master.zip) and extract it into the godot/modules-folder

## Status
Currenlty only tested ASIO driver under windows.

## TODO / ISSUES
- [WIN] [WASAPI] clashes with godots WASAPI imports
- [WIN] [WDMKS] also seems to clash with godot imports (error LNK2005: KSDATAFORMAT_SUBTYPE_MIDI already defined in dxguid.lib(dxguid.obj))
- [LINUX] build pipeline untested
- [OSX] build pipeline untested

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
