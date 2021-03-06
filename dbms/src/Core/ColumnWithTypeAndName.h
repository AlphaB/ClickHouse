#pragma once

#include <Columns/IColumn.h>
#include <DataTypes/IDataType.h>


namespace DB
{

/** Column data along with its data type and name.
  * Column data could be nullptr - to represent just 'header' of column.
  * Name could be either name from a table or some temporary generated name during expression evaluation.
  */

struct ColumnWithTypeAndName
{
    ColumnPtr column;
    DataTypePtr type;
    String name;

    ColumnWithTypeAndName() {}
    ColumnWithTypeAndName(const ColumnPtr & column_, const DataTypePtr & type_, const String name_)
        : column(column_), type(type_), name(name_) {}

    ColumnWithTypeAndName cloneEmpty() const;
    bool operator==(const ColumnWithTypeAndName & other) const;
    String prettyPrint() const;
};

}
