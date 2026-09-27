# gr-drm-next

`gr-drm-next` is an experimental GNU Radio 3.10 transmitter and receiver based
on the Digital Radio Mondiale waveform and the original `gr-drm` project.

> [!IMPORTANT]
> This project is **not a standards-compliant DRM implementation**. Its current
> audio path uses Opus inside a DReaM-inspired MSC superframe. Opus is not an
> audio codec defined by the ETSI DRM system specification, and current SDC
> signalling does not fully describe the experimental payload. Do not expect a
> conventional DRM or DReaM receiver to decode these transmissions.

The goal is to provide an open laboratory for modernizing the original GNU
Radio blocks, experimenting with alternative codecs, and developing a complete
transmitter/receiver chain. It must not be represented as a compliant DRM
broadcast system.

## Current state

The project currently includes:

- GNU Radio 3.10 C++ blocks with Python bindings in the `drm_next` namespace.
- A DRM-derived OFDM transmitter and receiver chain.
- FAC, SDC, and MSC channel coding/decoding blocks.
- Experimental mono Opus encoder and decoder blocks.
- Twenty 20 ms Opus packets per 400 ms audio superframe, with 12-bit packet
  borders and CRC-8 bytes inspired by DReaM's experimental Opus work.
- Configurable Opus bitrate, VBR/CBR mode, application, signal type, maximum
  bandwidth, complexity, and DTX.
- FAC-driven receiver detection of:
  - MSC 16-QAM or 64-QAM symmetrical mapping;
  - SDC 4-QAM or 16-QAM mapping;
  - short or long interleaving.
- A full audio loopback example with OFDM modulation, synchronization, channel
  decoding, constellation display, and sound-card output.

The legacy FAAC encoder and FAAD2 decoder blocks remain available for research
and comparison. FAAC produces ordinary raw AAC-LC; it does not provide the DRM
ER-AAC encoder required for a compliant legacy DRM audio service.

## Interoperability

The current Opus mode is an internal `gr-drm-next` experiment. In particular:

- Opus is not part of the official ETSI DRM audio specification.
- The implementation borrows framing ideas from DReaM but is not yet wire
  compatible with DReaM's experimental 48 kHz stereo Opus extension.
- The example presently uses 12 or 24 kHz mono audio.
- The SDC generator still uses the original AAC-oriented type 9 audio
  signalling and therefore does not accurately announce the Opus payload.
- A receiver currently needs bootstrap values for robustness mode, spectrum
  occupancy, MSC protection level, and audio sample rate.
- Mapping and interleaver selection can be recovered from FAC by the
  `Opus Receiver with FAC Mapping Detection` block.
- Automatic protection-level, multiplex, service, and audio-configuration
  recovery from SDC is still under development.
- Fully blind robustness-mode and spectrum-occupancy acquisition is not yet
  implemented.

Transmitter and receiver do not need to run in the same process or share a
`transm_params` object. The automatic receiver owns its bootstrap configuration
and derives the supported mapping fields from the received FAC.

## Dependencies

Required build dependencies are:

- GNU Radio 3.10 or newer;
- CMake 3.16 or newer;
- a C++17 compiler;
- Python 3 and pybind11;
- libopus;
- FAAC;
- FAAD2.

FAAC, FAAD2, and libopus are external system dependencies and are not bundled
or vendored in this repository. The AAC decoder requires a FAAD2 build with DRM
decoder support when its DRM mode is used. The experimental Opus loopback does
not use FAAC or FAAD2 at runtime, although the current build still requires them
because the legacy AAC blocks are built as part of the module.

## Building

Install the development packages for the dependencies, then run:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build
```

Set `CMAKE_INSTALL_PREFIX` when GNU Radio is installed outside the default
prefix. On Linux, refresh the dynamic linker cache when required by the chosen
installation prefix.

After installation, restart GNU Radio Companion so it reloads the installed
block definitions.

## Examples

The main flowgraph is:

```text
examples/drm_audio_loopback.grc
```

It connects a WAV source to the experimental Opus encoder, MSC channel coding,
cell mapping, OFDM modulation, receiver synchronization, channel decoding, Opus
decoding, and a GNU Radio Audio Sink. Change the WAV path to a mono file whose
sample rate matches the selected audio configuration.

For the default RM B, 10 kHz, 16-QAM configuration, the available Opus payload
limits the encoder to approximately 13.2 kbit/s. A bitrate of `0` selects the
maximum that fits the active MSC configuration. Values exceeding the available
MSC capacity are rejected instead of truncating packets.

The separately installed `Opus Receiver with FAC Mapping Detection` hierarchy
is intended for receiver flowgraphs where the transmitter may be on another
machine. The existing loopback still exposes the individual low-level receiver
blocks for development and inspection.

## Limitations

Notable missing or incomplete functionality includes:

- standards-compliant xHE-AAC or DRM HE-AAC audio encoding;
- compliant Opus signalling, because no such mode exists in the DRM standard;
- hierarchical MSC mappings (HMsym and HMmix);
- unequal error protection;
- multiple audio or data services;
- complete SDC entity parsing and runtime service reconfiguration;
- fully blind waveform acquisition;
- comprehensive operation over noisy or fading radio channels;
- DRM+ validation.

This is active experimental software. A successful local loopback does not
establish regulatory compliance, spectral-mask compliance, or interoperability
with commercial DRM equipment.

## Licensing

The project source is distributed under the GNU General Public License version
3; see [COPYING.txt](COPYING.txt). External codec libraries retain their own
licenses and must be installed separately. No FAAC, FAAD2, libopus, or DReaM
source code is included in this repository.

Digital Radio Mondiale, DRM, DReaM, AAC, and Opus names are used descriptively.
This project is independent experimental work and is not endorsed or certified
by the DRM Consortium or the maintainers of DReaM.
