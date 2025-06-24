#include <gtest/gtest.h>
#include <soundcoe/core/audio_context.hpp>
#include <soundcoe/resources/resource_manager.hpp>
#include <soundcoe/core/types.hpp>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <vector>
#include <future>

using namespace soundcoe;

class TestAudioFiles
{
public:
    static std::filesystem::path s_testRootDir;
    static std::filesystem::path s_testSubDir1;
    static std::filesystem::path s_testSubDir2;
    static bool s_filesCreated;

    static void createTestFiles()
    {
        if (s_filesCreated)
            return;

        s_testRootDir = std::filesystem::temp_directory_path() / "soundcoe_test";
        s_testSubDir1 = s_testRootDir / "sounds";
        s_testSubDir2 = s_testRootDir / "music";

        std::filesystem::create_directories(s_testSubDir1);
        std::filesystem::create_directories(s_testSubDir2);

        createWavFile(s_testSubDir1 / "test1.wav");
        createWavFile(s_testSubDir1 / "test2.wav");
        createWavFile(s_testSubDir2 / "music1.wav");

        std::ofstream(s_testSubDir1 / "readme.txt") << "Not audio\n";

        s_filesCreated = true;
    }

    static void cleanup()
    {
        if (!s_filesCreated)
            return;
        std::error_code ec;
        std::filesystem::remove_all(s_testRootDir, ec);
        s_filesCreated = false;
    }

private:
    static void createWavFile(const std::filesystem::path &path)
    {
        std::ofstream file(path, std::ios::binary);

        const uint8_t header[] = {
            'R', 'I', 'F', 'F', 0x24, 0xAC, 0x00, 0x00, 'W', 'A', 'V', 'E',
            'f', 'm', 't', ' ', 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
            0x22, 0x56, 0x00, 0x00, 0x44, 0xAC, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00,
            'd', 'a', 't', 'a', 0x00, 0xAC, 0x00, 0x00};

        file.write(reinterpret_cast<const char *>(header), sizeof(header));

        const uint16_t silence = 0;
        for (int i = 0; i < 22050; ++i)
            file.write(reinterpret_cast<const char *>(&silence), sizeof(silence));
    }
};

std::filesystem::path TestAudioFiles::s_testRootDir;
std::filesystem::path TestAudioFiles::s_testSubDir1;
std::filesystem::path TestAudioFiles::s_testSubDir2;
bool TestAudioFiles::s_filesCreated = false;

class ResourceManagerTests : public ::testing::Test
{
protected:
    ResourceManager m_resourceManager;
    void SetUp() override
    {
        TestAudioFiles::createTestFiles();

        m_resourceManager.initialize(TestAudioFiles::s_testRootDir.string(), 4, 2);
    }

    void TearDown() override
    {
        try { m_resourceManager.shutdown(); }
        catch (...) { }
    }

    static void SetUpTestSuite()
    {
        TestAudioFiles::createTestFiles();
    }

    static void TearDownTestSuite()
    {
        TestAudioFiles::cleanup();
    }
};

TEST_F(ResourceManagerTests, InitialState)
{
    EXPECT_EQ(m_resourceManager.getActiveSourceCount(), 0);
    EXPECT_EQ(m_resourceManager.getTotalSourceCount(), 4);
    EXPECT_EQ(m_resourceManager.getCachedBufferCount(), 0);
    EXPECT_EQ(m_resourceManager.getCacheSizeBytes(), 0);
    EXPECT_TRUE(m_resourceManager.getLoadedDirectories().empty());
}

TEST_F(ResourceManagerTests, MultipleShutdown)
{
    EXPECT_NO_THROW(m_resourceManager.shutdown());
    EXPECT_NO_THROW(m_resourceManager.shutdown());
    EXPECT_FALSE(m_resourceManager.isInitialized());
}

TEST_F(ResourceManagerTests, SourceAcquisitionAndRelease)
{
    auto sourceOpt = m_resourceManager.acquireSource(SoundPriority::Medium);
    EXPECT_TRUE(sourceOpt.has_value());
    EXPECT_EQ(m_resourceManager.getActiveSourceCount(), 1);

    if (sourceOpt.has_value())
    {
        EXPECT_TRUE(m_resourceManager.releaseSource(sourceOpt.value()));
        EXPECT_EQ(m_resourceManager.getActiveSourceCount(), 0);
    }
}

