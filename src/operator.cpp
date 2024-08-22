#include <Brewer/Type.hpp>
#include <Brewer/Util.hpp>
#include <Brewer/Value.hpp>
#include <Q/Operator.hpp>

Brewer::ValuePtr Q::GenRef(Brewer::Builder& builder, const Brewer::ValuePtr& operand, Brewer::TypePtr* result_type)
{
    if (result_type)
    {
        *result_type = Brewer::PointerType::Get(operand->GetType());
        return {};
    }

    if (const auto l_operand = std::dynamic_pointer_cast<Brewer::LValue>(operand))
        return Brewer::RValue::Direct(builder, Brewer::PointerType::Get(l_operand->GetType()), l_operand->GetPtr());
    return std::cerr
        << "cannot get reference to constant"
        << std::endl
        << Brewer::ErrMark<Brewer::ValuePtr>();
}

Brewer::ValuePtr Q::GenDeref(Brewer::Builder&, const Brewer::ValuePtr& operand, Brewer::TypePtr* result_type)
{
    if (result_type)
    {
        *result_type = std::dynamic_pointer_cast<Brewer::PointerType>(operand->GetType())->GetBase();
        return {};
    }
    return operand->Dereference();
}
