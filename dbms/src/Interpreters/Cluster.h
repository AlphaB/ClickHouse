#pragma once

#include <map>
#include <Interpreters/Settings.h>
#include <Client/ConnectionPool.h>
#include <Client/ConnectionPoolWithFailover.h>
#include <Poco/Net/SocketAddress.h>

namespace DB
{

/// Cluster contains connection pools to each node
/// With the local nodes, the connection is not established, but the request is executed directly.
/// Therefore we store only the number of local nodes
/// In the config, the cluster includes nodes <node> or <shard>
class Cluster
{
public:
    Cluster(Poco::Util::AbstractConfiguration & config, const Settings & settings, const String & cluster_name);

    /// Construct a cluster by the names of shards and replicas. Local are treated as well as remote ones.
    Cluster(const Settings & settings, const std::vector<std::vector<String>> & names,
            const String & username, const String & password);

    Cluster(const Cluster &) = delete;
    Cluster & operator=(const Cluster &) = delete;

    /// is used to set a limit on the size of the timeout
    static Poco::Timespan saturate(const Poco::Timespan & v, const Poco::Timespan & limit);

public:
    struct Address
    {
        /** In configuration file,
        * addresses are located either in <node> elements:
        * <node>
        *     <host>example01-01-1</host>
        *     <port>9000</port>
        *     <!-- <user>, <password>, <default_database> if needed -->
        * </node>
        * ...
        * or in <shard> and inside in <replica> elements:
        * <shard>
        *     <replica>
        *         <host>example01-01-1</host>
        *         <port>9000</port>
        *         <!-- <user>, <password>, <default_database> if needed -->
        *    </replica>
        * </shard>
        */
        Poco::Net::SocketAddress resolved_address;
        String host_name;
        UInt16 port;
        String user;
        String password;
        String default_database;    /// this database is selected when no database is specified for Distributed table
        UInt32 replica_num;

        Address(Poco::Util::AbstractConfiguration & config, const String & config_prefix);
        Address(const String & host_port_, const String & user_, const String & password_);
    };

    using Addresses = std::vector<Address>;
    using AddressesWithFailover = std::vector<Addresses>;

    struct ShardInfo
    {
    public:
        bool isLocal() const { return !local_addresses.empty(); }
        bool hasRemoteConnections() const { return pool != nullptr; }
        size_t getLocalNodeCount() const { return local_addresses.size(); }

    public:
        /// contains names of directories for asynchronous write to StorageDistributed
        std::vector<std::string> dir_names;
        UInt32 shard_num;    /// Shard number, starting with 1.
        int weight;
        Addresses local_addresses;
        ConnectionPoolWithFailoverPtr pool;
    };

    using ShardsInfo = std::vector<ShardInfo>;

    String getHashOfAddresses() const { return hash_of_addresses; }
    const ShardsInfo & getShardsInfo() const { return shards_info; }
    const Addresses & getShardsAddresses() const { return addresses; }
    const AddressesWithFailover & getShardsWithFailoverAddresses() const { return addresses_with_failover; }

    const ShardInfo & getAnyShardInfo() const
    {
        if (shards_info.empty())
            throw Exception("Cluster is empty", ErrorCodes::LOGICAL_ERROR);
        return shards_info.front();
    }

    /// The number of remote shards.
    size_t getRemoteShardCount() const { return remote_shard_count; }

    /// The number of clickhouse nodes located locally
    /// we access the local nodes directly.
    size_t getLocalShardCount() const { return local_shard_count; }

    /// The number of all shards.
    size_t getShardCount() const { return shards_info.size(); }

    /// Get a subcluster consisting of one shard - index by count (from 0) of the shard of this cluster.
    std::unique_ptr<Cluster> getClusterWithSingleShard(size_t index) const;

private:
    using SlotToShard = std::vector<size_t>;
    SlotToShard slot_to_shard;

public:
    const SlotToShard & getSlotToShard() const { return slot_to_shard; }

private:
    void initMisc();

    /// Hash list of addresses and ports.
    /// We need it in order to be able to perform resharding requests
    /// on tables that have the distributed engine.
    void calculateHashOfAddresses();

    /// For getClusterWithSingleShard implementation.
    Cluster(const Cluster & from, size_t index);

    String hash_of_addresses;
    /// Description of the cluster shards.
    ShardsInfo shards_info;
    /// Any remote shard.
    ShardInfo * any_remote_shard_info = nullptr;

    /// Non-empty is either addresses or addresses_with_failover.
    /// The size and order of the elements in the corresponding array corresponds to shards_info.

    /// An array of shards. Each shard is the address of one server.
    Addresses addresses;
    /// An array of shards. For each shard, an array of replica addresses (servers that are considered identical).
    AddressesWithFailover addresses_with_failover;

    size_t remote_shard_count = 0;
    size_t local_shard_count = 0;
};

using ClusterPtr = std::shared_ptr<Cluster>;


class Clusters
{
public:
    Clusters(Poco::Util::AbstractConfiguration & config, const Settings & settings, const String & config_name = "remote_servers");

    Clusters(const Clusters &) = delete;
    Clusters & operator=(const Clusters &) = delete;

    ClusterPtr getCluster(const std::string & cluster_name) const;

    void updateClusters(Poco::Util::AbstractConfiguration & config, const Settings & settings, const String & config_name = "remote_servers");

public:
    using Impl = std::map<String, ClusterPtr>;

    Impl getContainer() const;

protected:
    Impl impl;
    mutable std::mutex mutex;
};

using ClustersPtr = std::shared_ptr<Clusters>;

}
