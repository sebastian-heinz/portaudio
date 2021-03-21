portaudio
===
module for godot

## Table of contents
- [Why?](#why)
- [Setup](#setup)
  - [via submodule](#via-submodule)
  - [via download](#via-download)
- [Building](#building)
- [Driver](#driver)
- [Godot Integration](#godot-integration)
  - [Nodes](#nodes)
  - [Example GDScripts](#example-gdscripts)
  - [C++](#c)
- [Gotchas and Tips](#gotchas-and-tips)
- [TODO](#todo)
- [Dependencies](#dependencies)
- [Links](#links)

## Why?
### Pros
- Audio is not tied to any Godot-Node, it can be controlled from anywhere.
- Flexibility choose which driver to use at runtime, depending on the system and apis that are available.
- Full control over input buffer (for recording requirements) and output buffer. (beneficial for audio tools)
- Lower latency due to more direct access to audio hardware. (beneficial for music rythm games)(*1)

### Cons
- Requires to write your own audio pipeline. (from decoding the file format, to filling the playback buffer)
- More code to detect and configure the driver.

*1) only applies when utilizing c++ audio callback, not GDScript bindings

## Setup:
### via submodule
- open a terminal inside the godot source folder and run the following commands:  
`cd modules`  
`git submodule add https://github.com/sebastian-heinz/portaudio.git`  
`git submodule update --init --recursive`  

### via download 
- Download the zip (https://github.com/sebastian-heinz/portaudio/archive/master.zip) and extract it into the godot/modules-folder.
  Since it is missing the portaudio source, it has to be manually added. Get it from http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz and paste it into the empty   portaudio folder. (./portaudio/portaudio/)  
  <img src="/doc/port_audio_example.png" width="400">

## Building
Edit the [SCsub](./SCsub) file and comment / uncomment the desired host apis in `use_host_api = [`.
When building godot it will check if the host api is supported for the platform (windows, linux or osx) and available inside the `use_host_api`-array. Only then the host api will be available.

## Driver
| Driver    | windows       | x11           | osx 
| --------- | ------------- | ------------- |  ------------ |
| ASIO      | Working (*1)  | Not Tested    | Not Available |
| DSOUND    | Working       | Not Available | Not Available |
| WASAPI    | Working (*2)  | Not Available | Not Available |
| WDM/KS    | Working       | Not Available | Not Available |
| WMME      | Error         | Not Available | Not Available |
| JACK      | Not Available | Not Tested    | Not Available |
| ALSA      | Not Available | Not Tested    | Not Available |
| ASIHPI    | Not Available | Not Tested    | Not Available |
| COREAUDIO | Not Available | Not Available | Working       |
| OSS       | Not Available | Not Tested    | Not Available |

*1) requires that the enduser has asio drivers installed (ex. ASIO4ALL)  
*2) if used godots wasapi driver will be disabled  

## Godot Integration
- `PortAudio` is a singleton class, its purpose is to provide wrapper methods for all PortAudio calls (PA_*) with a godot type friendly interface. For direct access from anywhere

### Nodes
A `PortAudioTestNode` exists, simply add it to a scene via the editor and it will enumerate your host apis and devices and print them out:  
<img src="/doc/test_node.png" width="400">  

### Example GDScripts
#### Tone Generator:
Youtube Video:  
[![Godot - PortAudio Module](http://img.youtube.com/vi/6x_jZo_QxvE/0.jpg)](http://www.youtube.com/watch?v=6x_jZo_QxvE "Godot - PortAudio Module")
```
extends Node

class_name ToneGenerator

# Member variables
var sample_rate = 8000
var frequenzy_hz = 40
var duration = 1

# Constants
const SAMPLE_SIZE = 4 # float
	
func _ready():
	start_port_audio()
	return

func start_port_audio():
	var samples = duration * sample_rate
	
	var device_index = PortAudio.get_default_output_device()
	
	var out_p = PortAudioStreamParameter.new()
	out_p.set_device_index(device_index)
	out_p.set_channel_count(2)
	
	var stream = PortAudioStream.new()
	stream.set_output_stream_parameter(out_p)
	stream.set_sample_format(PortAudioStreamParameter.FLOAT_32)
	stream.set_frames_per_buffer(samples * SAMPLE_SIZE)

	var audio_callback = funcref(self, "audio_callback")
	var user_data = samples
	var err = PortAudio.open_stream(stream, audio_callback, user_data)
	if(err != PortAudio.NO_ERROR):
		push_error("open_stream: %s" % err)
		
	err = PortAudio.start_stream(stream)	
	if(err != PortAudio.NO_ERROR):
		push_error("start_stream: %s" % err)
		return

# Audio Callback
func audio_callback(data : PortAudioCallbackData):
	var buff = data.get_output_buffer()
	var samples = data.get_user_data()
	for i in range (samples):
		var sample : float = sin(2 * PI * float(i) / ( sample_rate / frequenzy_hz))
		buff.put_float(sample)
	return PortAudio.CONTINUE

```

### C++
This module will add PortAudio to the include path. It allows to work with PortAudio s library directly:   
```
#include <portaudio.h>
```

For tutorials on how to use PortAudio have a look at the official documentation: http://portaudio.com/docs/v19-doxydocs/initializing_portaudio.html

## Gotchas and Tips

### Callback Tips
Please refer to https://app.assembla.com/wiki/show/portaudio/Tips_Callbacks for a better understand about the delicate callback function.  

Especially:
```
Regarding crossing language boundaries such as calling Java or Lua:
In general it should be avoided. But, in fact, Lua has a bounded time GC so, 
like the Supercollider language, it could be used in a PortAudio callback 
so long as you make sure it doesn’t violate the points I made above: 
i.e. avoid calling the OS level memory allocator, file I/O, or doing other things 
which violate the above in your Lua user functions. . 
That said, running Lua in a PortAudio callback is definitely at the experimental end of the spectrum.
```

Exposing PortAudio to GDScript will have some performance overhead and introduces additional audio latency. If you are looking for low latency it would be best to utilzie the CallbackFunction in C++. To get a better understanding of how long the callback took, the duration in μs (Microsecond) can be obtained from the callback data `PortAudioCallbackData::get_last_call_duration()`.

### Frames Per Buffer
The callback provides a `frames_per_buffer`-variable. This does not represent bytes. Depending on the format (FLOAT_32 = 4bytes, INT_16 = 2bytes) and channels the buffer size can be calculated.
```
bytes_per_channel = (FLOAT_32 = 4 bytes) (INT_16 = 2byte) (INT_8 = 1byte)
buffer_size = frames_per_buffer * channels * bytes_per_channel
```
Ensure that each callback the buffer is filled up correctly or it could result in slow and crackling audio. The same also applies when utilizing blocking mode via `write()`.

### Time spend in Callback
If the execution time of the callback function is longer than the playback data provided to the buffer the audio might also become slow and crackling.
To calculate the playback duration of the buffer the requested frames can be divided by the sample rate.
```
time_request_seconds = frames_per_buffer / sample_rate
```
The execution time of the audio loop has to be faster than this time.

### Callback Result
The return value of the callback indicates if it should continue to be called or it can be signaled to stop.  
C++:
```
enum PortAudioCallbackResult {
	CONTINUE = 0,
	COMPLETE = 1,
	ABORT = 2,
};
```
GDScript:
```
PortAudio.CONTINUE
PortAudio.COMPLETE
PortAudio.ABORT
```

## TODO
- doc_classes need to be written for GDScript documentation.
- [WIN] [WDMKS]-driver clashes with godot imports. (error LNK2005: KSDATAFORMAT_SUBTYPE_MIDI already defined in dxguid.lib(dxguid.obj))
- [LINUX] build pipeline untested.

## Dependencies
- Godot: `4.x`
- PortAudio: https://github.com/PortAudio/portaudio/commit/eec7bb739771381ab698a6a611d933bdd72cbd8f
- ASIO SDK: `2.0+` (optional)

## Links  
Godot:  
https://github.com/godotengine/godot  

Portaudio:  
https://app.assembla.com/spaces/portaudio/git/source  
http://www.portaudio.com/  
https://github.com/PortAudio/portaudio

ASIO SDK 2.0:  
https://www.steinberg.net/en/company/developers.html  

Asio4All Driver:  
http://www.asio4all.org/
