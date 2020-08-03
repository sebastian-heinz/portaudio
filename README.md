portaudio
===
module for godot

## Setup:
### via submodule
- open a terminal inside the godot source folder and run the following commands:  
`cd modules`  
`git submodule add https://github.com/sebastian-heinz/portaudio.git`  
`git submodule update --init --recursive`  

### via download 
- Download the zip (https://github.com/sebastian-heinz/portaudio/archive/master.zip) and extract it into the godot/modules-folder.
  Since it is missing the portaudio source, it has to be manually added. Get it from http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz and paste it into the empty   portaudio folder. (./portaudio/portaudio/).  
  <img src="/doc/port_audio_example.png" width="400">

## Building
edit the [SCsub](./SCsub) file and comment / uncomment the desired host apis `use_host_api = [`.
When building godot it will check if the host api is supported for the platform (windows, linux or osx) and available inside the `use_host_api`-array. Only then the host api will be available.

## Driver
| Driver    | Windows       | x11           | osx 
| --------- | ------------- | ------------- |  ------------ |
| ASIO      | Working (*2)  | Not Tested    | Not Available |
| DSOUND    | Working       | Not Available | Not Available |
| WASAPI    | Working (*1)  | Not Available | Not Available |
| WDM/KS    | Working       | Not Available | Not Available |
| WMME      | Error         | Not Available | Not Available |
| JACK      | Not Available | Not Tested    | Not Available |
| ALSA      | Not Available | Not Tested    | Not Available |
| ASIHPI    | Not Available | Not Tested    | Not Available |
| COREAUDIO | Not Available | Not Available | Not Tested    |
| OSS       | Not Available | Not Tested    | Not Available |

*1) if used godots wasapi driver will be disabled  
*2) requires that the enduser has asio drivers installed (ex. ASIO4ALL)  

## Godot
- `PortAudio` is a singleton class, its purpose is to provide wrapper methods for all PortAudio calls (PA_*) with a godot type friendly interface. For direct access from anywhere
- `PortAudioNode` is a godot node. Its primary purpose is to be extended via GDScript, so that it is possible to have the PortAudio-Audio-Callback available in GDScript.
- If working with c++ one can simply import the `#include "portaudio/include/portaudio.h"` PortAudio-Header and directly work with PortAudio's API.

## TODO
- The GDScript bindings need to be improoved, need to create more wrapper classes to expose functionality to GDScript.
- [WIN] [WDMKS]-driver clashes with godot imports. (error LNK2005: KSDATAFORMAT_SUBTYPE_MIDI already defined in dxguid.lib(dxguid.obj))
- [LINUX] build pipeline untested.
- [OSX] build pipeline untested.

## Dependencies
- PortAudio: `pa_stable_v190600_20161030`
- ASIO SDK 2.0+ (optional)

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