TEST_F(ResourceManagerTests, SourcePoolExhaustion)
{
    std::vector<std::reference_wrapper<SoundSource>> sources;

    for(int i = 0; i < 4; ++i)
    {
        auto sourceOpt = m_resourceManager.acquireSource(SoundPriority::Low);
        EXPECT_TRUE(sourceOpt.has_value());
        if(sourceOpt.has_value())
            sources.push_back(sourceOpt.value());
    }

    EXPECT_EQ(m_resourceManager.getActiveSourceCount(), 4);

    auto highPriorityOpt = m_resourceManager.acquireSource(SoundPriority::High);
    EXPECT_TRUE(highPriorityOpt.has_value());
    EXPECT_EQ(m_resourceManager.getActiveSourceCount(), 4);

    if(highPriorityOpt.has_value())
        m_resourceManager.releaseSource(highPriorityOpt.value());

    for(auto &source : sources)
        m_resourceManager.releaseSource(source);
}

TEST_F(ResourceManagerTests, PriorityReplacement)
{
    std::vector<std::reference_wrapper<SoundSource>> lowSources;

    for(int i = 0; i < 4; ++i)
    {
        auto sourceOpt = m_resourceManager.acquireSource(SoundPriority::Low);
        if(sourceOpt.has_value())
            lowSources.push_back(sourceOpt.value());
    }

    auto criticalOpt = m_resourceManager.acquireSource(SoundPriority::Critical);
    EXPECT_TRUE(criticalOpt.has_value());

    if(criticalOpt.has_value())
        m_resourceManager.releaseSource(criticalOpt.value());

    for(auto &source : lowSources)
        m_resourceManager.releaseSource(source);
}

TEST_F(ResourceManagerTests, BufferLoadingAndCaching)
{
    auto bufferOpt1 = m_resourceManager.getBuffer("sounds/test1.wav");
    EXPECT_TRUE(bufferOpt1.has_value());
    EXPECT_EQ(m_resourceManager.getCachedBufferCount(), 1);
    EXPECT_GT(m_resourceManager.getCacheSizeBytes(), 0);

    auto bufferOpt2 = m_resourceManager.getBuffer("sounds/test1.wav");
    EXPECT_TRUE(bufferOpt2.has_value());

    if(bufferOpt1.has_value() && bufferOpt2.has_value())
        EXPECT_EQ(&bufferOpt1.value().get(), &bufferOpt2.value().get());

    EXPECT_EQ(m_resourceManager.getCachedBufferCount(), 1);

    EXPECT_TRUE(m_resourceManager.releaseBuffer("sounds/test1.wav"));
    EXPECT_TRUE(m_resourceManager.releaseBuffer("sounds/test1.wav"));
}

TEST_F(ResourceManagerTests, BufferReferenceCounting)
{
    const std::string filename = "sounds/test1.wav";

    auto buffer1 = m_resourceManager.getBuffer(filename);
    auto buffer2 = m_resourceManager.getBuffer(filename);
    auto buffer3 = m_resourceManager.getBuffer(filename);

    EXPECT_TRUE(buffer1.has_value());
    EXPECT_TRUE(buffer2.has_value());
    EXPECT_TRUE(buffer3.has_value());
    EXPECT_EQ(m_resourceManager.getCachedBufferCount(), 1);

    m_resourceManager.releaseBuffer(filename);
    m_resourceManager.releaseBuffer(filename);
    EXPECT_EQ(m_resourceManager.getCachedBufferCount(), 0);

    m_resourceManager.releaseBuffer(filename);
    buffer1 = m_resourceManager.getBuffer(filename);
    EXPECT_TRUE(buffer1.has_value());
    size_t cleaned = m_resourceManager.cleanupUnusedBuffers();
    EXPECT_EQ(cleaned, 1);
    EXPECT_EQ(m_resourceManager.getCachedBufferCount(), 0);
}

TEST_F(ResourceManagerTests, InvalidBufferRequests)
{
    auto nonExistent = m_resourceManager.getBuffer("nonexistent.wav");
    EXPECT_FALSE(nonExistent.has_value());

    auto nonAudio = m_resourceManager.getBuffer("sounds/readme.txt");
    EXPECT_FALSE(nonAudio.has_value());

    auto empty = m_resourceManager.getBuffer("");
    EXPECT_FALSE(empty.has_value());
}

