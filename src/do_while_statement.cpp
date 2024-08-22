#include <Brewer/Builder.hpp>
#include <Brewer/Value.hpp>
#include <Q/AST.hpp>

Q::DoWhileStatement::DoWhileStatement(const Brewer::SourceLocation& loc,
                                      Brewer::StmtPtr loop,
                                      Brewer::ExprPtr condition)
    : Statement(loc), Loop(std::move(loop)), Condition(std::move(condition))
{
}

std::ostream& Q::DoWhileStatement::Dump(std::ostream& stream) const
{
    return stream << "do " << Loop << " while " << Condition;
}

void Q::DoWhileStatement::GenIRNoVal(Brewer::Builder& builder) const
{
    const auto bkp = builder.IRBuilder().GetInsertBlock();
    const auto fn = bkp->getParent();
    const auto loop = llvm::BasicBlock::Create(builder.IRContext(), "loop", fn);
    const auto head = llvm::BasicBlock::Create(builder.IRContext(), "head", fn);
    const auto end = llvm::BasicBlock::Create(builder.IRContext(), "end", fn);

    const auto br = builder.IRBuilder().CreateBr(loop);

    builder.IRBuilder().SetInsertPoint(loop);
    Loop->GenIRNoVal(builder);
    builder.IRBuilder().CreateBr(head);

    builder.IRBuilder().SetInsertPoint(head);
    const auto condition = Condition->GenIR(builder);
    if (!condition)
    {
        builder.IRBuilder().SetInsertPoint(bkp);
        br->eraseFromParent();
        loop->eraseFromParent();
        head->eraseFromParent();
        end->eraseFromParent();
        return;
    }
    builder.IRBuilder().CreateCondBr(condition->Get(), loop, end);

    builder.IRBuilder().SetInsertPoint(end);
}
