#include <Brewer/Builder.hpp>
#include <Q/AST.hpp>

Q::CompoundStatement::CompoundStatement(const Brewer::SourceLocation& loc, std::vector<Brewer::StmtPtr>& body)
    : Statement(loc)
{
    Body.reserve(body.size());
    for (Brewer::StmtPtr& ptr : body)
        Body.push_back(std::move(ptr));
}

static size_t depth = 0;

static std::ostream& indentation(std::ostream& stream)
{
    for (size_t i = 0; i < depth; ++i)
        stream << ' ';
    return stream;
}

std::ostream& Q::CompoundStatement::Dump(std::ostream& stream) const
{
    stream << "{";
    depth += 4;

    for (const auto& ptr : Body)
        indentation(stream << std::endl) << ptr;

    depth -= 4;
    return indentation(stream << std::endl) << "}";
}

void Q::CompoundStatement::GenIRNoVal(Brewer::Builder& builder) const
{
    builder.Push();
    for (const auto& ptr : Body)
        ptr->GenIRNoVal(builder);
    builder.Pop();
}
