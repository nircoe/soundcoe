#include <soundcoe/core/error.hpp>
#include <soundcoe/core/types.hpp>
#include <soundcoe/resources/resource_manager.hpp>
#include <soundcoe/resources/sound_source.hpp>
#include <utils/test_audio_files.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <string>
#include <thread>
#include <vector>

using namespace soundcoe;
using namespace soundcoe::internal;

//==============================================================================
//         ResourceManagerTests - resource_manager functionality tests
//==============================================================================

class ResourceManagerTests : public ::testing::Test
{
protected:
    resource_manager m_resource_manager;

    void SetUp() override
    {
        test_audio_files::create_test_files();

        ASSERT_TRUE(m_resource_manager.initialize(test_audio_files::s_test_root_dir.string(), 4, 2));
    }

    void TearDown() override
    {
        m_resource_manager.shutdown();
        test_audio_files::cleanup();
    }

    static void SetUpTestSuite()
    {
        test_audio_files::create_test_files();
    }

    static void TearDownTestSuite()
    {
        test_audio_files::cleanup();
    }
};

//==============================================================================
//                                  Lifecycle
//==============================================================================

TEST_F(ResourceManagerTests, Lifecycle)
{
    // Test 1: state right after initialize
    {
        EXPECT_EQ(m_resource_manager.get_active_source_count(), 0);
        EXPECT_EQ(m_resource_manager.get_total_source_count(), 4);
        EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 0);
        EXPECT_EQ(m_resource_manager.get_cache_size_bytes(), 0);
        EXPECT_TRUE(m_resource_manager.get_loaded_directories().empty());
    }

    // Test 2: operations fail after shutdown
    {
        ASSERT_TRUE(m_resource_manager.preload_directory("sounds"));
        std::size_t pool_index;
        [[maybe_unused]] auto source = m_resource_manager.acquire_source(pool_index, sound_priority::medium);

        EXPECT_NO_THROW(m_resource_manager.shutdown());
        EXPECT_FALSE(m_resource_manager.is_initialized());

        std::size_t new_index;
        auto new_source = m_resource_manager.acquire_source(new_index, sound_priority::medium);
        ASSERT_FALSE(new_source);
        EXPECT_EQ(new_source.error().code, error_code::not_initialized);

        auto buffer = m_resource_manager.get_buffer("sounds/test1.wav");
        ASSERT_FALSE(buffer);
        EXPECT_EQ(buffer.error().code, error_code::not_initialized);
    }

    // Test 3: shutdown can be called repeatedly
    {
        EXPECT_NO_THROW(m_resource_manager.shutdown());
        EXPECT_NO_THROW(m_resource_manager.shutdown());
        EXPECT_FALSE(m_resource_manager.is_initialized());
    }
}

//==============================================================================
//                                 Source Pool
//==============================================================================

TEST_F(ResourceManagerTests, SourcePool)
{
    // Test 1: acquire and release one source
    {
        std::size_t pool_index;
        auto source_opt = m_resource_manager.acquire_source(pool_index, sound_priority::medium);
        EXPECT_TRUE(source_opt.has_value());
        EXPECT_EQ(m_resource_manager.get_active_source_count(), 1);

        if (source_opt.has_value())
        {
            m_resource_manager.release_source(source_opt.value());
            EXPECT_EQ(m_resource_manager.get_active_source_count(), 0);
        }
    }

    // Test 2: high priority replaces a low one in a full pool
    {
        std::vector<std::reference_wrapper<sound_source>> sources;

        for (int i = 0; i < 4; ++i)
        {
            std::size_t pool_index;
            auto source_opt = m_resource_manager.acquire_source(pool_index, sound_priority::low);
            EXPECT_TRUE(source_opt.has_value());
            if (source_opt.has_value())
                sources.push_back(source_opt.value());
        }

        EXPECT_EQ(m_resource_manager.get_active_source_count(), 4);

        std::size_t high_pool_index;
        auto high_priority_opt = m_resource_manager.acquire_source(high_pool_index, sound_priority::high);
        EXPECT_TRUE(high_priority_opt.has_value());
        EXPECT_EQ(m_resource_manager.get_active_source_count(), 4);

        if (high_priority_opt.has_value())
            m_resource_manager.release_source(high_priority_opt.value());

        for (auto &source : sources)
            m_resource_manager.release_source(source);
    }

    // Test 3: critical priority replaces a low one
    {
        std::vector<std::reference_wrapper<sound_source>> low_sources;

        for (int i = 0; i < 4; ++i)
        {
            std::size_t pool_index;
            auto source_opt = m_resource_manager.acquire_source(pool_index, sound_priority::low);
            if (source_opt.has_value())
                low_sources.push_back(source_opt.value());
        }

        std::size_t critical_index;
        auto critical_opt = m_resource_manager.acquire_source(critical_index, sound_priority::critical);
        EXPECT_TRUE(critical_opt.has_value());

        if (critical_opt.has_value())
            m_resource_manager.release_source(critical_opt.value());

        for (auto &source : low_sources)
            m_resource_manager.release_source(source);
    }

    // Test 4: low priority fails when the pool is full of high
    {
        std::vector<std::reference_wrapper<sound_source>> sources;
        for (int i = 0; i < 4; ++i)
        {
            std::size_t pool_index;
            auto source = m_resource_manager.acquire_source(pool_index, sound_priority::high);
            ASSERT_TRUE(source);
            sources.push_back(source.value());
        }

        std::size_t low_index;
        auto low_priority = m_resource_manager.acquire_source(low_index, sound_priority::low);
        ASSERT_FALSE(low_priority);
        EXPECT_EQ(low_priority.error().code, error_code::resource_exhausted);
        EXPECT_EQ(m_resource_manager.get_active_source_count(), 4);

        for (auto &source : sources)
            m_resource_manager.release_source(source);
    }
}

