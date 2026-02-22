# audio — Audio Playback

The `audio` module provides audio device management, sound effect loading, and
music streaming using [miniaudio](https://miniaud.io/) when raylib is not
available, or raylib's built-in audio API when built with `--raylib=<path>`.

> **No external dependencies required on Linux, macOS, or Windows.**
> `miniaudio` is a single-header C library bundled under `src/vendor/` that
> uses platform-native audio APIs (ALSA/PulseAudio on Linux, Core Audio on
> macOS, WASAPI on Windows) and falls back to a null device when no hardware is
> present.

## Supported Formats

WAV, MP3, FLAC, OGG Vorbis, and any other format supported by miniaudio's
built-in decoders.

## Import

```izilang
import "audio";

// Or with a namespace alias
import * as aud from "std.audio";
```

## Quick Start

```izilang
import "audio";

// 1. Initialise the audio device
audio.initDevice();
audio.setMasterVolume(0.8);

// 2. Load a short sound effect (decoded entirely into memory)
var sfx = audio.loadSound("assets/jump.wav");

// 3. Load a music track (streamed from disk)
var music = audio.loadMusic("assets/background.mp3");

// 4. Play
sfx.play();
music.play();

// 5. Game loop (update is a no-op with miniaudio — streaming is automatic)
// music.update();   // only needed when built with raylib

// 6. Cleanup
sfx.unload();
music.unload();
audio.closeDevice();
```

## Module Functions

### `audio.initDevice()`

Open the audio output device and start the audio engine. Must be called before
loading or playing any sounds.

Throws a runtime error if no audio device could be opened.

### `audio.closeDevice()`

Stop the audio engine and release the audio device. Safe to call even if the
device was never initialised.

### `audio.isDeviceReady()` → bool

Return `true` if the audio device has been successfully initialised.

```izilang
if (!audio.isDeviceReady()) {
    audio.initDevice();
}
```

### `audio.setMasterVolume(volume)`

Set the global output volume. `volume` is a number in the range `[0.0, 1.0]`.

This may be called before `initDevice()`; the value is remembered and applied
when the device is opened.

```izilang
audio.setMasterVolume(0.5);  // 50 % volume
```

### `audio.loadSound(path)` → sound object

Decode and load an audio file entirely into memory. Suitable for short sound
effects that need low-latency playback.

Requires `audio.initDevice()` to have been called first.

```izilang
var sfx = audio.loadSound("assets/click.wav");
```

### `audio.loadMusic(path)` → music object

Open an audio file for streaming playback. Suitable for long music tracks that
should not be held entirely in RAM.

Requires `audio.initDevice()` to have been called first.

```izilang
var bgm = audio.loadMusic("assets/theme.mp3");
```

## Sound Object Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `play` | `()` | Start playback from the current position |
| `stop` | `()` | Stop playback and rewind to the beginning |
| `pause` | `()` | Pause playback at the current position |
| `resume` | `()` | Resume playback from the paused position |
| `isPlaying` | `() → bool` | Return `true` if the sound is currently playing |
| `setVolume` | `(volume)` | Set per-sound volume `[0.0, 1.0]` |
| `setPitch` | `(pitch)` | Set playback pitch (1.0 = normal) |
| `setPan` | `(pan)` | Set stereo pan (`−1.0` = left, `0.0` = centre, `1.0` = right) |
| `unload` | `()` | Free the sound's memory |

## Music Object Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `play` | `()` | Start streaming playback |
| `stop` | `()` | Stop and rewind |
| `pause` | `()` | Pause streaming |
| `resume` | `()` | Resume streaming |
| `update` | `()` | No-op with miniaudio (streaming is automatic); kept for API compatibility with the raylib backend |
| `isPlaying` | `() → bool` | Return `true` if music is playing |
| `setVolume` | `(volume)` | Per-track volume `[0.0, 1.0]` |
| `setPitch` | `(pitch)` | Playback pitch (1.0 = normal) |
| `setPan` | `(pan)` | Stereo pan |
| `seek` | `(seconds)` | Seek to position in seconds |
| `getLength` | `() → number` | Duration in seconds |
| `getTimePlayed` | `() → number` | Current playback position in seconds |
| `unload` | `()` | Close the stream and free resources |

## Complete Example

```izilang
import "audio";

audio.initDevice();
audio.setMasterVolume(0.8);

var sfx   = audio.loadSound("assets/explosion.wav");
var music = audio.loadMusic("assets/level1.mp3");

music.play();
music.setVolume(0.6);

// Simulate a game loop
var i = 0;
while (i < 60) {
    // Trigger effect every 20 frames
    if (i % 20 == 0) {
        sfx.play();
    }
    i = i + 1;
}

print("Track length: " + music.getLength() + "s");
print("Position: " + music.getTimePlayed() + "s");

sfx.unload();
music.unload();
audio.closeDevice();
```

## Notes

- `audio.initDevice()` is idempotent — calling it more than once is safe.
- On headless systems without audio hardware, miniaudio falls back to a null
  output device, so the module remains fully functional (useful for testing and
  server environments).
- `music.update()` exists for API compatibility with the raylib backend but is a
  no-op with miniaudio; streaming runs on a background thread automatically.
- Sound and music objects are garbage-collected when they go out of scope, but
  calling `unload()` explicitly is recommended to release resources promptly.
