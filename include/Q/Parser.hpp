#pragma once

#include <Brewer/Brewer.hpp>

namespace Q
{
    Brewer::StmtPtr ParseCompound(Brewer::Parser&);
    Brewer::StmtPtr ParseDef(Brewer::Parser&);
    Brewer::StmtPtr ParseDoWhile(Brewer::Parser&);
    Brewer::StmtPtr ParseIf(Brewer::Parser&);
    Brewer::StmtPtr ParseReturn(Brewer::Parser&);
    Brewer::StmtPtr ParseUseAs(Brewer::Parser&);
    Brewer::StmtPtr ParseWhile(Brewer::Parser&);
}
