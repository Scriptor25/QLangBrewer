#include <fstream>
#include <iostream>
#include <Brewer/Pipeline.hpp>

int main(const int argc, const char** argv)
{
    std::string input_filename;
    std::string output_filename = "a.out";

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-o")
        {
            output_filename = argv[++i];
            continue;
        }

        if (input_filename.empty())
        {
            input_filename = arg;
            continue;
        }

        break;
    }

    if (input_filename.empty())
    {
        std::cerr << "no input file" << std::endl;
        return 1;
    }

    std::ifstream stream(input_filename);

    Brewer::Pipeline(stream, input_filename)
        .DumpAST()
        .DumpIR()
        .BuildAndEmit(output_filename);
}
