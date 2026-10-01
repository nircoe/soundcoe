#include <gtest/gtest.h>
#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/resources/resource_manager.hpp>
#include <soundcoe/core/types.hpp>
#include <soundcoe/core/error.hpp>
#include "utils/test_audio_files.hpp"
#include <thread>
#include <chrono>
#include <vector>
#include <future>
#include <expected>

using namespace soundcoe;
using namespace soundcoe::internal;

class ResourceManagerTests : public ::testing::Test
{
protected:
    resource_manager m_resource_manager;
    
    void SetUp() override
    {
        test_audio_files::create_test_files();

        m_resource_manager.initialize(test_audio_files::s_test_root_dir.string(), 4, 2);
    }

    void TearDown() override
    {
        try { m_resource_manager.shutdown(); }
        catch (...) { }
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

TEST_F(ResourceManagerTests, InitialState)
{
    EXPECT_EQ(m_resource_manager.get_active_source_count(), 0);
    EXPECT_EQ(m_resource_manager.get_total_source_count(), 4);
    EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 0);
    EXPECT_EQ(m_resource_manager.get_cache_size_bytes(), 0);
    EXPECT_TRUE(m_resource_manager.get_loaded_directories().empty());
}

TEST_F(ResourceManagerTests, MultipleShutdown)
{
    EXPECT_NO_THROW(m_resource_manager.shutdown());
    EXPECT_NO_THROW(m_resource_manager.shutdown());
    EXPECT_FALSE(m_resource_manager.is_initialized());
}

TEST_F(ResourceManagerTests, SourceAcquisitionAndRelease)
{
    size_t pool_index;
    auto source_opt = m_resource_manager.acquire_source(pool_index, sound_priority::medium);
    EXPECT_TRUE(source_opt.has_value());
    EXPECT_EQ(m_resource_manager.get_active_source_count(), 1);

    if (source_opt.has_value())
    {
        EXPECT_TRUE(m_resource_manager.release_source(source_opt.value()));
        EXPECT_EQ(m_resource_manager.get_active_source_count(), 0);
    }
}

TEST_F(ResourceManagerTests, SourcePoolExhaustion)
{
    std::vector<std::reference_wrapper<sound_source>> sources;

    for(int i = 0; i < 4; ++i)
    {
        size_t pool_index;
        auto source_opt = m_resource_manager.acquire_source(pool_index, sound_priority::low);
        EXPECT_TRUE(source_opt.has_value());
        if(source_opt.has_value())
            sources.push_back(source_opt.value());
    }

    EXPECT_EQ(m_resource_manager.get_active_source_count(), 4);

    size_t high_pool_index;
    auto high_priority_opt = m_resource_manager.acquire_source(high_pool_index, sound_priority::high);
    EXPECT_TRUE(high_priority_opt.has_value());
    EXPECT_EQ(m_resource_manager.get_active_source_count(), 4);

    if(high_priority_opt.has_value())
        m_resource_manager.release_source(high_priority_opt.value());

    for(auto &source : sources)
        m_resource_manager.release_source(source);
}

TEST_F(ResourceManagerTests, PriorityReplacement)
{
    std::vector<std::reference_wrapper<sound_source>> low_sources;

    for(int i = 0; i < 4; ++i)
    {
        size_t pool_index;
        auto source_opt = m_resource_manager.acquire_source(pool_index, sound_priority::low);
        if(source_opt.has_value())
            low_sources.push_back(source_opt.value());
    }

    size_t critical_index;
    auto critical_opt = m_resource_manager.acquire_source(critical_index, sound_priority::critical);
    EXPECT_TRUE(critical_opt.has_value());

    if(critical_opt.has_value())
        m_resource_manager.release_source(critical_opt.value());

    for(auto &source : low_sources)
        m_resource_manager.release_source(source);
}

TEST_F(ResourceManagerTests, BufferLoadingAndCaching)
{
    // First preload the directory to make files available
    EXPECT_TRUE(m_resource_manager.preload_directory("sounds"));

    auto buffer_opt1 = m_resource_manager.get_buffer("test1.wav");
    EXPECT_TRUE(buffer_opt1.has_value());
    EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 2); // test1.wav and test2.wav from directory
    EXPECT_GT(m_resource_manager.get_cache_size_bytes(), 0);

    auto buffer_opt2 = m_resource_manager.get_buffer("test1.wav");
    EXPECT_TRUE(buffer_opt2.has_value());

    if(buffer_opt1.has_value() && buffer_opt2.has_value())
    {
        EXPECT_EQ(&buffer_opt1.value().get(), &buffer_opt2.value().get());
    }

    EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 2); // Still 2 total files

    EXPECT_TRUE(m_resource_manager.release_buffer("test1.wav"));
    EXPECT_TRUE(m_resource_manager.release_buffer("test1.wav"));
}

TEST_F(ResourceManagerTests, BufferReferenceCounting)
{
    // First preload the directory to make files available
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
    EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 2); // Still 2 total files

    m_resource_manager.release_buffer(filename);
    buffer1 = m_resource_manager.get_buffer(filename);
    EXPECT_TRUE(buffer1.has_value());
    m_resource_manager.release_buffer(filename);

    // Clear all reference_wrapper optionals to ensure no references remain
    buffer1.reset();
    buffer2.reset();
    buffer3.reset();

    EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 2); // Still 2 files in cache
    size_t cleaned = m_resource_manager.cleanup_unused_buffers();
    EXPECT_EQ(cleaned, 2); // Both files should be cleaned (both have ref count 0)
    EXPECT_EQ(m_resource_manager.get_cached_buffer_count(), 0);
}

TEST_F(ResourceManagerTests, InvalidBufferRequests)
{
    auto non_existent = m_resource_manager.get_buffer("nonexistent.wav");
    EXPECT_FALSE(non_existent.has_value());

    auto non_audio = m_resource_manager.get_buffer("sounds/readme.txt");
    EXPECT_FALSE(non_audio.has_value());

    auto empty = m_resource_manager.get_buffer("");
    EXPECT_FALSE(empty.has_value());
}

TEST_F(ResourceManagerTests, DirectoryOperations)
{
    EXPECT_TRUE(m_resource_manager.preload_directory("sounds"));
    EXPECT_TRUE(m_resource_manager.is_directory_loaded("sounds"));
    EXPECT_GT(m_resource_manager.get_cached_buffer_count(), 0);

    auto dirs = m_resource_manager.get_loaded_directories();
    EXPECT_FALSE(dirs.empty());

    size_t buffers_before = m_resource_manager.get_cached_buffer_count();
    EXPECT_TRUE(m_resource_manager.unload_directory("sounds"));
    EXPECT_FALSE(m_resource_manager.is_directory_loaded("sounds"));
    EXPECT_LT(m_resource_manager.get_cached_buffer_count(), buffers_before);
}

TEST_F(ResourceManagerTests, InvalidDirectoryOperations)
{
    EXPECT_FALSE(m_resource_manager.preload_directory("nonexistent"));
    EXPECT_FALSE(m_resource_manager.is_directory_loaded("nonexistent"));
    EXPECT_FALSE(m_resource_manager.preload_directory(""));
    EXPECT_TRUE(m_resource_manager.unload_directory("notloaded"));
}

TEST_F(ResourceManagerTests, FileLoadingThroughDirectory)
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

TEST_F(ResourceManagerTests, ConcurrentSourceAccess)
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
                size_t pool_index;
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

