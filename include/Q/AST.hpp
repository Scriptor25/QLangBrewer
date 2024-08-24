#pragma once

#include <Brewer/AST.hpp>
#include <Brewer/Type.hpp>

namespace Q
{
    struct Param
    {
        Param(Brewer::TypePtr, std::string);

        Brewer::TypePtr Type;
        std::string Name;
    };

    struct CompoundStatement : Brewer::Statement
    {
        CompoundStatement(const Brewer::SourceLocation&, std::vector<Brewer::StmtPtr>& body);

        std::ostream& Dump(std::ostream&) const override;
        void GenIRNoVal(Brewer::Builder&) const override;

        std::vector<Brewer::StmtPtr> Body;
    };

    struct DefFunctionStatement : Brewer::Statement
    {
        DefFunctionStatement(const Brewer::SourceLocation&,
                             Brewer::FuncMode mode,
                             Brewer::TypePtr self,
                             Brewer::TypePtr result,
                             std::string name,
                             const std::vector<Param>& params,
                             bool vararg,
                             Brewer::StmtPtr body);

        std::ostream& Dump(std::ostream&) const override;
        void GenIRNoVal(Brewer::Builder&) const override;

        Brewer::FuncMode Mode;
        Brewer::TypePtr Self;
        Brewer::TypePtr Result;
        std::string Name;
        std::vector<Param> Params;
        bool VarArg;
        Brewer::StmtPtr Body;
    };

    struct DefVariableStatement : Brewer::Statement
    {
        DefVariableStatement(const Brewer::SourceLocation&,
                             Brewer::TypePtr type,
                             std::string name,
                             Brewer::ExprPtr init);

        std::ostream& Dump(std::ostream&) const override;
        void GenIRNoVal(Brewer::Builder&) const override;

        Brewer::TypePtr Type;
        std::string Name;
        Brewer::ExprPtr Init;
    };

    struct DoWhileStatement : Brewer::Statement
    {
        DoWhileStatement(const Brewer::SourceLocation&, Brewer::StmtPtr loop, Brewer::ExprPtr condition);

        std::ostream& Dump(std::ostream&) const override;
        void GenIRNoVal(Brewer::Builder&) const override;

        Brewer::StmtPtr Loop;
        Brewer::ExprPtr Condition;
    };

    struct IfStatement : Brewer::Statement
    {
        IfStatement(const Brewer::SourceLocation&,
                    Brewer::ExprPtr if_,
                    Brewer::StmtPtr then,
                    Brewer::StmtPtr else_);

        std::ostream& Dump(std::ostream&) const override;
        void GenIRNoVal(Brewer::Builder&) const override;

        Brewer::ExprPtr If;
        Brewer::StmtPtr Then;
        Brewer::StmtPtr Else;
    };

    struct ReturnStatement : Brewer::Statement
    {
        ReturnStatement(const Brewer::SourceLocation&, Brewer::ExprPtr result);

        std::ostream& Dump(std::ostream&) const override;
        void GenIRNoVal(Brewer::Builder&) const override;

        Brewer::ExprPtr Result;
    };

    struct WhileStatement : Brewer::Statement
    {
        WhileStatement(const Brewer::SourceLocation&, Brewer::ExprPtr condition, Brewer::StmtPtr loop);

        std::ostream& Dump(std::ostream&) const override;
        void GenIRNoVal(Brewer::Builder&) const override;

        Brewer::ExprPtr Condition;
        Brewer::StmtPtr Loop;
    };
}
