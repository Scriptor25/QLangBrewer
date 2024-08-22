#include <Brewer/Builder.hpp>
#include <Brewer/Parser.hpp>
#include <Brewer/Value.hpp>
#include <Q/AST.hpp>
#include <Q/Parser.hpp>

Brewer::StmtPtr Q::ParseWhile(Brewer::Parser& parser)
{
    auto loc = parser.Expect("while").Location;
    auto condition = parser.ParseExpr();
    auto loop = parser.Parse();
    return std::make_unique<WhileStatement>(loc, std::move(condition), std::move(loop));
}

Q::WhileStatement::WhileStatement(const Brewer::SourceLocation& loc, Brewer::ExprPtr condition, Brewer::StmtPtr loop)
    : Statement(loc), Condition(std::move(condition)), Loop(std::move(loop))
{
}

std::ostream& Q::WhileStatement::Dump(std::ostream& stream) const
{
    return stream << "while " << Condition << " " << Loop;
}

void Q::WhileStatement::GenIRNoVal(Brewer::Builder& builder) const
{
    const auto bkp = builder.IRBuilder().GetInsertBlock();
    const auto fn = bkp->getParent();
    const auto head = llvm::BasicBlock::Create(builder.IRContext(), "head", fn);
    const auto loop = llvm::BasicBlock::Create(builder.IRContext(), "loop", fn);
    const auto end = llvm::BasicBlock::Create(builder.IRContext(), "end", fn);

    const auto br = builder.IRBuilder().CreateBr(head);

    builder.IRBuilder().SetInsertPoint(head);
    const auto condition = Condition->GenIR(builder);
    if (!condition)
    {
        builder.IRBuilder().SetInsertPoint(bkp);
        br->eraseFromParent();
        head->eraseFromParent();
        loop->eraseFromParent();
        end->eraseFromParent();
        return;
    }
    builder.IRBuilder().CreateCondBr(condition->Get(), loop, end);

    builder.IRBuilder().SetInsertPoint(loop);
    Loop->GenIRNoVal(builder);
    builder.IRBuilder().CreateBr(head);

    builder.IRBuilder().SetInsertPoint(end);
}