TEST_F(ResourceManagerTests, DirectoryOperations)
{
    EXPECT_TRUE(m_resourceManager.preloadDirectory("sounds"));
    EXPECT_TRUE(m_resourceManager.isDirectoryLoaded("sounds"));
    EXPECT_GT(m_resourceManager.getCachedBufferCount(), 0);

    auto dirs = m_resourceManager.getLoadedDirectories();
    EXPECT_FALSE(dirs.empty());

    size_t buffersBefore = m_resourceManager.getCachedBufferCount();
    EXPECT_TRUE(m_resourceManager.unloadDirectory("sounds"));
    EXPECT_FALSE(m_resourceManager.isDirectoryLoaded("sounds"));

    m_resourceManager.cleanupUnusedBuffers();
    EXPECT_LT(m_resourceManager.getCachedBufferCount(), buffersBefore);
}

TEST_F(ResourceManagerTests, InvalidDirectoryOperations)
{
    EXPECT_FALSE(m_resourceManager.preloadDirectory("nonexistent"));
    EXPECT_FALSE(m_resourceManager.isDirectoryLoaded("nonexistent"));
    EXPECT_FALSE(m_resourceManager.preloadDirectory(""));
    EXPECT_TRUE(m_resourceManager.unloadDirectory("notloaded"));
}

TEST_F(ResourceManagerTests, FileOperations)
{
    EXPECT_TRUE(m_resourceManager.preloadFile("sounds/test1.wav"));
    EXPECT_EQ(m_resourceManager.getCachedBufferCount(), 1);

    auto buffer = m_resourceManager.getBuffer("sounds/test1.wav");
    EXPECT_TRUE(buffer.has_value());

    m_resourceManager.releaseBuffer("sounds/test1.wav");
    EXPECT_TRUE(m_resourceManager.unloadFile("sounds/test1.wav"));

    EXPECT_FALSE(m_resourceManager.preloadFile("nonexistent.wav"));
    EXPECT_FALSE(m_resourceManager.preloadFile("sounds/readme.txt"));
    EXPECT_TRUE(m_resourceManager.unloadFile("nonexistent.wav"));
}

TEST_F(ResourceManagerTests, ConcurrentSourceAccess)
{
    const int numThreads = 3;
    std::vector<std::future<std::vector<std::reference_wrapper<SoundSource>>>> futures;

    for (int i = 0; i < numThreads; ++i)
    {
        auto future = std::async(std::launch::async, [&]()
                                 {
            std::vector<std::reference_wrapper<SoundSource>> sources;
            for (int j = 0; j < 2; ++j)
            {
                auto sourceOpt = m_resourceManager.acquireSource(SoundPriority::Medium);
                if (sourceOpt.has_value())
                    sources.push_back(sourceOpt.value());
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            return sources; });
        futures.push_back(std::move(future));
    }

    std::vector<std::reference_wrapper<SoundSource>> allSources;
    for (auto &future : futures)
    {
        auto sources = future.get();
        allSources.insert(allSources.end(), sources.begin(), sources.end());
    }

    EXPECT_LE(allSources.size(), 4);
    EXPECT_EQ(m_resourceManager.getActiveSourceCount(), allSources.size());

    for (auto &source : allSources)
        m_resourceManager.releaseSource(source);

    EXPECT_EQ(m_resourceManager.getActiveSourceCount(), 0);
}

TEST_F(ResourceManagerTests, ConcurrentBufferAccess)
{
    std::vector<std::future<void>> futures;
    std::vector<std::string> files = {"sounds/test1.wav", "sounds/test2.wav", "music/music1.wav"};

    for (int i = 0; i < 3; ++i)
    {
        auto future = std::async(std::launch::async, [&, i]()
                                 {
            const std::string& filename = files[i];
            for (int j = 0; j < 3; ++j)
            {
                auto buffer = m_resourceManager.getBuffer(filename);
                if (buffer.has_value())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    m_resourceManager.releaseBuffer(filename);
                }
            } });
        futures.push_back(std::move(future));
    }

    for (auto &future : futures)
        future.wait();

    EXPECT_GT(m_resourceManager.getCachedBufferCount(), 0);
}

