#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <cassert>
#include "consts.h"


namespace engine {
    class AudioManager final {
    public:
        static void Initialize();

        static void SetBGM(Mix_Music *music);
        static void PlayBGM();
        static void ClearBGM();
        static void PauseBGM();
        static void StopBGM();
        static void ResumeBGM();
        static void PlaySound(Mix_Chunk *sound);
        static bool HasBGM();

        static void Finalize();
    private:
        static Mix_Music *currentBGM;
    };
}