//==============================================================================
//                                Buffer Caching
//==============================================================================

TEST_F(ResourceManagerTests, BufferCaching)
{
    // Test 1: buffers are cached and shared
    {
        EXPECT_TRUE(m_resource_manager.preload_directory("sounds"));

        auto buffer_opt1 = m_resource_manager.get_buffer("test1.wav");
        EXPECT_TRUE(buffer_opt1.has_value());
        EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 2); // test1.wav and test2.wav from directory
        EXPECT_GT(m_resource_manager.get_cache_size_bytes(), 0);

        auto buffer_opt2 = m_resource_manager.get_buffer("test1.wav");
        EXPECT_TRUE(buffer_opt2.has_value());

        if (buffer_opt1.has_value() && buffer_opt2.has_value())
        {
            EXPECT_EQ(&buffer_opt1.value().get(), &buffer_opt2.value().get());
        }

        EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 2);

        m_resource_manager.release_buffer("test1.wav");
        m_resource_manager.release_buffer("test1.wav");
    }

    // Test 2: reference counts and cleanup
    {
        EXPECT_TRUE(m_resource_manager.preload_directory("sounds"));

        const std::string filename = "test1.wav";

        auto buffer1 = m_resource_manager.get_buffer(filename);
        auto buffer2 = m_resource_manager.get_buffer(filename);
        auto buffer3 = m_resource_manager.get_buffer(filename);

        EXPECT_TRUE(buffer1.has_value());
        EXPECT_TRUE(buffer2.has_value());
        EXPECT_TRUE(buffer3.has_value());
        EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 2); // test1.wav and test2.wav from directory

        m_resource_manager.release_buffer(filename);
        m_resource_manager.release_buffer(filename);
        EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 2);

        m_resource_manager.release_buffer(filename);
        buffer1 = m_resource_manager.get_buffer(filename);
        EXPECT_TRUE(buffer1.has_value());
        m_resource_manager.release_buffer(filename);

        EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 2);
        std::size_t cleaned = m_resource_manager.cleanup_unused_buffers();
        EXPECT_EQ(cleaned, 2); // Both files should be cleaned (both have ref count 0)
        EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 0);
    }

    // Test 3: cache size stays under the limit
    {
        m_resource_manager.shutdown();
        ASSERT_TRUE(m_resource_manager.initialize(test_audio_files::s_test_root_dir.string(), 4, 1));

        ASSERT_TRUE(m_resource_manager.preload_directory("sounds"));
        ASSERT_TRUE(m_resource_manager.preload_directory("music"));

        std::vector<std::string> files = {"test1.wav", "test2.wav", "music1.wav"};

        for (const auto &file : files)
        {
            auto buffer = m_resource_manager.get_buffer(file);
            if (buffer.has_value())
                m_resource_manager.release_buffer(file);
        }

        EXPECT_GT(m_resource_manager.get_cached_buffer_count(), 0);
        EXPECT_LE(m_resource_manager.get_cache_size_bytes(), 1 * 1024 * 1024);
    }

    // Test 4: a buffer bigger than the limit is refused
    {
        m_resource_manager.shutdown();
        ASSERT_TRUE(m_resource_manager.initialize(test_audio_files::s_test_root_dir.string(), 4, 0));
        ASSERT_TRUE(m_resource_manager.preload_directory("sounds"));
        EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 0);

        auto buffer = m_resource_manager.get_buffer("test1.wav");
        ASSERT_FALSE(buffer);
        EXPECT_EQ(buffer.error().code, error_code::resource_exhausted);
        EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 0);
        EXPECT_EQ(m_resource_manager.get_cache_size_bytes(), 0);
    }
}