TEST_F(ResourceManagerTests, CompleteWorkflow)
{
    EXPECT_TRUE(m_resourceManager.preloadDirectory("sounds"));
    EXPECT_GT(m_resourceManager.getCachedBufferCount(), 0);

    auto source1 = m_resourceManager.acquireSource(SoundPriority::Medium);
    auto source2 = m_resourceManager.acquireSource(SoundPriority::Medium);
    auto buffer1 = m_resourceManager.getBuffer("sounds/test1.wav");
    auto buffer2 = m_resourceManager.getBuffer("sounds/test2.wav");

    EXPECT_TRUE(source1.has_value());
    EXPECT_TRUE(source2.has_value());
    EXPECT_TRUE(buffer1.has_value());
    EXPECT_TRUE(buffer2.has_value());
    EXPECT_EQ(m_resourceManager.getActiveSourceCount(), 2);

    auto highPriority = m_resourceManager.acquireSource(SoundPriority::High);
    EXPECT_TRUE(highPriority.has_value());

    if (source1.has_value())
        m_resourceManager.releaseSource(source1.value());
    if (source2.has_value())
        m_resourceManager.releaseSource(source2.value());
    if (highPriority.has_value())
        m_resourceManager.releaseSource(highPriority.value());

    m_resourceManager.releaseBuffer("sounds/test1.wav");
    m_resourceManager.releaseBuffer("sounds/test2.wav");

    EXPECT_EQ(m_resourceManager.getActiveSourceCount(), 0);
}

TEST_F(ResourceManagerTests, ErrorConditions)
{
    std::filesystem::path corruptFile = TestAudioFiles::s_testSubDir1 / "corrupt.wav";
    std::ofstream corrupt(corruptFile, std::ios::binary);
    corrupt << "Invalid WAV data";
    corrupt.close();

    auto corruptBuffer = m_resourceManager.getBuffer("sounds/corrupt.wav");
    EXPECT_FALSE(corruptBuffer.has_value());

    std::filesystem::remove(corruptFile);
}

TEST_F(ResourceManagerTests, CacheLimits)
{
    m_resourceManager.shutdown();
    m_resourceManager.initialize(TestAudioFiles::s_testRootDir.string(), 4, 1);

    std::vector<std::string> files = {"sounds/test1.wav", "sounds/test2.wav", "music/music1.wav"};

    for (const auto &file : files)
    {
        auto buffer = m_resourceManager.getBuffer(file);
        if (buffer.has_value())
            m_resourceManager.releaseBuffer(file);
    }

    EXPECT_GT(m_resourceManager.getCachedBufferCount(), 0);
    EXPECT_LE(m_resourceManager.getCacheSizeBytes(), 1 * 1024 * 1024);
}

TEST_F(ResourceManagerTests, ProperShutdown)
{
    m_resourceManager.preloadDirectory("sounds");
    auto source = m_resourceManager.acquireSource(SoundPriority::Medium);

    EXPECT_NO_THROW(m_resourceManager.shutdown());
    EXPECT_FALSE(m_resourceManager.isInitialized());

    auto newSource = m_resourceManager.acquireSource(SoundPriority::Medium);
    EXPECT_FALSE(newSource.has_value());

    auto buffer = m_resourceManager.getBuffer("sounds/test1.wav");
    EXPECT_FALSE(buffer.has_value());
}

class SoundBufferTests : public ::testing::Test
{
private:
    AudioContext m_audioContext;

protected:
    void SetUp() override
    {
        TestAudioFiles::createTestFiles();
        m_audioContext.initialize();
    }

    void TearDown() override
    {
        try { m_audioContext.shutdown(); }
        catch(...) {}
    }

    static void SetUpTestSuite() { TestAudioFiles::createTestFiles(); }
    static void TearDownTestSuite() { TestAudioFiles::cleanup(); }
};

TEST_F(SoundBufferTests, DefaultConstruction)
{
    SoundBuffer buffer;
    EXPECT_FALSE(buffer.isLoaded());
    EXPECT_EQ(buffer.getBufferId(), 0);
    EXPECT_EQ(buffer.getFileName(), "");
}

TEST_F(SoundBufferTests, FileConstructionAndLoading)
{
    std::string filename = (TestAudioFiles::s_testSubDir1 / "test1.wav").string();
    SoundBuffer buffer(filename);
    
    EXPECT_TRUE(buffer.isLoaded());
    EXPECT_NE(buffer.getBufferId(), 0);
    EXPECT_GT(buffer.getDuration(), 0.0f);
    EXPECT_EQ(buffer.getFileName(), filename);
}

TEST_F(SoundBufferTests, MoveSemantics)
{
    std::string filename = (TestAudioFiles::s_testSubDir1 / "test1.wav").string();
    SoundBuffer buffer1(filename);
    ALuint originalId = buffer1.getBufferId();
    
    SoundBuffer buffer2 = std::move(buffer1);
    EXPECT_EQ(buffer2.getBufferId(), originalId);
    EXPECT_FALSE(buffer1.isLoaded());
    EXPECT_EQ(buffer1.getBufferId(), 0);
}

