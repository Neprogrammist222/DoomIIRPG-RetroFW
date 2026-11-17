#include "Sound.h"
#include "Sounds.h"
#include "CAppContainer.h"
#include "ZipFile.h"
#include <SDL/SDL.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>

static constexpr int NUM_SOUND_FILES = sizeof(Sounds::RESOURCE_FILE_NAMES) / sizeof(*Sounds::RESOURCE_FILE_NAMES);
static constexpr int MAX_CHANNELS = 10;

struct CachedWav {
    uint8_t* data;
    uint32_t len;
    bool tried;
};

struct ChannelPlayback {
    bool active;
    bool loop;
    bool isMusic;
    int16_t resID;
    int priority;
    uint32_t pos;
};

static CachedWav g_wavCache[NUM_SOUND_FILES];
static ChannelPlayback g_playState[MAX_CHANNELS];
static SDL_AudioSpec g_audioSpec;
static bool g_audioOpened = false;

static bool isMusicRes(int16_t resID) {
    return (resID >= 1067 && resID <= 1071);
}

static void sdlAudioCallback(void* userdata, Uint8* stream, int len) {
    std::memset(stream, 0, len);
    Sound* snd = static_cast<Sound*>(userdata);
    if (!snd) return;

    for (int i = 0; i < MAX_CHANNELS; ++i) {
        ChannelPlayback& ch = g_playState[i];
        if (!ch.active) continue;

        int idx = ch.resID - 1000;
        if (idx < 0 || idx >= NUM_SOUND_FILES || !g_wavCache[idx].data || g_wavCache[idx].len == 0) {
            ch.active = false;
            continue;
        }

        if (ch.isMusic && !snd->allowMusics) continue;
        if (!ch.isMusic && !snd->allowSounds) continue;

        int volPct = ch.isMusic ? snd->musicVolume : snd->soundFxVolume;
        if (volPct <= 0) continue;
        if (volPct > 100) volPct = 100;
        int sdlVol = (volPct * SDL_MIX_MAXVOLUME) / 100;

        const uint8_t* src = g_wavCache[idx].data;
        uint32_t totalLen = g_wavCache[idx].len;
        int outOffset = 0;

        while (outOffset < len && ch.active) {
            uint32_t avail = totalLen - ch.pos;
            uint32_t toMix = std::min<uint32_t>(static_cast<uint32_t>(len - outOffset), avail);
            if (toMix > 0) {
                SDL_MixAudio(stream + outOffset, src + ch.pos, toMix, sdlVol);
                ch.pos += toMix;
                outOffset += static_cast<int>(toMix);
            }
            if (ch.pos >= totalLen) {
                if (ch.loop) {
                    ch.pos = 0;
                } else {
                    ch.active = false;
                }
            }
        }
    }
}

static int16_t readSample16(const uint8_t* framePtr, int ch, int inChannels, Uint16 fmt) {
    if (ch >= inChannels) ch = 0;
    if (fmt == AUDIO_U8) {
        return static_cast<int16_t>((static_cast<int>(*(framePtr + ch)) - 128) << 8);
    } else if (fmt == AUDIO_S8) {
        return static_cast<int16_t>(static_cast<int>(*(reinterpret_cast<const int8_t*>(framePtr) + ch)) << 8);
    } else {
        const uint8_t* p = framePtr + ch * 2;
        return static_cast<int16_t>((*p) | ((*(p + 1)) << 8));
    }
}

