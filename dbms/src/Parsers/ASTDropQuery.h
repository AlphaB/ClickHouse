#pragma once

#include <Parsers/IAST.h>


namespace DB
{


/** DROP query
  */
class ASTDropQuery : public IAST
{
public:
    bool detach{false};    /// DETACH query, not DROP.
    bool if_exists{false};
    String database;
    String table;

    ASTDropQuery() = default;
    ASTDropQuery(const StringRange range_) : IAST(range_) {}

    /** Get the text that identifies this element. */
    String getID() const override { return (detach ? "DetachQuery_" : "DropQuery_") + database + "_" + table; };

    ASTPtr clone() const override { return std::make_shared<ASTDropQuery>(*this); }

protected:
    void formatImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const override
    {
        if (table.empty() && !database.empty())
        {
            settings.ostr << (settings.hilite ? hilite_keyword : "")
                << (detach ? "DETACH DATABASE " : "DROP DATABASE ")
                << (if_exists ? "IF EXISTS " : "")
                << (settings.hilite ? hilite_none : "")
                << backQuoteIfNeed(database);
            return;
        }

        settings.ostr << (settings.hilite ? hilite_keyword : "")
            << (detach ? "DETACH TABLE " : "DROP TABLE ")
            << (if_exists ? "IF EXISTS " : "") << (settings.hilite ? hilite_none : "")
            << (!database.empty() ? backQuoteIfNeed(database) + "." : "") << backQuoteIfNeed(table);
    }
};

}
