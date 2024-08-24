#include <Brewer/Builder.hpp>
#include <Brewer/Context.hpp>
#include <Brewer/Type.hpp>
#include <Brewer/Value.hpp>
#include <llvm/IR/Verifier.h>
#include <Q/AST.hpp>

#include "Brewer/Util.hpp"

Q::Param::Param(Brewer::TypePtr type, std::string name)
    : Type(std::move(type)), Name(std::move(name))
{
}

Q::DefFunctionStatement::DefFunctionStatement(const Brewer::SourceLocation& loc,
                                              const Brewer::FuncMode mode,
                                              Brewer::TypePtr self,
                                              Brewer::TypePtr result,
                                              std::string name,
                                              const std::vector<Param>& params,
                                              const bool vararg,
                                              Brewer::StmtPtr body)
    : Statement(loc),
      Mode(mode),
      Self(std::move(self)),
      Result(std::move(result)),
      Name(std::move(name)),
      Params(params),
      VarArg(vararg),
      Body(std::move(body))
{
}

std::ostream& Q::DefFunctionStatement::Dump(std::ostream& stream) const
{
    stream << "def ";
    switch (Mode)
    {
    case Brewer::FuncMode_Normal:
        stream << Result->GetName() << " ";
        break;
    case Brewer::FuncMode_Ctor:
        stream << "+";
        break;
    case Brewer::FuncMode_Dtor:
        stream << "-";
        break;
    case Brewer::FuncMode_Member:
        stream << Result->GetName() << " " << Self->GetName() << ":";
        break;
    }
    stream << Name << "(";
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
    if (!dynamic_cast<CompoundStatement*>(Body.get()))
        stream << "= ";
    return stream << Body;
}

void Q::DefFunctionStatement::GenIRNoVal(Brewer::Builder& builder) const
{
    std::vector<Brewer::TypePtr> param_types;
    for (const auto& param : Params) param_types.push_back(param.Type);
    const auto type = Brewer::FunctionType::Get(Mode, Self, Result, param_types, VarArg);

    auto& ref = builder.GetFunction(Mode == Brewer::FuncMode_Member ? Self : nullptr, Name);

    llvm::Function* fn;
    if (!ref)
    {
        const auto fn_ty = type->GenIR(builder);
        fn = llvm::Function::Create(fn_ty, llvm::GlobalValue::ExternalLinkage, Name, builder.IRModule());
        if (Self) fn->getArg(0)->setName("self");
        for (size_t i = Self ? 1 : 0; i < Params.size(); ++i)
            fn->getArg(i)->setName(Params[i].Name);
        ref = Brewer::RValue::Direct(builder, Brewer::PointerType::Get(type), fn);
    }
    else
    {
        fn = llvm::cast<llvm::Function>(ref->Get());
    }

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

    if (Self)
        builder.GetSymbol("self") = Brewer::LValue::Direct(builder, Self, fn->getArg(0));
    for (size_t i = 0; i < Params.size(); ++i)
        builder.GetSymbol(Params[i].Name) = Brewer::RValue::Direct(builder,
                                                                   Params[i].Type,
                                                                   fn->getArg(i + (Self ? 1 : 0)));

    if (const auto p = dynamic_cast<Brewer::Expression*>(Body.get()))
    {
        auto result = p->GenIR(builder);
        if (!Result->IsVoid())
        {
            result = builder.GenCast(result, Result);
            builder.IRBuilder().CreateRet(result->Get());
        }
    }
    else
    {
        Body->GenIRNoVal(builder);
    }
    builder.Pop();

    if (Result->IsVoid())
        for (auto& block : *fn)
            if (!block.getTerminator())
            {
                builder.IRBuilder().SetInsertPoint(&block);
                builder.IRBuilder().CreateRetVoid();
            }

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
