#include <Brewer/Builder.hpp>
#include <Brewer/Context.hpp>
#include <Brewer/Type.hpp>
#include <Brewer/Value.hpp>
#include <llvm/IR/Verifier.h>
#include <Q/AST.hpp>

Q::Param::Param(Brewer::TypePtr type, std::string name)
    : Type(std::move(type)), Name(std::move(name))
{
}

Q::DefFunctionStatement::DefFunctionStatement(const Brewer::SourceLocation& loc,
                                              Brewer::TypePtr result,
                                              std::string name,
                                              const std::vector<Param>& params,
                                              const bool vararg,
                                              Brewer::StmtPtr body)
    : Statement(loc),
      Result(std::move(result)),
      Name(std::move(name)),
      Params(params),
      VarArg(vararg),
      Body(std::move(body))
{
}

std::ostream& Q::DefFunctionStatement::Dump(std::ostream& stream) const
{
    stream << "def " << Result->GetName() << " " << Name << "(";
    for (size_t i = 0; i < Params.size(); ++i)
    {
        if (i > 0) stream << ", ";
        stream << Params[i].Type->GetName();
        if (!Params[i].Name.empty()) stream << " " << Params[i].Name;
    }
    if (VarArg)
    {
        if (!Params.empty()) stream << ", ";
        stream << "?";
    }
    stream << ") ";
    if (!Body) return stream;
    return stream << Body;
}

void Q::DefFunctionStatement::GenIRNoVal(Brewer::Builder& builder) const
{
    std::vector<Brewer::TypePtr> param_types;
    for (const auto& param : Params) param_types.push_back(param.Type);
    const auto type = Brewer::FunctionType::Get(Result, param_types, VarArg);

    auto fn = builder.IRModule().getFunction(Name);

    if (!fn)
    {
        const auto fn_ty = type->GenIR(builder);
        fn = llvm::Function::Create(fn_ty, llvm::GlobalValue::ExternalLinkage, Name, builder.IRModule());
        for (size_t i = 0; i < Params.size(); ++i)
            fn->getArg(i)->setName(Params[i].Name);
    }

    builder[Name] = Brewer::RValue::Direct(builder, Brewer::PointerType::Get(type), fn);

    if (!Body) return;

    if (!fn->empty())
    {
        std::cerr
            << "at " << Location << ": "
            << "cannot redefine function"
            << std::endl;
        return;
    }

    const auto bb = llvm::BasicBlock::Create(builder.IRContext(), "entry", fn);
    builder.IRBuilder().SetInsertPoint(bb);

    builder.Push();
    builder.GetContext().CurrentResult() = Result;
    for (size_t i = 0; i < Params.size(); ++i)
        builder[Params[i].Name] = Brewer::RValue::Direct(builder, Params[i].Type, fn->getArg(i));
    Body->GenIRNoVal(builder);
    builder.Pop();

    if (verifyFunction(*fn, &llvm::errs()))
    {
        llvm::errs() << '\n';
        std::cerr
            << "at " << Location << ": "
            << "failed to verify function:"
            << std::endl;

        fn->print(llvm::errs());
        fn->erase(fn->begin(), fn->end());
    }

    builder.IRBuilder().ClearInsertionPoint();
}
