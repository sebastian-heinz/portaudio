portaudio
===
module for godot

## Why?
### Pros
- Audio is not tied to any Godot-Node, it can be controlled from anywhere.
- Flexibility choose which driver to use at runtime, depending on the system and apis that are available.
- Most likely a lower latency due to more direct access to audio hardware. (beneficial for music rythm games)
- Full control over input buffer (for recording requirements) and output buffer. (beneficial for audio tools)

### Cons
- Requires to write your own audio pipeline. (from decoding the file format, to filling the playback buffer)
- More code to detect and configure the driver.

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
| COREAUDIO | Not Available | Not Available | Not Tested    |
| OSS       | Not Available | Not Tested    | Not Available |

*1) requires that the enduser has asio drivers installed (ex. ASIO4ALL)  
*2) if used godots wasapi driver will be disabled  

## Godot
- `PortAudio` is a singleton class, its purpose is to provide wrapper methods for all PortAudio calls (PA_*) with a godot type friendly interface. For direct access from anywhere

### GDScript Example
#### Tone Generator:
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
		push_error("start_stream: %s" % err)
		
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
	return 0

```

## Examples
A `PortAudioTestNode` exists, simply add it to a scene via the editor and it will enumerate your host apis and devices and print them out:  
<img src="/doc/test_node.png" width="800">  

## TODO
- doc_classes need to be written for GDScript documentation.
- Provide some wav / ogg playback capabilities out of the box for ease of access, maybe able to utilize existing godot `AudioStream` class.
- [WIN] [WDMKS]-driver clashes with godot imports. (error LNK2005: KSDATAFORMAT_SUBTYPE_MIDI already defined in dxguid.lib(dxguid.obj))
- [LINUX] build pipeline untested.
- [OSX] build pipeline untested.

## Dependencies
- Godot: `3.2`
- PortAudio: `pa_stable_v190600_20161030`
- ASIO SDK: `2.0+` (optional)

## Links  
Godot:  
https://github.com/godotengine/godot  

Portaudio:  
https://app.assembla.com/spaces/portaudio/git/source  
http://www.portaudio.com/  

ASIO SDK 2.0:  
https://www.steinberg.net/en/company/developers.html  

Asio4All Driver:  
http://www.asio4all.org/