//==============================================================================
//                                 Directories
//==============================================================================

TEST_F(ResourceManagerTests, Directories)
{
    // Test 1: preload and unload a directory
    {
        EXPECT_TRUE(m_resource_manager.preload_directory("sounds"));
        EXPECT_TRUE(m_resource_manager.is_directory_loaded("sounds"));
        EXPECT_GT(m_resource_manager.get_cached_buffer_count(), 0);

        auto dirs = m_resource_manager.get_loaded_directories();
        EXPECT_FALSE(dirs.empty());

        std::size_t buffers_before = m_resource_manager.get_cached_buffer_count();
        EXPECT_TRUE(m_resource_manager.unload_directory("sounds"));
        EXPECT_FALSE(m_resource_manager.is_directory_loaded("sounds"));
        EXPECT_LT(m_resource_manager.get_cached_buffer_count(), buffers_before);
    }

    // Test 2: missing and empty directory names
    {
        auto missing = m_resource_manager.preload_directory("nonexistent");
        ASSERT_FALSE(missing);
        EXPECT_EQ(missing.error().code, error_code::directory_not_found);
        EXPECT_FALSE(m_resource_manager.is_directory_loaded("nonexistent"));

        auto empty = m_resource_manager.preload_directory("");
        ASSERT_FALSE(empty);
        EXPECT_EQ(empty.error().code, error_code::invalid_argument);

        EXPECT_TRUE(m_resource_manager.unload_directory("notloaded"));
    }

    // Test 3: files load only through a loaded directory
    {
        EXPECT_TRUE(m_resource_manager.preload_directory("sounds"));
        EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 2); // test1.wav and test2.wav

        auto buffer = m_resource_manager.get_buffer("test1.wav");
        EXPECT_TRUE(buffer.has_value());

        m_resource_manager.release_buffer("test1.wav");
        EXPECT_TRUE(m_resource_manager.unload_directory("sounds"));

        // Test that files can only be loaded through directories
        auto buffer_after_unload = m_resource_manager.get_buffer("test1.wav");
        EXPECT_FALSE(buffer_after_unload.has_value());
    }
}

//==============================================================================
//                                 Concurrency
//==============================================================================

TEST_F(ResourceManagerTests, Concurrency)
{
    // Test 1: sources acquired from several threads
    {
        const int num_threads = 3;
        std::vector<sound_priority> priorities = {sound_priority::low, sound_priority::medium, sound_priority::high};
        std::vector<std::future<std::vector<std::reference_wrapper<sound_source>>>> futures;

        for (int i = 0; i < num_threads; ++i)
        {
            auto priority = priorities[i];
            auto future = std::async(std::launch::async, [&, priority]()
                                     {
                std::vector<std::reference_wrapper<sound_source>> sources;
                for (int j = 0; j < 2; ++j)
                {
                    std::size_t pool_index;
                    auto source_opt = m_resource_manager.acquire_source(pool_index, priority);
                    if (source_opt.has_value())
                        sources.push_back(source_opt.value());
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                return sources; });
            futures.push_back(std::move(future));
        }

        std::vector<std::reference_wrapper<sound_source>> all_sources;
        for (auto &future : futures)
        {
            auto sources = future.get();
            all_sources.insert(all_sources.end(), sources.begin(), sources.end());
        }

        EXPECT_LE(all_sources.size(), 6);
        EXPECT_GE(all_sources.size(), 4);
        EXPECT_EQ(m_resource_manager.get_active_source_count(), 4);

        for (auto &source : all_sources)
            m_resource_manager.release_source(source);

        EXPECT_EQ(m_resource_manager.get_active_source_count(), 0);
    }

    // Test 2: buffers requested from several threads
    {
        ASSERT_TRUE(m_resource_manager.preload_directory("sounds"));
        ASSERT_TRUE(m_resource_manager.preload_directory("music"));

        std::vector<std::future<void>> futures;
        std::vector<std::string> files = {"test1.wav", "test2.wav", "music1.wav"};

        for (int i = 0; i < 3; ++i)
        {
            auto future = std::async(std::launch::async, [&, i]()
                                     {
                const std::string& filename = files[i];
                for (int j = 0; j < 3; ++j)
                {
                    auto buffer = m_resource_manager.get_buffer(filename);
                    if (buffer.has_value())
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        m_resource_manager.release_buffer(filename);
                    }
                } });
            futures.push_back(std::move(future));
        }

        for (auto &future : futures)
            future.wait();

        EXPECT_GT(m_resource_manager.get_cached_buffer_count(), 0);
    }
}