static bool ensureWavLoaded(int16_t resID) {
    int idx = resID - 1000;
    if (idx < 0 || idx >= NUM_SOUND_FILES) return false;
    if (g_wavCache[idx].data != nullptr) return true;
    if (g_wavCache[idx].tried) return false;
    g_wavCache[idx].tried = true;

    CAppContainer* container = CAppContainer::getInstance();
    if (!container || !container->zipFile) return false;

    std::string path = std::string("Payload/Doom2rpg.app/Packages/sounds2/") + *(Sounds::RESOURCE_FILE_NAMES + idx);

    int rawSize = 0;
    uint8_t* rawData = container->zipFile->readZipFileEntry(path.c_str(), &rawSize);
    if (!rawData || rawSize <= 0) {
        if (rawData) std::free(rawData);
        return false;
    }

    SDL_RWops* rw = SDL_RWFromMem(rawData, rawSize);
    SDL_AudioSpec wavSpec;
    Uint8* wavBuf = nullptr;
    Uint32 wavLen = 0;

    if (!rw || !SDL_LoadWAV_RW(rw, 1, &wavSpec, &wavBuf, &wavLen)) {
        std::free(rawData);
        return false;
    }
    std::free(rawData);

    int inBytesPerSample = (wavSpec.format == AUDIO_U8 || wavSpec.format == AUDIO_S8) ? 1 : 2;
    int inFrameBytes = inBytesPerSample * wavSpec.channels;
    if (inFrameBytes <= 0 || wavSpec.freq <= 0) {
        SDL_FreeWAV(wavBuf);
        return false;
    }

    uint32_t inFrames = wavLen / static_cast<uint32_t>(inFrameBytes);
    uint32_t outFrames = static_cast<uint32_t>((static_cast<uint64_t>(inFrames) * g_audioSpec.freq) / wavSpec.freq);
    if (outFrames == 0) {
        SDL_FreeWAV(wavBuf);
        return false;
    }

    uint32_t outBytes = outFrames * static_cast<uint32_t>(g_audioSpec.channels) * sizeof(int16_t);
    int16_t* outBuf = static_cast<int16_t*>(std::malloc(outBytes));
    if (!outBuf) {
        SDL_FreeWAV(wavBuf);
        return false;
    }

    for (uint32_t i = 0; i < outFrames; ++i) {
        uint32_t srcFrame = static_cast<uint32_t>((static_cast<uint64_t>(i) * wavSpec.freq) / g_audioSpec.freq);
        if (srcFrame >= inFrames) srcFrame = inFrames - 1;
        const uint8_t* framePtr = wavBuf + srcFrame * static_cast<uint32_t>(inFrameBytes);
        for (int c = 0; c < g_audioSpec.channels; ++c) {
            *(outBuf + i * g_audioSpec.channels + c) = readSample16(framePtr, c, wavSpec.channels, wavSpec.format);
        }
    }

    SDL_FreeWAV(wavBuf);
    g_wavCache[idx].data = reinterpret_cast<uint8_t*>(outBuf);
    g_wavCache[idx].len = outBytes;
    return true;
}

void Sound::SoundStream::StartFade(int volume, int fadeBeg, int fadeEnd) {
    this->volume = volume;
    this->fadeBeg = fadeBeg;
    this->fadetime = fadeEnd;
    this->fadeInProgress = true;
}

Sound::Sound()
    : app(nullptr), field_0x4(false), allowSounds(true), allowMusics(true),
      soundFxVolume(80), musicVolume(70), field_0x10(0), resID(-1), flags(0),
      priority(0), field_0x15c(0), field_0x160(0), field_0x162(0),
      soundsLoaded(false), alContext(nullptr), alDevice(nullptr) {
    std::memset(this->channel, 0, sizeof(this->channel));
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        (this->channel + i)->resID = -1;
    }
}

Sound::~Sound() {
    this->openAL_Close();
}

bool Sound::startup() {
    this->app = CAppContainer::getInstance()->app;
    this->allowSounds = true;
    this->allowMusics = true;
    if (this->soundFxVolume <= 0) this->soundFxVolume = 80;
    if (this->musicVolume <= 0) this->musicVolume = 70;
    this->openAL_Init();
    this->soundsLoaded = true;
    return true;
}

void Sound::openAL_Init() {
    if (g_audioOpened) return;

    SDL_AudioSpec desired;
    std::memset(&desired, 0, sizeof(desired));
    desired.freq = 22050;
    desired.format = AUDIO_S16SYS;
    desired.channels = 2;
    desired.samples = 512;
    desired.callback = sdlAudioCallback;
    desired.userdata = this;

    if (SDL_OpenAudio(&desired, &g_audioSpec) < 0) {
        std::fprintf(stderr, "SDL_OpenAudio failed: %s\n", SDL_GetError());
        return;
    }

    std::fprintf(stderr, "SDL_OpenAudio OK: freq=%d ch=%d fmt=0x%x\n", g_audioSpec.freq, g_audioSpec.channels, g_audioSpec.format);
    std::memset(g_wavCache, 0, sizeof(g_wavCache));
    std::memset(g_playState, 0, sizeof(g_playState));
    g_audioOpened = true;
    SDL_PauseAudio(0);
}

void Sound::openAL_Close() {
    if (!g_audioOpened) return;
    SDL_LockAudio();
    std::memset(g_playState, 0, sizeof(g_playState));
    SDL_UnlockAudio();
    SDL_CloseAudio();
    g_audioOpened = false;

    for (int i = 0; i < NUM_SOUND_FILES; ++i) {
        if (g_wavCache[i].data) {
            std::free(g_wavCache[i].data);
            g_wavCache[i].data = nullptr;
        }
        g_wavCache[i].len = 0;
        g_wavCache[i].tried = false;
    }
}

void Sound::openAL_SetSystemVolume(int volume) {
    this->soundFxVolume = volume;
}

void Sound::openAL_SetVolume(ALuint, int) {}

void Sound::openAL_Suspend() {
    if (g_audioOpened) SDL_PauseAudio(1);
}

void Sound::openAL_Resume() {
    if (g_audioOpened) SDL_PauseAudio(0);
}

bool Sound::openAL_IsPlaying(ALuint source) {
    if (source >= MAX_CHANNELS) return false;
    return g_playState[source].active;
}

