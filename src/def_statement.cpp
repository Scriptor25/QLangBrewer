#include <Brewer/Context.hpp>
#include <Brewer/Parser.hpp>
#include <Brewer/Type.hpp>
#include <Q/AST.hpp>
#include <Q/Parser.hpp>

Brewer::StmtPtr Q::ParseDef(Brewer::Parser& parser)
{
    auto loc = parser.Expect("def").Location;

    Brewer::FuncMode mode;
    Brewer::TypePtr self;
    Brewer::TypePtr type;
    std::string name;

    if (parser.NextIfAt("+"))
    {
        mode = Brewer::FuncMode_Ctor;
        type = parser.GetContext().GetVoidTy();
        name = parser.Expect(Brewer::TokenType_Name).Value;
        self = Brewer::Type::Get(parser.GetContext(), name);
    }
    else if (parser.NextIfAt("-"))
    {
        mode = Brewer::FuncMode_Dtor;
        type = parser.GetContext().GetVoidTy();
        name = parser.Expect(Brewer::TokenType_Name).Value;
        self = Brewer::Type::Get(parser.GetContext(), name);
    }
    else
    {
        mode = Brewer::FuncMode_Normal;
        type = parser.ParseType();
        name = parser.Expect(Brewer::TokenType_Name).Value;
        if (parser.NextIfAt(":"))
        {
            mode = Brewer::FuncMode_Member;
            self = Brewer::Type::Get(parser.GetContext(), name);
            name = parser.Expect(Brewer::TokenType_Name).Value;
        }
    }

    if (parser.NextIfAt("("))
    {
        std::vector<Param> params;
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

        parser.GetContext().GetFunction(mode == Brewer::FuncMode_Member ? self : nullptr, name) =
            Brewer::Type::GetFunPtr(mode, self, type, param_types, vararg);

        Brewer::StmtPtr body;
        if (parser.At("{") || parser.NextIfAt("="))
        {
            parser.GetContext().Push();
            for (auto& param : params)
                parser.GetContext().GetSymbol(param.Name) = param.Type;
            if (mode != Brewer::FuncMode_Normal)
                parser.GetContext().GetSymbol("self") = self;
            body = parser.Parse();
            parser.GetContext().Pop();
        }
        return std::make_unique<DefFunctionStatement>(loc, mode, self, type, name, params, vararg, std::move(body));
    }

    parser.GetContext().GetSymbol(name) = type;

    if (parser.NextIfAt("="))
    {
        auto init = parser.ParseExpr();
        return std::make_unique<DefVariableStatement>(loc, type, name, std::move(init));
    }

    return std::make_unique<DefVariableStatement>(loc, type, name, nullptr);
}
