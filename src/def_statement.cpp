#include <Brewer/Builder.hpp>
#include <Brewer/Context.hpp>
#include <Brewer/Parser.hpp>
#include <Brewer/Type.hpp>
#include <Brewer/Value.hpp>
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

        if (auto& ref = parser.GetBuilder().GetFunction(mode == Brewer::FuncMode_Member ? self : nullptr, name); !ref)
            ref = Brewer::Value::Empty(Brewer::Type::GetFunPtr(mode, self, type, param_types, vararg));

        Brewer::StmtPtr body;
        if (parser.At("{") || parser.NextIfAt("="))
        {
            parser.GetBuilder().Push();
            for (auto& param : params)
                parser.GetBuilder().GetSymbol(param.Name) = Brewer::Value::Empty(param.Type);
            if (mode != Brewer::FuncMode_Normal)
                parser.GetBuilder().GetSymbol("self") = Brewer::Value::Empty(self);
            body = parser.Parse();
            parser.GetBuilder().Pop();
        }

        return std::make_unique<DefFunctionStatement>(loc, mode, self, type, name, params, vararg, std::move(body));
    }

    if (auto& ref = parser.GetBuilder().GetSymbol(name); !ref)
        ref = Brewer::Value::Empty(type);

    if (parser.NextIfAt("="))
    {
        auto init = parser.ParseExpr();
        return std::make_unique<DefVariableStatement>(loc, type, name, std::move(init));
    }

    return std::make_unique<DefVariableStatement>(loc, type, name, nullptr);
}
