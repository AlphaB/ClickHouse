#pragma once

#include <Storages/IStorage.h>
#include <ext/shared_ptr_helper.hpp>

namespace DB
{

/// Provides information about graphite configuration.
class StorageSystemGraphite
    : private ext::shared_ptr_helper<StorageSystemGraphite>
    , public IStorage
{
    friend class ext::shared_ptr_helper<StorageSystemGraphite>;

public:
    static StoragePtr create(const std::string & name_);

    std::string getName() const override { return "SystemGraphite"; }
    std::string getTableName() const override { return name; }
    const NamesAndTypesList & getColumnsListImpl() const override { return columns; }

    BlockInputStreams read(
        const Names & column_names,
        const ASTPtr & query,
        const Context & context,
        QueryProcessingStage::Enum & processed_stage,
        size_t max_block_size,
        unsigned num_streams) override;

private:
    const std::string name;
    NamesAndTypesList columns;

    StorageSystemGraphite(const std::string & name_);
};

}
