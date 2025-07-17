#include <soundcoe/playback/sound_manager.hpp>
#include <logcoe.hpp>

namespace soundcoe
{
    namespace detail
    {
        SoundManager& getSoundManagerInstance()
        {
            static SoundManager s_soundManager;
            logcoe::info("getSoundManagerInstance() called, returning reference to static instance");
            return s_soundManager;
        }
    }
}