bool Sound::openAL_IsPaused(ALuint) { return false; }
bool Sound::openAL_GetALFormat(AudioStreamBasicDescription, ALenum*) { return false; }
void Sound::openAL_PlaySound(ALuint, ALint) {}
void Sound::openAL_LoadSound(int, Sound::SoundStream*) {}
bool Sound::openAL_LoadWAVFromFile(ALuint, const char*) { return false; }
bool Sound::openAL_LoadAudioFileData(const char*, ALenum*, ALvoid**, ALsizei*, ALsizei*) { return false; }
bool Sound::openAL_OpenAudioFile(const char*, InputStream*) { return false; }

bool Sound::openAL_LoadAllSounds() {
    if (!g_audioOpened) this->openAL_Init();
    this->soundsLoaded = true;
    return true;
}

bool Sound::cacheSounds() {
    if (!g_audioOpened) this->openAL_Init();
    return true;
}

int Sound::getFreeSlot(int prio) {
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        if (!g_playState[i].active) return i;
    }
    int bestSlot = -1;
    int lowestPrio = prio;
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        if (!g_playState[i].loop && g_playState[i].priority <= lowestPrio) {
            lowestPrio = g_playState[i].priority;
            bestSlot = i;
        }
    }
    return bestSlot;
}

void Sound::playSound(int16_t resID, uint8_t flags, int priority, bool) {
    if (!g_audioOpened) this->openAL_Init();
    if (!g_audioOpened) return;

    bool isMusic = isMusicRes(resID) || ((flags & 1) != 0);
    if (isMusic && !this->allowMusics) return;
    if (!isMusic && !this->allowSounds) return;

    if (!ensureWavLoaded(resID)) return;

    SDL_LockAudio();
    if (isMusic) {
        for (int i = 0; i < MAX_CHANNELS; ++i) {
            if (g_playState[i].active && g_playState[i].isMusic) {
                if (g_playState[i].resID == resID) {
                    SDL_UnlockAudio();
                    return;
                }
g_playState[i].active = false;
(this->channel + i)->resID = -1;
}
}
}
int slot = this->getFreeSlot(priority);
if (slot >= 0 && slot < MAX_CHANNELS) {
g_playState[slot].active = true;
g_playState[slot].loop = ((flags & 1) != 0);
g_playState[slot].isMusic = isMusic;
g_playState[slot].resID = resID;
g_playState[slot].priority = priority;
g_playState[slot].pos = 0;
(this->channel + slot)->resID = resID;
(this->channel + slot)->priority = static_cast<int16_t>(priority);
}
SDL_UnlockAudio();
}
void Sound::soundStop() {
if (!g_audioOpened) return;
SDL_LockAudio();
for (int i = 0; i < MAX_CHANNELS; ++i) {
g_playState[i].active = false;
(this->channel + i)->resID = -1;
}
SDL_UnlockAudio();
}
void Sound::stopSound(int resID, bool) {
if (!g_audioOpened) return;
SDL_LockAudio();
for (int i = 0; i < MAX_CHANNELS; ++i) {
if (g_playState[i].active && g_playState[i].resID == resID) {
g_playState[i].active = false;
(this->channel + i)->resID = -1;
}
}
SDL_UnlockAudio();
}
bool Sound::isSoundPlaying(int16_t resID) {
if (!g_audioOpened) return false;
SDL_LockAudio();
bool playing = false;
for (int i = 0; i < MAX_CHANNELS; ++i) {
if (g_playState[i].active && g_playState[i].resID == resID) {
playing = true;
break;
}
}
SDL_UnlockAudio();
return playing;
}
void Sound::updateVolume() {}
void Sound::playCombatSound(int16_t resID, uint8_t flags, int priority) {
this->playSound(resID, flags, priority, false);
}
bool Sound::cacheCombatSound(int resID) {
if (!g_audioOpened) this->openAL_Init();
return ensureWavLoaded(static_cast<int16_t>(resID));
}
void Sound::freeMonsterSounds() {}
void Sound::volumeUp(int volume) {
this->soundFxVolume = std::min(100, this->soundFxVolume + volume);
this->allowSounds = (this->soundFxVolume > 0);
}
void Sound::volumeDown(int volume) {
this->soundFxVolume = std::max(0, this->soundFxVolume - volume);
if (this->soundFxVolume == 0) this->allowSounds = false;
}
void Sound::musicVolumeUp(int volume) {
this->musicVolume = std::min(100, this->musicVolume + volume);
this->allowMusics = (this->musicVolume > 0);
}
void Sound::musicVolumeDown(int volume) {
this->musicVolume = std::max(0, this->musicVolume - volume);
if (this->musicVolume == 0) this->allowMusics = false;
}
void Sound::startFrame() {}
void Sound::endFrame() {}
