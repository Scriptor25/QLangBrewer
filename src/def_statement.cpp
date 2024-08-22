#include <Brewer/Context.hpp>
#include <Brewer/Parser.hpp>
#include <Brewer/Type.hpp>
#include <Q/AST.hpp>
#include <Q/Parser.hpp>

Brewer::StmtPtr Q::ParseDef(Brewer::Parser& parser)
{
    auto loc = parser.Expect("def").Location;

    auto type = parser.ParseType();
    auto name = parser.Expect(Brewer::TokenType_Name).Value;

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

        parser.GetContext().GetSymbol(name) =
            Brewer::PointerType::Get(
                Brewer::FunctionType::Get(type, param_types, vararg));

        if (parser.NextIfAt("=") || parser.At("{"))
        {
            parser.GetContext().Push();
            for (auto& param : params)
                parser.GetContext().GetSymbol(param.Name) = param.Type;
            auto body = parser.Parse();
            parser.GetContext().Pop();
            return std::make_unique<DefFunctionStatement>(loc, type, name, params, vararg, std::move(body));
        }

        return std::make_unique<DefFunctionStatement>(loc, type, name, params, vararg, nullptr);
    }

    parser.GetContext().GetSymbol(name) = type;

    if (parser.NextIfAt("="))
    {
        auto init = parser.ParseExpr();
        return std::make_unique<DefVariableStatement>(loc, type, name, std::move(init));
    }

    return std::make_unique<DefVariableStatement>(loc, type, name, nullptr);
}
