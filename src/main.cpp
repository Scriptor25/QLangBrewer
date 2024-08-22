#include <filesystem>
#include <fstream>
#include <iostream>
#include <Brewer/Pipeline.hpp>
#include <Q/Operator.hpp>
#include <Q/Parser.hpp>

int main(const int argc, const char** argv)
{
    std::string input_filename;
    std::string module_id;
    std::string output_filename;

    bool dump_ast = false;
    bool dump_ir = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-o")
        {
            output_filename = argv[++i];
            continue;
        }
        if (arg == "-m")
        {
            module_id = argv[++i];
            continue;
        }
        if (arg == "-da")
        {
            dump_ast = true;
            continue;
        }
        if (arg == "-di")
        {
            dump_ir = true;
            continue;
        }
        if (input_filename.empty())
        {
            input_filename = arg;
            continue;
        }
        std::cerr << "unused argument '" << arg << "'" << std::endl;
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

    if (output_filename.empty()) output_filename = "a.out";
    if (module_id.empty()) module_id = std::filesystem::path(input_filename).replace_extension().filename().string();

    Brewer::Pipeline()
        .ParseStmtFn("{", Q::ParseCompound)
        .ParseStmtFn("def", Q::ParseDef)
        .ParseStmtFn("do", Q::ParseDoWhile)
        .ParseStmtFn("if", Q::ParseIf)
        .ParseStmtFn("return", Q::ParseReturn)
        .ParseStmtFn("use", Q::ParseUseAs)
        .ParseStmtFn("while", Q::ParseWhile)
        .GenUnaryFn("&", Q::GenRef)
        .GenUnaryFn("*", Q::GenDeref)
        .DumpAST(dump_ast)
        .DumpIR(dump_ir)
        .ModuleID(module_id)
        .BuildAndEmit(stream, input_filename, output_filename);
    return 0;
}
