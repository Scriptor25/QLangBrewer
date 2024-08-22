#pragma once

#include <Brewer/Brewer.hpp>

namespace Q
{
    Brewer::ValuePtr GenRef(Brewer::Builder&, const Brewer::ValuePtr&, Brewer::TypePtr*);
    Brewer::ValuePtr GenDeref(Brewer::Builder&, const Brewer::ValuePtr&, Brewer::TypePtr*);
}
