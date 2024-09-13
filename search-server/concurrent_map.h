#pragma once

#include <algorithm>
#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <mutex>

using namespace std;

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct SingleMap {
        std::mutex mutex;
        std::map<Key, Value> map;
    };

    struct Access {
        lock_guard<mutex> guard;
        Value& ref_to_value;

        Access(const Key& key, SingleMap& bucket) : guard(bucket.mutex), ref_to_value(bucket.map[key]) {}

    };

    explicit ConcurrentMap(size_t bucket_count) : buckets_(bucket_count) {}

    Access operator[](const Key& key) {
        auto& bucket = buckets_[static_cast<uint64_t>(key) % buckets_.size()];
        return {key, bucket};
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        for (auto& [mutex, map] : buckets_) {
            std::lock_guard g(mutex);
            result.insert(map.begin(), map.end());
        }
        return result;
    }

    void Erase(Key value) {
        BuildOrdinaryMap().erase(value);
    }

private:
    std::vector<SingleMap> buckets_;
};