//==============================================================================
//                              Complete Workflow
//==============================================================================

TEST_F(ResourceManagerTests, CompleteWorkflow)
{
    EXPECT_TRUE(m_resource_manager.preload_directory("sounds"));
    EXPECT_GT(m_resource_manager.get_cached_buffer_count(), 0);

    std::size_t pool_index1, pool_index2;
    auto source1 = m_resource_manager.acquire_source(pool_index1, sound_priority::medium);
    auto source2 = m_resource_manager.acquire_source(pool_index2, sound_priority::medium);
    auto buffer1 = m_resource_manager.get_buffer("test1.wav");
    auto buffer2 = m_resource_manager.get_buffer("test2.wav");

    EXPECT_TRUE(source1.has_value());
    EXPECT_TRUE(source2.has_value());
    EXPECT_TRUE(buffer1.has_value());
    EXPECT_TRUE(buffer2.has_value());
    EXPECT_EQ(m_resource_manager.get_active_source_count(), 2);

    std::size_t high_index;
    auto high_priority = m_resource_manager.acquire_source(high_index, sound_priority::high);
    EXPECT_TRUE(high_priority.has_value());

    if (source1.has_value())
        m_resource_manager.release_source(source1.value());
    if (source2.has_value())
        m_resource_manager.release_source(source2.value());
    if (high_priority.has_value())
        m_resource_manager.release_source(high_priority.value());

    m_resource_manager.release_buffer("sounds/test1.wav");
    m_resource_manager.release_buffer("sounds/test2.wav");

    EXPECT_EQ(m_resource_manager.get_active_source_count(), 0);
}

//==============================================================================
//                               Invalid Buffers
//==============================================================================

TEST_F(ResourceManagerTests, InvalidBuffers)
{
    // Test 1: corrupt file is refused
    {
        std::filesystem::path corrupt_file = test_audio_files::s_test_sub_dir1 / "corrupt.wav";
        std::ofstream corrupt(corrupt_file, std::ios::binary);
        corrupt << "Invalid WAV data";
        corrupt.close();

        ASSERT_TRUE(m_resource_manager.preload_directory("sounds"));

        auto corrupt_buffer = m_resource_manager.get_buffer("corrupt.wav");
        ASSERT_FALSE(corrupt_buffer);
        EXPECT_EQ(corrupt_buffer.error().code, error_code::unsupported_format);

        std::filesystem::remove(corrupt_file);
    }

    // Test 2: missing, non audio and empty names
    {
        auto non_existent = m_resource_manager.get_buffer("nonexistent.wav");
        ASSERT_FALSE(non_existent);
        EXPECT_EQ(non_existent.error().code, error_code::file_not_found);

        ASSERT_TRUE(m_resource_manager.preload_directory("sounds"));
        auto non_audio = m_resource_manager.get_buffer("readme.txt");
        ASSERT_FALSE(non_audio);
        EXPECT_EQ(non_audio.error().code, error_code::unsupported_format);

        auto empty = m_resource_manager.get_buffer("");
        ASSERT_FALSE(empty);
        EXPECT_EQ(empty.error().code, error_code::invalid_argument);
    }
}

//==============================================================================
//                             Uninitialized Errors
//==============================================================================

TEST_F(ResourceManagerTests, UninitializedErrors)
{
    resource_manager uninitialized;

    // Test 1: an empty root directory is rejected and nothing gets initialized
    {
        auto r = uninitialized.initialize("");
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::invalid_argument);
        EXPECT_FALSE(uninitialized.is_initialized());
    }

    // Test 2: directory, source and buffer operations need an initialized manager
    {
        auto preload = uninitialized.preload_directory("sounds");
        ASSERT_FALSE(preload);
        EXPECT_EQ(preload.error().code, error_code::not_initialized);

        auto unload = uninitialized.unload_directory("sounds");
        ASSERT_FALSE(unload);
        EXPECT_EQ(unload.error().code, error_code::not_initialized);

        std::size_t pool_index;
        auto source = uninitialized.acquire_source(pool_index);
        ASSERT_FALSE(source);
        EXPECT_EQ(source.error().code, error_code::not_initialized);

        auto buffer = uninitialized.get_buffer("test1.wav");
        ASSERT_FALSE(buffer);
        EXPECT_EQ(buffer.error().code, error_code::not_initialized);
    }

    // Test 3: initialize on an already initialized manager is a success
    {
        EXPECT_TRUE(m_resource_manager.initialize(test_audio_files::s_test_root_dir.string()));
        EXPECT_TRUE(m_resource_manager.is_initialized());
    }
}
