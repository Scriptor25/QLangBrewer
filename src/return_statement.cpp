#include <Brewer/Builder.hpp>
#include <Brewer/Context.hpp>
#include <Brewer/Parser.hpp>
#include <Brewer/Value.hpp>
#include <Q/AST.hpp>
#include <Q/Parser.hpp>

Brewer::StmtPtr Q::ParseReturn(Brewer::Parser& parser)
{
    auto loc = parser.Expect("return").Location;

    if (parser.NextIfAt("void"))
        return std::make_unique<ReturnStatement>(loc, nullptr);

    auto result = parser.ParseExpr();
    return std::make_unique<ReturnStatement>(loc, std::move(result));
}

Q::ReturnStatement::ReturnStatement(const Brewer::SourceLocation& loc, Brewer::ExprPtr result)
    : Statement(loc), Result(std::move(result))
{
}

std::ostream& Q::ReturnStatement::Dump(std::ostream& stream) const
{
    stream << "return ";
    if (!Result) return stream << "void";
    return stream << Result;
}

void Q::ReturnStatement::GenIRNoVal(Brewer::Builder& builder) const
{
    if (!Result)
    {
        builder.IRBuilder().CreateRetVoid();
        return;
    }

    auto result = Result->GenIR(builder);
    if (!result) return;
    result = builder.GenCast(result, builder.GetContext().CurrentResult());

    builder.IRBuilder().CreateRet(result->Get());
}
