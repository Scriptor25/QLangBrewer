#include <fstream>
#include <iostream>
#include <Brewer/Pipeline.hpp>
#include <Q/AST.hpp>

#include "Brewer/Context.hpp"
#include "Brewer/Parser.hpp"
#include "Brewer/Type.hpp"

static Brewer::StmtPtr parse_compound(Brewer::Parser& parser)
{
    auto loc = parser.Expect("{").Location;

    std::vector<Brewer::StmtPtr> body;
    parser.GetContext().Push();
    while (!parser.NextIfAt("}"))
    {
        auto ptr = parser.Parse();
        body.push_back(std::move(ptr));
    }
    parser.GetContext().Pop();

    return std::make_unique<Q::CompoundStatement>(loc, body);
}

static Brewer::StmtPtr parse_def(Brewer::Parser& parser)
{
    auto loc = parser.Expect("def").Location;

    auto type = parser.ParseType();
    auto name = parser.Expect(Brewer::TokenType_Name).Value;

    if (parser.NextIfAt("("))
    {
        std::vector<Q::Param> params;
        std::vector<Brewer::TypePtr> param_types;
        bool vararg = false;
        while (!parser.NextIfAt(")"))
        {
            if (parser.NextIfAt("?"))
            {
                vararg = true;
                parser.Expect(")");
                break;
            }

            auto param_type = parser.ParseType();
            std::string param_name;
            if (parser.At(Brewer::TokenType_Name)) param_name = parser.Skip().Value;
            params.emplace_back(param_type, param_name);
            param_types.push_back(param_type);
            if (!parser.At(")")) parser.Expect(",");
        }

        parser.GetContext().GetSymbol(name) =
            Brewer::PointerType::Get(
                Brewer::FunctionType::Get(type, param_types, vararg));

        if (parser.At("{"))
        {
            parser.GetContext().Push();
            for (auto& param : params)
                parser.GetContext().GetSymbol(param.Name) = param.Type;
            auto body = parse_compound(parser);
            parser.GetContext().Pop();
            return std::make_unique<Q::DefFunctionStatement>(loc, type, name, params, vararg, std::move(body));
        }

        return std::make_unique<Q::DefFunctionStatement>(loc, type, name, params, vararg, nullptr);
    }

    parser.GetContext().GetSymbol(name) = type;

    if (parser.NextIfAt("="))
    {
        auto init = parser.ParseExpr();
        return std::make_unique<Q::DefVariableStatement>(loc, type, name, std::move(init));
    }

    return std::make_unique<Q::DefVariableStatement>(loc, type, name, nullptr);
}

static Brewer::StmtPtr parse_do_while(Brewer::Parser& parser)
{
    auto loc = parser.Expect("do").Location;
    auto loop = parser.Parse();
    parser.Expect("while");
    auto condition = parser.ParseExpr();
    return std::make_unique<Q::DoWhileStatement>(loc, std::move(loop), std::move(condition));
}

static Brewer::StmtPtr parse_if(Brewer::Parser& parser)
{
    auto loc = parser.Expect("if").Location;

    auto if_ = parser.ParseExpr();
    auto then = parser.Parse();

    if (parser.NextIfAt("else"))
    {
        auto else_ = parser.Parse();
        return std::make_unique<Q::IfStatement>(loc, std::move(if_), std::move(then), std::move(else_));
    }

    return std::make_unique<Q::IfStatement>(loc, std::move(if_), std::move(then), nullptr);
}

static Brewer::StmtPtr parse_return(Brewer::Parser& parser)
{
    auto loc = parser.Expect("return").Location;

    if (parser.NextIfAt("void"))
        return std::make_unique<Q::ReturnStatement>(loc, nullptr);

    auto result = parser.ParseExpr();
    return std::make_unique<Q::ReturnStatement>(loc, std::move(result));
}

static Brewer::StmtPtr parse_while(Brewer::Parser& parser)
{
    auto loc = parser.Expect("while").Location;
    auto condition = parser.ParseExpr();
    auto loop = parser.Parse();
    return std::make_unique<Q::WhileStatement>(loc, std::move(condition), std::move(loop));
}

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
        .ParseStmtFn("{", parse_compound)
        .ParseStmtFn("def", parse_def)
        .ParseStmtFn("do", parse_do_while)
        .ParseStmtFn("if", parse_if)
        .ParseStmtFn("return", parse_return)
        .ParseStmtFn("while", parse_while)
        .DumpIR()
        .BuildAndEmit(output_filename);
    return 0;
}
