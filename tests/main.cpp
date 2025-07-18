#include <testcoe.hpp>
#include <iostream>
#include <string>

int printHelp()
{
    std::cout << "Usage: ./soundcoe_tests [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help           Display this help message" << std::endl;
    std::cout << "  --all            Run all tests (default)" << std::endl;
    std::cout << "  --suite=NAME     Run only the specified test suite" << std::endl;
    std::cout << "  --test=SUITE.TEST Run only the specified test" << std::endl;
    std::cout << std::endl;
    std::cout << "Available test suites:" << std::endl;
    std::cout << "  AudioContextTests    - AudioContext singleton and initialization tests" << std::endl;
    std::cout << "  ErrorHandlerTests    - ErrorHandler functionality tests" << std::endl;
    std::cout << "  Vec3Tests            - Vec3 math operations tests" << std::endl;
    std::cout << "  MathTests            - Math utility functions tests" << std::endl;
    std::cout << "  ResourceManagerTests - ResourceManager comprehensive functionality tests" << std::endl;
    std::cout << "  SoundBufferTests     - SoundBuffer loading and management tests" << std::endl;
    std::cout << "  SoundSourceTests     - SoundSource playback and properties tests" << std::endl;
    std::cout << "  SoundManagerTests    - SoundManager high-level audio management tests" << std::endl;
    std::cout << std::endl;
    std::cout << "Example usage:" << std::endl;
    std::cout << "  ./soundcoe_tests --suite=AudioContextTests" << std::endl;
    std::cout << "  ./soundcoe_tests --test=Vec3Tests.Addition" << std::endl;
    std::cout << "  ./soundcoe_tests --test=ResourceManagerTests.SourceAcquisitionAndRelease" << std::endl;
    std::cout << "  ./soundcoe_tests --suite=SoundBufferTests" << std::endl;
    std::cout << "  ./soundcoe_tests --test=SoundSourceTests.PropertySettersAndGetters" << std::endl;
    std::cout << "  ./soundcoe_tests --suite=SoundManagerTests" << std::endl;
    std::cout << "  ./soundcoe_tests --test=SoundManagerTests.FadeInSound" << std::endl;

    return 0;
}

int main(int argc, char **argv)
{
    std::cout << "===================================================" << std::endl;
    std::cout << "                soundcoe Test Suite                " << std::endl;
    std::cout << "===================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Comprehensive testing for soundcoe audio library." << std::endl;
    std::cout << "Testing Core Layer: AudioContext, ErrorHandler, Types (Vec3)" << std::endl;
    std::cout << "Testing Resources Layer: ResourceManager with advanced features" << std::endl;
    std::cout << "Testing Playback Layer: SoundManager with fade effects and audio control" << std::endl;
    std::cout << std::endl;

    testcoe::init(&argc, argv);

    bool askForAll = false;
    std::string suiteName;
    std::string testName;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help")
            return printHelp();
        else if (arg == "--all")
            askForAll = true;
        else if (!askForAll && arg.substr(0, 8) == "--suite=")
            suiteName = arg.substr(8);
        else if (!askForAll && arg.substr(0, 7) == "--test=")
        {
            std::string fullTest = arg.substr(7);
            size_t dotPos = fullTest.find('.');
            if (dotPos != std::string::npos)
            {
                suiteName = fullTest.substr(0, dotPos);
                testName = fullTest.substr(dotPos + 1);
            }
        }
    }

    if (askForAll || (testName.empty() && suiteName.empty()))
    {
        std::cout << "Running all soundcoe tests..." << std::endl;
        return testcoe::run();
    }

    if (!testName.empty())
    {
        std::cout << "Running test: " << suiteName << "." << testName << std::endl;
        return testcoe::run_test(suiteName, testName);
    }

    if (!suiteName.empty())
    {
        std::cout << "Running suite: " << suiteName << std::endl;
        return testcoe::run_suite(suiteName);
    }

    return 0;
}