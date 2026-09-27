GR-DRM
======

A SOFTWARE DRM/DRM+ TRANSMITTER FOR GNU RADIO

This checkout is the `gr-drm-next` development line: a GNU Radio 3.10 DRM
encoder/receiver project. It retains the existing `drm` module name for
flowgraph compatibility while receiver blocks are developed alongside the
transmitter.

Contents
--------

1: Installation

2: Usage

3: Features

4: (Current) Constraints

5: Known Bugs


Installation
------------

Dependencies: GNU Radio 3.10 or newer, Python 3, pybind11, FAAC, and FAAD2.

FAAC remains the audio encoder used by the transmitter. Both the legacy
`faacEnc*` API and the FAAC 2.1 API are supported. The encoder is configured
for raw MPEG-4 AAC-LC output, preserving the existing DRM audio framing.
FAAD2 is used by the DRM AAC decoder block. FAAD2 is a system dependency and
is intentionally not bundled with this project.

From Source (manual)
====================

Install the development packages for the dependencies above, then build with:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build
```

Set `CMAKE_INSTALL_PREFIX` if the module should be installed outside GNU
Radio's default prefix. On Linux, refresh the dynamic linker cache if required
by the selected prefix.
		
Usage
-----

The `examples/drm_audio_loopback.grc` flowgraph connects the existing FAAC
encoder to the FAAD2 decoder through a vectorized DRM MSC frame and plays the
recovered PCM through GNU Radio's Audio Sink. It is a perfect audio-path test;
it intentionally bypasses OFDM synchronization, channel estimation, and MLC
decoding.

After successful installation of gr-drm, you can either use the flow graph
in `apps/grc_flowgraph` or the GUI in `apps/gui`. The DRM+ flow graph is 
completely untested due to the lack of a receiver.

Of course you have to set the path to your USRP (or leave it blank for 
autodetection) and a source wav-file (either 12 or 24 kHz). There are also 
several other parameters you can change (see section Features). It is also 
possible to record a wav-file that can be decoded with DREAM.

As wav files usually aren't sampled with 12 or 24 kHz, I use sox for convenient
resampling.
Syntax: `sox <mysong.wav> -r <new_sample_rate> <mysongresampled.wav> resample`


Features
--------

This project features a DRM/DRM+ software transmitter fully integrated into GNU Radio
Companion.

You are also free to play around with several robustness modes (RM) and spectrum 
occupancies (SO, signal bandwidth) ranging from 4.5 to 20 kHz. The corresponding
bit rates vary from below 5 kbps to about 55 kbps. A configuration that is widely
used is RM B (==1) and 10 kHz bandwidth (SO 3). Among other parameters, the
station label and a text message can also be set. Please note that not every combination of Robustness 
mode and Spectrum Occupancy is valid. 


(Current) Constraints
---------------------

As this project is still under development, there are some features of the DRM
standard that are not (yet) available. The most important are:

- Unequal Error Protection (UEP)
- Hierarchical mapping (HMsym and HMmix)
- multiple audio/data streams (currently only one audio stream in AAC Mono (no 
  SBR) is possible)


Known Bugs
----------

- text message corrupted for SO 5 / RM A (and possibly other modes) 

If you find any bugs or have ideas you think the project could benefit 
from, just write me: felix.wunsch[at]kit.edu

last updated on: 2016/10/08
