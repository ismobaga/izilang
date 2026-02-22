#include "native_audio.hpp"
#include "native.hpp"
#include "interpreter.hpp"
#include <memory>
#include <string>

#ifdef HAVE_RAYLIB
#include <raylib.h>
#else
#include "../vendor/miniaudio.h"
#endif

namespace izi {

// ---------------------------------------------------------------------------
// Sound wrapper (holds a raylib Sound handle)
// ---------------------------------------------------------------------------
#ifdef HAVE_RAYLIB
struct SoundHandle {
    ::Sound sound{};
    bool loaded = false;
    ~SoundHandle() {
        if (loaded) {
            UnloadSound(sound);
            loaded = false;
        }
    }
};

struct MusicHandle {
    ::Music music{};
    bool loaded = false;
    ~MusicHandle() {
        if (loaded) {
            UnloadMusicStream(music);
            loaded = false;
        }
    }
};
#else
// miniaudio-backed audio handles

// Global engine state — shared across all audio calls within one process.
struct MaEngineState {
    ma_engine engine{};
    bool initialized = false;
    float pendingVolume = 1.0f;

    ~MaEngineState() {
        if (initialized) {
            ma_engine_uninit(&engine);
            initialized = false;
        }
    }
};

static MaEngineState& getAudioEngine() {
    static MaEngineState s;
    return s;
}

struct SoundHandle {
    ma_sound sound{};
    bool loaded = false;
    ~SoundHandle() {
        if (loaded) {
            ma_sound_uninit(&sound);
            loaded = false;
        }
    }
};

// Music uses the same ma_sound API but is initialised in streaming mode.
struct MusicHandle {
    ma_sound sound{};
    bool loaded = false;
    ~MusicHandle() {
        if (loaded) {
            ma_sound_uninit(&sound);
            loaded = false;
        }
    }
};
#endif

// ---------------------------------------------------------------------------
// Build a sound object from a loaded sound handle
// ---------------------------------------------------------------------------
#ifdef HAVE_RAYLIB
static Value buildSoundObject(std::shared_ptr<SoundHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["play"] = Value{std::make_shared<NativeFunction>("play", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) PlaySound(handle->sound);
            return Nil{};
        })};

    obj->entries["stop"] = Value{std::make_shared<NativeFunction>("stop", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) StopSound(handle->sound);
            return Nil{};
        })};

    obj->entries["pause"] = Value{std::make_shared<NativeFunction>("pause", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) PauseSound(handle->sound);
            return Nil{};
        })};

    obj->entries["resume"] = Value{std::make_shared<NativeFunction>("resume", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ResumeSound(handle->sound);
            return Nil{};
        })};

    obj->entries["isPlaying"] = Value{std::make_shared<NativeFunction>("isPlaying", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            return handle->loaded && static_cast<bool>(IsSoundPlaying(handle->sound));
        })};

    obj->entries["setVolume"] = Value{std::make_shared<NativeFunction>("setVolume", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("sound.setVolume() takes 1 argument (volume).");
            if (handle->loaded)
                SetSoundVolume(handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPitch"] = Value{std::make_shared<NativeFunction>("setPitch", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("sound.setPitch() takes 1 argument (pitch).");
            if (handle->loaded)
                SetSoundPitch(handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPan"] = Value{std::make_shared<NativeFunction>("setPan", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("sound.setPan() takes 1 argument (pan).");
            if (handle->loaded)
                SetSoundPan(handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["unload"] = Value{std::make_shared<NativeFunction>("unload", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) {
                UnloadSound(handle->sound);
                handle->loaded = false;
            }
            return Nil{};
        })};

    return Value{obj};
}
#else
static Value buildSoundObject(std::shared_ptr<SoundHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["play"] = Value{std::make_shared<NativeFunction>("play", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_start(&handle->sound);
            return Nil{};
        })};

    obj->entries["stop"] = Value{std::make_shared<NativeFunction>("stop", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_stop(&handle->sound);
            return Nil{};
        })};

    obj->entries["pause"] = Value{std::make_shared<NativeFunction>("pause", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_stop(&handle->sound);
            return Nil{};
        })};

    obj->entries["resume"] = Value{std::make_shared<NativeFunction>("resume", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_start(&handle->sound);
            return Nil{};
        })};

    obj->entries["isPlaying"] = Value{std::make_shared<NativeFunction>("isPlaying", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            return handle->loaded && static_cast<bool>(ma_sound_is_playing(&handle->sound));
        })};

    obj->entries["setVolume"] = Value{std::make_shared<NativeFunction>("setVolume", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("sound.setVolume() takes 1 argument (volume).");
            if (handle->loaded)
                ma_sound_set_volume(&handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPitch"] = Value{std::make_shared<NativeFunction>("setPitch", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("sound.setPitch() takes 1 argument (pitch).");
            if (handle->loaded)
                ma_sound_set_pitch(&handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPan"] = Value{std::make_shared<NativeFunction>("setPan", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("sound.setPan() takes 1 argument (pan).");
            if (handle->loaded)
                ma_sound_set_pan(&handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["unload"] = Value{std::make_shared<NativeFunction>("unload", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) {
                ma_sound_uninit(&handle->sound);
                handle->loaded = false;
            }
            return Nil{};
        })};

    return Value{obj};
}
#endif

// ---------------------------------------------------------------------------
// Build a music stream object from a loaded music handle
// ---------------------------------------------------------------------------
#ifdef HAVE_RAYLIB
static Value buildMusicObject(std::shared_ptr<MusicHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["play"] = Value{std::make_shared<NativeFunction>("play", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) PlayMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["stop"] = Value{std::make_shared<NativeFunction>("stop", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) StopMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["pause"] = Value{std::make_shared<NativeFunction>("pause", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) PauseMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["resume"] = Value{std::make_shared<NativeFunction>("resume", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ResumeMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["update"] = Value{std::make_shared<NativeFunction>("update", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) UpdateMusicStream(handle->music);
            return Nil{};
        })};

    obj->entries["isPlaying"] = Value{std::make_shared<NativeFunction>("isPlaying", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            return handle->loaded && static_cast<bool>(IsMusicStreamPlaying(handle->music));
        })};

    obj->entries["setVolume"] = Value{std::make_shared<NativeFunction>("setVolume", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.setVolume() takes 1 argument (volume).");
            if (handle->loaded)
                SetMusicVolume(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPitch"] = Value{std::make_shared<NativeFunction>("setPitch", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.setPitch() takes 1 argument (pitch).");
            if (handle->loaded)
                SetMusicPitch(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPan"] = Value{std::make_shared<NativeFunction>("setPan", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.setPan() takes 1 argument (pan).");
            if (handle->loaded)
                SetMusicPan(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["seek"] = Value{std::make_shared<NativeFunction>("seek", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.seek() takes 1 argument (position).");
            if (handle->loaded)
                SeekMusicStream(handle->music, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["getLength"] = Value{std::make_shared<NativeFunction>("getLength", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            return static_cast<double>(GetMusicTimeLength(handle->music));
        })};

    obj->entries["getTimePlayed"] = Value{std::make_shared<NativeFunction>("getTimePlayed", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            return static_cast<double>(GetMusicTimePlayed(handle->music));
        })};

    obj->entries["unload"] = Value{std::make_shared<NativeFunction>("unload", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) {
                UnloadMusicStream(handle->music);
                handle->loaded = false;
            }
            return Nil{};
        })};

    return Value{obj};
}
#else
static Value buildMusicObject(std::shared_ptr<MusicHandle> handle) {
    auto obj = std::make_shared<Map>();

    obj->entries["play"] = Value{std::make_shared<NativeFunction>("play", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_start(&handle->sound);
            return Nil{};
        })};

    obj->entries["stop"] = Value{std::make_shared<NativeFunction>("stop", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_stop(&handle->sound);
            return Nil{};
        })};

    obj->entries["pause"] = Value{std::make_shared<NativeFunction>("pause", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_stop(&handle->sound);
            return Nil{};
        })};

    obj->entries["resume"] = Value{std::make_shared<NativeFunction>("resume", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) ma_sound_start(&handle->sound);
            return Nil{};
        })};

    obj->entries["update"] = Value{std::make_shared<NativeFunction>("update", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
            // miniaudio streams are updated automatically on a background thread.
            return Nil{};
        })};

    obj->entries["isPlaying"] = Value{std::make_shared<NativeFunction>("isPlaying", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            return handle->loaded && static_cast<bool>(ma_sound_is_playing(&handle->sound));
        })};

    obj->entries["setVolume"] = Value{std::make_shared<NativeFunction>("setVolume", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.setVolume() takes 1 argument (volume).");
            if (handle->loaded)
                ma_sound_set_volume(&handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPitch"] = Value{std::make_shared<NativeFunction>("setPitch", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.setPitch() takes 1 argument (pitch).");
            if (handle->loaded)
                ma_sound_set_pitch(&handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["setPan"] = Value{std::make_shared<NativeFunction>("setPan", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.setPan() takes 1 argument (pan).");
            if (handle->loaded)
                ma_sound_set_pan(&handle->sound, static_cast<float>(asNumber(args[0])));
            return Nil{};
        })};

    obj->entries["seek"] = Value{std::make_shared<NativeFunction>("seek", 1,
        [handle](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("music.seek() takes 1 argument (position).");
            if (handle->loaded)
                ma_sound_seek_to_pcm_frame(&handle->sound,
                    static_cast<ma_uint64>(asNumber(args[0]) *
                        ma_engine_get_sample_rate(&getAudioEngine().engine)));
            return Nil{};
        })};

    obj->entries["getLength"] = Value{std::make_shared<NativeFunction>("getLength", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            float len = 0.0f;
            ma_sound_get_length_in_seconds(&handle->sound, &len);
            return static_cast<double>(len);
        })};

    obj->entries["getTimePlayed"] = Value{std::make_shared<NativeFunction>("getTimePlayed", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (!handle->loaded) return 0.0;
            float t = 0.0f;
            ma_sound_get_cursor_in_seconds(&handle->sound, &t);
            return static_cast<double>(t);
        })};

    obj->entries["unload"] = Value{std::make_shared<NativeFunction>("unload", 0,
        [handle](Interpreter&, const std::vector<Value>&) -> Value {
            if (handle->loaded) {
                ma_sound_uninit(&handle->sound);
                handle->loaded = false;
            }
            return Nil{};
        })};

    return Value{obj};
}
#endif

