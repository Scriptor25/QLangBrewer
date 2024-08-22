#include <Brewer/Builder.hpp>
#include <Brewer/Parser.hpp>
#include <Brewer/Value.hpp>
#include <llvm/IR/BasicBlock.h>
#include <Q/AST.hpp>
#include <Q/Parser.hpp>

Brewer::StmtPtr Q::ParseIf(Brewer::Parser& parser)
{
    auto loc = parser.Expect("if").Location;

    auto if_ = parser.ParseExpr();
    auto then = parser.Parse();

    if (parser.NextIfAt("else"))
    {
        auto else_ = parser.Parse();
        return std::make_unique<IfStatement>(loc, std::move(if_), std::move(then), std::move(else_));
    }

    return std::make_unique<IfStatement>(loc, std::move(if_), std::move(then), nullptr);
}

Q::IfStatement::IfStatement(const Brewer::SourceLocation& loc,
                            Brewer::ExprPtr if_,
                            Brewer::StmtPtr then,
                            Brewer::StmtPtr else_)
    : Statement(loc), If(std::move(if_)), Then(std::move(then)), Else(std::move(else_))
{
}

std::ostream& Q::IfStatement::Dump(std::ostream& stream) const
{
    stream << "if " << If << " " << Then;
    if (!Else) return stream;
    return stream << " else " << Else;
}

void Q::IfStatement::GenIRNoVal(Brewer::Builder& builder) const
{
    const auto bkp = builder.IRBuilder().GetInsertBlock();
    const auto fn = bkp->getParent();
    const auto if_ = llvm::BasicBlock::Create(builder.IRContext(), "if", fn);
    const auto then = llvm::BasicBlock::Create(builder.IRContext(), "then", fn);
    const auto else_ = Else
                           ? llvm::BasicBlock::Create(builder.IRContext(), "else", fn)
                           : llvm::BasicBlock::Create(builder.IRContext(), "end", fn);
    const auto end = Else ? llvm::BasicBlock::Create(builder.IRContext(), "end", fn) : else_;

    const auto br = builder.IRBuilder().CreateBr(if_);

    builder.IRBuilder().SetInsertPoint(if_);
    const auto condition = If->GenIR(builder);
    if (!condition)
    {
        builder.IRBuilder().SetInsertPoint(bkp);
        br->eraseFromParent();
        if_->eraseFromParent();
        then->eraseFromParent();
        if (Else) else_->eraseFromParent();
        end->eraseFromParent();
        return;
    }
    builder.IRBuilder().CreateCondBr(condition->Get(), then, else_);

    builder.IRBuilder().SetInsertPoint(then);
    Then->GenIRNoVal(builder);
    if (!then->getTerminator())
        builder.IRBuilder().CreateBr(end);

    if (Else)
    {
        builder.IRBuilder().SetInsertPoint(else_);
        Else->GenIRNoVal(builder);
        if (!else_->getTerminator())
            builder.IRBuilder().CreateBr(end);
    }

    if (end->hasNPredecessors(0))
    {
        builder.IRBuilder().ClearInsertionPoint();
        end->eraseFromParent();
        return;
    }

    builder.IRBuilder().SetInsertPoint(end);
}
