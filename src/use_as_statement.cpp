#include <Brewer/AST.hpp>
#include <Brewer/Context.hpp>
#include <Brewer/Parser.hpp>
#include <Q/Parser.hpp>

Brewer::StmtPtr Q::ParseUseAs(Brewer::Parser& parser)
{
    parser.Expect("use");
    const auto name = parser.Expect(Brewer::TokenType_Name).Value;
    parser.Expect("as");
    const auto type = parser.ParseType();
    parser.GetContext().GetType(name) = type;
    return {};
}