// ---------------------------------------------------------------------------
// Build the top-level audio module Map
// ---------------------------------------------------------------------------
Value createAudioModule(Interpreter& /*interp*/) {
    auto module = std::make_shared<Map>();

    // audio.initDevice()
    module->entries["initDevice"] = Value{std::make_shared<NativeFunction>("initDevice", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            InitAudioDevice();
#else
            auto& eng = getAudioEngine();
            if (eng.initialized) return Nil{};
            ma_result result = ma_engine_init(nullptr, &eng.engine);
            if (result != MA_SUCCESS)
                throw std::runtime_error(
                    "audio.initDevice(): failed to initialize audio device (code " +
                    std::to_string(result) + ").");
            eng.initialized = true;
            ma_engine_set_volume(&eng.engine, eng.pendingVolume);
#endif
            return Nil{};
        })};

    // audio.closeDevice()
    module->entries["closeDevice"] = Value{std::make_shared<NativeFunction>("closeDevice", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            CloseAudioDevice();
#else
            auto& eng = getAudioEngine();
            if (eng.initialized) {
                ma_engine_uninit(&eng.engine);
                eng.initialized = false;
            }
#endif
            return Nil{};
        })};

    // audio.isDeviceReady() -> bool
    module->entries["isDeviceReady"] = Value{std::make_shared<NativeFunction>("isDeviceReady", 0,
        [](Interpreter&, const std::vector<Value>&) -> Value {
#ifdef HAVE_RAYLIB
            return static_cast<bool>(IsAudioDeviceReady());
#else
            return getAudioEngine().initialized;
#endif
        })};

    // audio.setMasterVolume(volume)
    module->entries["setMasterVolume"] = Value{std::make_shared<NativeFunction>("setMasterVolume", 1,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1)
                throw std::runtime_error("audio.setMasterVolume() takes 1 argument (volume).");
#ifdef HAVE_RAYLIB
            SetMasterVolume(static_cast<float>(asNumber(args[0])));
#else
            auto& eng = getAudioEngine();
            float vol = static_cast<float>(asNumber(args[0]));
            eng.pendingVolume = vol;
            if (eng.initialized)
                ma_engine_set_volume(&eng.engine, vol);
#endif
            return Nil{};
        })};

    // audio.loadSound(path) -> sound object
    module->entries["loadSound"] = Value{std::make_shared<NativeFunction>("loadSound", 1,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("audio.loadSound() takes 1 string argument (path).");
            const std::string& path = std::get<std::string>(args[0]);
#ifdef HAVE_RAYLIB
            auto handle = std::make_shared<SoundHandle>();
            handle->sound = LoadSound(path.c_str());
            handle->loaded = true;
            return buildSoundObject(handle);
#else
            auto& eng = getAudioEngine();
            if (!eng.initialized)
                throw std::runtime_error(
                    "audio.loadSound(): audio device not initialized. Call audio.initDevice() first.");
            auto handle = std::make_shared<SoundHandle>();
            ma_result result = ma_sound_init_from_file(
                &eng.engine, path.c_str(), MA_SOUND_FLAG_DECODE, nullptr, nullptr, &handle->sound);
            if (result != MA_SUCCESS)
                throw std::runtime_error(
                    "audio.loadSound(): failed to load sound from '" + path + "' (code " +
                    std::to_string(result) + ").");
            handle->loaded = true;
            return buildSoundObject(handle);
#endif
        })};

    // audio.loadMusic(path) -> music stream object
    module->entries["loadMusic"] = Value{std::make_shared<NativeFunction>("loadMusic", 1,
        [](Interpreter&, const std::vector<Value>& args) -> Value {
            if (args.size() != 1 || !std::holds_alternative<std::string>(args[0]))
                throw std::runtime_error("audio.loadMusic() takes 1 string argument (path).");
            const std::string& path = std::get<std::string>(args[0]);
#ifdef HAVE_RAYLIB
            auto handle = std::make_shared<MusicHandle>();
            handle->music = LoadMusicStream(path.c_str());
            handle->loaded = true;
            return buildMusicObject(handle);
#else
            auto& eng = getAudioEngine();
            if (!eng.initialized)
                throw std::runtime_error(
                    "audio.loadMusic(): audio device not initialized. Call audio.initDevice() first.");
            auto handle = std::make_shared<MusicHandle>();
            // Streaming mode: no MA_SOUND_FLAG_DECODE so data is streamed from disk.
            ma_result result = ma_sound_init_from_file(
                &eng.engine, path.c_str(), 0, nullptr, nullptr, &handle->sound);
            if (result != MA_SUCCESS)
                throw std::runtime_error(
                    "audio.loadMusic(): failed to load music from '" + path + "' (code " +
                    std::to_string(result) + ").");
            handle->loaded = true;
            return buildMusicObject(handle);
#endif
        })};

    return Value{module};
}

}  // namespace izi
