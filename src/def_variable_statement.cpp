#include <Brewer/Builder.hpp>
#include <Brewer/Type.hpp>
#include <Brewer/Value.hpp>
#include <Q/AST.hpp>

Q::DefVariableStatement::DefVariableStatement(const Brewer::SourceLocation& loc,
                                              Brewer::TypePtr type,
                                              std::string name,
                                              Brewer::ExprPtr init)
    : Statement(loc), Type(std::move(type)), Name(std::move(name)), Init(std::move(init))
{
}

std::ostream& Q::DefVariableStatement::Dump(std::ostream& stream) const
{
    stream << "def " << Type->GetName() << " " << Name;
    if (!Init) return stream;
    return stream << " = " << Init;
}

void Q::DefVariableStatement::GenIRNoVal(Brewer::Builder& builder) const
{
    const auto value = Brewer::LValue::Alloca(builder, Type, Name);

    if (Init)
    {
        auto init = Init->GenIR(builder);
        if (!init) return;
        init = builder.GenCast(init, Type);
        if (!init) return;
        value->Set(init->Get());
    }

    builder.GetSymbol(Name) = value;
}