TEST_F(SoundBufferTests, LoadAndUnload)
{
    SoundBuffer buffer;
    std::string filename = (TestAudioFiles::s_testSubDir1 / "test1.wav").string();
    
    buffer.loadFromFile(filename);
    EXPECT_TRUE(buffer.isLoaded());
    EXPECT_NE(buffer.getBufferId(), 0);
    
    buffer.unload();
    EXPECT_FALSE(buffer.isLoaded());
    EXPECT_EQ(buffer.getBufferId(), 0);
}

TEST_F(SoundBufferTests, InvalidFileHandling)
{
    SoundBuffer buffer;
    EXPECT_NO_THROW(buffer.loadFromFile("nonexistent.wav"));
    EXPECT_FALSE(buffer.isLoaded());
    
    std::string txtFile = (TestAudioFiles::s_testSubDir1 / "readme.txt").string();
    EXPECT_NO_THROW(buffer.loadFromFile(txtFile));
    EXPECT_FALSE(buffer.isLoaded());
}

class SoundSourceTests : public ::testing::Test
{
private:
    AudioContext m_audioContext;

protected:
    void SetUp() override
    {
        TestAudioFiles::createTestFiles();
        m_audioContext.initialize();
    }

    void TearDown() override
    {
        try { m_audioContext.shutdown(); }
        catch(...) {}
    }

    static void SetUpTestSuite() { TestAudioFiles::createTestFiles(); }
    static void TearDownTestSuite() { TestAudioFiles::cleanup(); }
};

TEST_F(SoundSourceTests, DefaultConstruction)
{
    SoundSource source;
    EXPECT_TRUE(source.isCreated());
    EXPECT_NE(source.getSourceId(), 0);
    EXPECT_EQ(source.getVolume(), 1.0f);
    EXPECT_EQ(source.getPitch(), 1.0f);
    EXPECT_FALSE(source.isLooping());
}

TEST_F(SoundSourceTests, PropertySettersAndGetters)
{
    SoundSource source;
    
    EXPECT_TRUE(source.setVolume(0.5f));
    EXPECT_FLOAT_EQ(source.getVolume(), 0.5f);
    
    EXPECT_TRUE(source.setPitch(1.5f));
    EXPECT_FLOAT_EQ(source.getPitch(), 1.5f);
    
    Vec3 pos(1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(source.setPosition(pos));
    EXPECT_EQ(source.getPosition().x, pos.x);
    EXPECT_EQ(source.getPosition().y, pos.y);
    EXPECT_EQ(source.getPosition().z, pos.z);
    
    EXPECT_TRUE(source.setLooping(true));
    EXPECT_TRUE(source.isLooping());
}

TEST_F(SoundSourceTests, BufferAttachmentAndPlayback)
{
    std::string filename = (TestAudioFiles::s_testSubDir1 / "test1.wav").string();
    SoundBuffer buffer(filename);
    SoundSource source;
    
    EXPECT_NO_THROW(source.attachBuffer(buffer));
    EXPECT_EQ(source.getBufferId(), buffer.getBufferId());
    
    EXPECT_TRUE(source.play());
    EXPECT_TRUE(source.pause());
    EXPECT_TRUE(source.stop());
    EXPECT_TRUE(source.isStopped());
    
    source.detachBuffer();
    EXPECT_EQ(source.getBufferId(), 0);
}

TEST_F(SoundSourceTests, StateManagement)
{
    std::string filename = (TestAudioFiles::s_testSubDir1 / "test1.wav").string();
    SoundBuffer buffer(filename);
    SoundSource source;
    
    source.attachBuffer(buffer);
    EXPECT_EQ(source.getState(), SoundState::Stopped);
    EXPECT_TRUE(source.isStopped());
    
    source.play();
    SoundState state = source.getState();
    EXPECT_TRUE(state == SoundState::Playing || state == SoundState::Stopped);
    
    source.stop();
    EXPECT_EQ(source.getState(), SoundState::Stopped);
}

TEST_F(SoundSourceTests, MoveSemantics)
{
    SoundSource source1;
    ALuint originalId = source1.getSourceId();
    
    SoundSource source2 = std::move(source1);
    EXPECT_EQ(source2.getSourceId(), originalId);
    EXPECT_FALSE(source1.isCreated());
    EXPECT_EQ(source1.getSourceId(), 0);
}