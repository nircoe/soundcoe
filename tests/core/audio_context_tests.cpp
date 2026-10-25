#include <soundcoe/core/audio_context.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <future>
#include <thread>
#include <vector>

using namespace soundcoe;
using namespace soundcoe::internal;

//==============================================================================
//       AudioContextTests - audio_context singleton and initialization tests
//==============================================================================

class AudioContextTests : public ::testing::Test
{
protected:
    audio_context m_audio_context;

    void SetUp() override
    {
        ASSERT_TRUE(m_audio_context.initialize());
    }

    void TearDown() override
    {
        static_cast<void>(m_audio_context.shutdown());
    }
};

TEST_F(AudioContextTests, InitializationAndShutdown)
{
    // Test 1: device and context exist after initialize
    {
        EXPECT_TRUE(m_audio_context.is_initialized());

        EXPECT_NE(m_audio_context.get_device(), nullptr);
        EXPECT_NE(m_audio_context.get_context(), nullptr);
    }

    // Test 2: reads from several threads stay valid
    {
        const int num_threads = 4;
        std::vector<std::future<bool>> futures;

        for (int i = 0; i < num_threads; ++i)
        {
            futures.push_back(std::async(std::launch::async, [&]() {
                bool all_succeeded = true;

                for (int j = 0; j < 10; ++j)
                {
                    all_succeeded &= m_audio_context.is_initialized();
                    all_succeeded &= (m_audio_context.get_device() != nullptr);
                    all_succeeded &= (m_audio_context.get_context() != nullptr);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                return all_succeeded;
            }));
        }

        for (auto& future : futures)
        {
            EXPECT_TRUE(future.get());
        }
    }

    // Test 3: shutdown can be called repeatedly
    {
        EXPECT_TRUE(m_audio_context.shutdown());
        EXPECT_TRUE(m_audio_context.shutdown());
        EXPECT_TRUE(m_audio_context.shutdown());

        EXPECT_FALSE(m_audio_context.is_initialized());
    }
}
