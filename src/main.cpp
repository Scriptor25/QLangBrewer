#include <fstream>
#include <iostream>
#include <Brewer/AST.hpp>
#include <Brewer/Pipeline.hpp>
#include <Q/Operator.hpp>
#include <Q/Parser.hpp>

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
    if (!stream)
    {
        std::cerr << "failed to open " << input_filename << std::endl;
        return 1;
    }

    Brewer::Pipeline(stream, input_filename)
        .ParseStmtFn("{", Q::ParseCompound)
        .ParseStmtFn("def", Q::ParseDef)
        .ParseStmtFn("do", Q::ParseDoWhile)
        .ParseStmtFn("if", Q::ParseIf)
        .ParseStmtFn("return", Q::ParseReturn)
        .ParseStmtFn("use", Q::ParseUseAs)
        .ParseStmtFn("while", Q::ParseWhile)
        .GenUnaryFn("&", Q::GenRef)
        .GenUnaryFn("*", Q::GenDeref)
        .DumpIR()
        .BuildAndEmit(output_filename);
    return 0;
}