TEST_F(ResourceManagerTests, ConcurrentBufferAccess)
{
    m_resource_manager.preload_directory("sounds");
    m_resource_manager.preload_directory("music");

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

TEST_F(ResourceManagerTests, CompleteWorkflow)
{
    EXPECT_TRUE(m_resource_manager.preload_directory("sounds"));
    EXPECT_GT(m_resource_manager.get_cached_buffer_count(), 0);

    size_t pool_index1, pool_index2;
    auto source1 = m_resource_manager.acquire_source(pool_index1, sound_priority::medium);
    auto source2 = m_resource_manager.acquire_source(pool_index2, sound_priority::medium);
    auto buffer1 = m_resource_manager.get_buffer("test1.wav");
    auto buffer2 = m_resource_manager.get_buffer("test2.wav");

    EXPECT_TRUE(source1.has_value());
    EXPECT_TRUE(source2.has_value());
    EXPECT_TRUE(buffer1.has_value());
    EXPECT_TRUE(buffer2.has_value());
    EXPECT_EQ(m_resource_manager.get_active_source_count(), 2);

    size_t high_index;
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

TEST_F(ResourceManagerTests, ErrorConditions)
{
    std::filesystem::path corrupt_file = test_audio_files::s_test_sub_dir1 / "corrupt.wav";
    std::ofstream corrupt(corrupt_file, std::ios::binary);
    corrupt << "Invalid WAV data";
    corrupt.close();

    auto corrupt_buffer = m_resource_manager.get_buffer("sounds/corrupt.wav");
    EXPECT_FALSE(corrupt_buffer.has_value());

    std::filesystem::remove(corrupt_file);
}

TEST_F(ResourceManagerTests, CacheLimits)
{
    m_resource_manager.shutdown();
    m_resource_manager.initialize(test_audio_files::s_test_root_dir.string(), 4, 1);

    m_resource_manager.preload_directory("sounds");
    m_resource_manager.preload_directory("music");

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

TEST_F(ResourceManagerTests, ProperShutdown)
{
    m_resource_manager.preload_directory("sounds");
    size_t pool_index;
    [[maybe_unused]] auto source = m_resource_manager.acquire_source(pool_index, sound_priority::medium);

    EXPECT_NO_THROW(m_resource_manager.shutdown());
    EXPECT_FALSE(m_resource_manager.is_initialized());

    size_t new_index;
    auto new_source = m_resource_manager.acquire_source(new_index, sound_priority::medium);
    EXPECT_FALSE(new_source.has_value());

    auto buffer = m_resource_manager.get_buffer("sounds/test1.wav");
    EXPECT_FALSE(buffer.has_value());
}

class SoundBufferTests : public ::testing::Test
{
private:
    audio_context m_audio_context;

protected:
    void SetUp() override
    {
        test_audio_files::create_test_files();
        m_audio_context.initialize();
    }

    void TearDown() override
    {
        try { m_audio_context.shutdown(); }
        catch(...) {}
    }

    static void SetUpTestSuite() { test_audio_files::create_test_files(); }
    static void TearDownTestSuite() { test_audio_files::cleanup(); }
};

TEST_F(SoundBufferTests, DefaultConstruction)
{
    sound_buffer buffer;
    EXPECT_FALSE(buffer.is_loaded());
    EXPECT_EQ(buffer.get_buffer_id(), 0);
    EXPECT_EQ(buffer.get_filename(), "");
}

TEST_F(SoundBufferTests, FileConstructionAndLoading)
{
    std::string filename = (test_audio_files::s_test_sub_dir1 / "test1.wav").string();
    sound_buffer buffer(filename);

    EXPECT_TRUE(buffer.is_loaded());
    EXPECT_NE(buffer.get_buffer_id(), 0);
    EXPECT_GT(buffer.get_duration(), 0.0f);
    EXPECT_EQ(buffer.get_filename(), filename);
}

TEST_F(SoundBufferTests, MoveSemantics)
{
    std::string filename = (test_audio_files::s_test_sub_dir1 / "test1.wav").string();
    sound_buffer buffer1(filename);
    ALuint original_id = buffer1.get_buffer_id();

    sound_buffer buffer2 = std::move(buffer1);
    EXPECT_EQ(buffer2.get_buffer_id(), original_id);
    EXPECT_FALSE(buffer1.is_loaded());
    EXPECT_EQ(buffer1.get_buffer_id(), 0);
}

TEST_F(SoundBufferTests, LoadAndUnload)
{
    sound_buffer buffer;
    std::string filename = (test_audio_files::s_test_sub_dir1 / "test1.wav").string();

    buffer.load_from_file(filename);
    EXPECT_TRUE(buffer.is_loaded());
    EXPECT_NE(buffer.get_buffer_id(), 0);

    buffer.unload();
    EXPECT_FALSE(buffer.is_loaded());
    EXPECT_EQ(buffer.get_buffer_id(), 0);
}

TEST_F(SoundBufferTests, InvalidFileHandling)
{
    sound_buffer buffer;
    EXPECT_THROW(buffer.load_from_file("nonexistent.wav"), std::runtime_error);
    EXPECT_FALSE(buffer.is_loaded());

    std::string txt_file = (test_audio_files::s_test_sub_dir1 / "readme.txt").string();
    EXPECT_THROW(buffer.load_from_file(txt_file), std::runtime_error);
    EXPECT_FALSE(buffer.is_loaded());
}

class SoundSourceTests : public ::testing::Test
{
private:
    audio_context m_audio_context;

protected:
    void SetUp() override
    {
        test_audio_files::create_test_files();
        m_audio_context.initialize();
    }

    void TearDown() override
    {
        try { m_audio_context.shutdown(); }
        catch(...) {}
    }

    static void SetUpTestSuite() { test_audio_files::create_test_files(); }
    static void TearDownTestSuite() { test_audio_files::cleanup(); }
};

TEST_F(SoundSourceTests, DefaultConstruction)
{
    sound_source source;
    EXPECT_FALSE(source.is_created());
    EXPECT_EQ(source.get_source_id(), 0);
    EXPECT_EQ(source.get_volume(), 1.0f);
    EXPECT_EQ(source.get_pitch(), 1.0f);
    EXPECT_FALSE(source.is_looping());
}

TEST_F(SoundSourceTests, PropertySettersAndGetters)
{
    sound_source source;
    ASSERT_TRUE(source.create());

    EXPECT_TRUE(source.set_volume(0.5f));
    EXPECT_FLOAT_EQ(source.get_volume(), 0.5f);

    EXPECT_TRUE(source.set_pitch(1.5f));
    EXPECT_FLOAT_EQ(source.get_pitch(), 1.5f);

    vec3 pos(1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(source.set_position(pos));
    EXPECT_EQ(source.get_position().x, pos.x);
    EXPECT_EQ(source.get_position().y, pos.y);
    EXPECT_EQ(source.get_position().z, pos.z);

    EXPECT_TRUE(source.set_looping(true));
    EXPECT_TRUE(source.is_looping());
}

TEST_F(SoundSourceTests, BufferAttachmentAndPlayback)
{
    std::string filename = (test_audio_files::s_test_sub_dir1 / "test1.wav").string();
    sound_buffer buffer(filename);
    sound_source source;

    EXPECT_TRUE(source.attach_buffer(buffer));
    EXPECT_EQ(source.get_buffer_id(), buffer.get_buffer_id());

    EXPECT_TRUE(source.play());
    EXPECT_TRUE(source.pause());
    EXPECT_TRUE(source.stop());
    EXPECT_TRUE(source.is_stopped());

    EXPECT_TRUE(source.detach_buffer());
    EXPECT_EQ(source.get_buffer_id(), 0);
}

TEST_F(SoundSourceTests, StateManagement)
{
    std::string filename = (test_audio_files::s_test_sub_dir1 / "test1.wav").string();
    sound_buffer buffer(filename);
    sound_source source;

    ASSERT_TRUE(source.attach_buffer(buffer));
    EXPECT_EQ(source.get_state(), sound_state::initial);
    EXPECT_FALSE(source.is_stopped());

    ASSERT_TRUE(source.play());
    sound_state state = source.get_state();
    EXPECT_TRUE(state == sound_state::playing || state == sound_state::stopped);

    ASSERT_TRUE(source.stop());
    EXPECT_EQ(source.get_state(), sound_state::stopped);
}

TEST_F(SoundSourceTests, NotCreatedOperations)
{
    // Test 1: every operation on a source that was never created gives invalid_state
    {
        sound_source source;

        std::vector<std::expected<void, error>> results;
        results.push_back(source.play());
        results.push_back(source.pause());
        results.push_back(source.stop());
        results.push_back(source.set_volume(0.5f));
        results.push_back(source.set_pitch(1.5f));
        results.push_back(source.set_position(vec3(1.0f, 2.0f, 3.0f)));
        results.push_back(source.set_velocity(vec3(1.0f, 2.0f, 3.0f)));
        results.push_back(source.set_looping(true));

        for (const auto &r : results)
        {
            ASSERT_FALSE(r);
            EXPECT_EQ(r.error().code, error_code::invalid_state);
        }
        EXPECT_FALSE(source.is_created());
    }

    // Test 2: the message names the operation
    {
        sound_source source;
        auto r = source.play();
        ASSERT_FALSE(r);
        EXPECT_NE(r.error().message.find("sound_source::play"), std::string::npos);
    }

    // Test 3: destroy and detach_buffer on a source that was never created are a success
    {
        sound_source source;
        EXPECT_TRUE(source.detach_buffer());
        EXPECT_TRUE(source.destroy());
    }
}

TEST_F(SoundSourceTests, CreateAndDestroy)
{
    sound_source source;

    // Test 1: create works and a second create stays a success
    {
        ASSERT_TRUE(source.create());
        EXPECT_TRUE(source.is_created());
        EXPECT_NE(source.get_source_id(), 0);
        EXPECT_TRUE(source.create());
    }

    // Test 2: destroy resets the source and operations fail again afterwards
    {
        ASSERT_TRUE(source.destroy());
        EXPECT_FALSE(source.is_created());
        EXPECT_EQ(source.get_source_id(), 0);

        auto r = source.play();
        ASSERT_FALSE(r);
        EXPECT_EQ(r.error().code, error_code::invalid_state);
    }
}

TEST_F(SoundSourceTests, MoveSemantics)
{
    sound_source source1;
    ALuint original_id = source1.get_source_id();

    sound_source source2 = std::move(source1);
    EXPECT_EQ(source2.get_source_id(), original_id);
    EXPECT_FALSE(source1.is_created());
    EXPECT_EQ(source1.get_source_id(), 0);
}
