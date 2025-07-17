#include <soundcoe.hpp>

namespace soundcoe
{
    namespace detail
    {
        SoundManager& getSoundManagerInstance()
        {
            static SoundManager s_soundManager;
            return s_soundManager;
        }
    }
}