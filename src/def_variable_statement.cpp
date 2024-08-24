#include <Brewer/Builder.hpp>
#include <Brewer/Type.hpp>
#include <Brewer/Value.hpp>
#include <Q/AST.hpp>

using namespace Brewer;

Q::DefVariableStatement::DefVariableStatement(const SourceLocation& loc,
                                              TypePtr type,
                                              std::string name,
                                              ExprPtr init)
    : Statement(loc), Type(std::move(type)), Name(std::move(name)), Init(std::move(init))
{
}

std::ostream& Q::DefVariableStatement::Dump(std::ostream& stream) const
{
    stream << "def " << Type->GetName() << " " << Name;
    if (!Init) return stream;
    return stream << " = " << Init;
}

void Q::DefVariableStatement::GenIRNoVal(Builder& builder) const
{
    if (!builder.IRBuilder().GetInsertBlock())
    {
        const auto type = Type->GenIR(builder);
        llvm::Constant* init;
        if (Init)
        {
            auto i = Init->GenIR(builder);
            if (!i) return;
            i = builder.GenCast(i, Type);
            if (!i) return;
            init = llvm::cast<llvm::Constant>(i->Get());
        }
        else
        {
            init = llvm::Constant::getNullValue(type);
        }

        const auto ptr = new llvm::GlobalVariable(type, false, llvm::GlobalValue::ExternalLinkage, init, Name);
        builder.IRModule().insertGlobalVariable(ptr);

        if (!Init)
        {
            if (const auto& ctor = builder.GetCtor(Type))
            {
                const auto func_type = FunctionType::From(PointerType::From(ctor->GetType())->GetBase());
                const auto fn_ty = func_type->GenIR(builder);
                const auto callee = ctor->Get();

                builder.IRBuilder().SetInsertPoint(&builder.GetGlobalCtor()->getEntryBlock());
                builder.IRBuilder().CreateCall(fn_ty, callee, {ptr});
                builder.IRBuilder().ClearInsertionPoint();
            }
        }

        const auto value = LValue::Direct(builder, Type, ptr);
        builder.GetSymbol(Name) = value;
        return;
    }

    const auto value = LValue::Alloca(builder, Type, Name);

    if (Init)
    {
        auto init = Init->GenIR(builder);
        if (!init) return;
        init = builder.GenCast(init, Type);
        if (!init) return;
        value->Set(init->Get());
    }
    else
    {
        // look for default constructor
        if (const auto& ctor = builder.GetCtor(Type))
        {
            const auto func_type = FunctionType::From(PointerType::From(ctor->GetType())->GetBase());
            const auto fn_ty = func_type->GenIR(builder);
            const auto callee = ctor->Get();

            builder.IRBuilder().CreateCall(fn_ty, callee, {value->GetPtr()});
        }
    }

    builder.GetSymbol(Name) = value;
}
