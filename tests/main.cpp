#include <testcoe.hpp>
#include <iostream>
#include <string>
#include <cstddef>

int print_help()
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
    std::cout << "  ErrorTests           - error type and to_string tests" << std::endl;
    std::cout << "  Vec3Tests            - vec3 math operations tests" << std::endl;
    std::cout << "  MathTests            - Math utility functions tests" << std::endl;
    std::cout << "  ResourceManagerTests - ResourceManager comprehensive functionality tests" << std::endl;
    std::cout << "  AudioDataTests       - AudioData decoder failure tests" << std::endl;
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
    std::cout << "Testing Core Layer: AudioContext, ErrorHandler, Error, Types (Vec3)" << std::endl;
    std::cout << "Testing Resources Layer: ResourceManager, SoundBuffer, SoundSource, AudioData" << std::endl;
    std::cout << "Testing Utils Layer: Math utility functions" << std::endl;
    std::cout << "Testing Playback Layer: SoundManager with fade effects and audio control" << std::endl;
    std::cout << std::endl;

    testcoe::init(&argc, argv);

    bool ask_for_all = false;
    std::string suite_name;
    std::string test_name;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help")
            return print_help();
        else if (arg == "--all")
            ask_for_all = true;
        else if (!ask_for_all && arg.substr(0, 8) == "--suite=")
            suite_name = arg.substr(8);
        else if (!ask_for_all && arg.substr(0, 7) == "--test=")
        {
            std::string full_test = arg.substr(7);
            std::size_t dot_pos = full_test.find('.');
            if (dot_pos != std::string::npos)
            {
                suite_name = full_test.substr(0, dot_pos);
                test_name = full_test.substr(dot_pos + 1);
            }
        }
    }

    if (ask_for_all || (test_name.empty() && suite_name.empty()))
    {
        std::cout << "Running all soundcoe tests..." << std::endl;
        return testcoe::run();
    }

    if (!test_name.empty())
    {
        std::cout << "Running test: " << suite_name << "." << test_name << std::endl;
        return testcoe::run_test(suite_name, test_name);
    }

    if (!suite_name.empty())
    {
        std::cout << "Running suite: " << suite_name << std::endl;
        return testcoe::run_suite(suite_name);
    }

    return 0;
}
