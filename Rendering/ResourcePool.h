//
// Created by harish on 10/19/26.
//

#ifndef VOXEL_RAYTRACER_RESOURCEPOOL_H
#define VOXEL_RAYTRACER_RESOURCEPOOL_H
#include <cstdint>
#include <vector>

template<typename Tag>
struct Handle {
    uint32_t id=UINT32_MAX;
    explicit operator bool() const { return id != UINT32_MAX; }
};

template<typename PooledType,typename Tag>
class ResourcePool {
    std::vector<PooledType> _items;
    std::vector<uint32_t> _free;
public:
    Handle<Tag> Add(PooledType n) {
        if (!_free.empty()) {
            uint32_t i = _free.back();
            _free.pop_back();
            _items[i] = n;
            return Handle<Tag> {i};
        }
        _items.push_back(n);
        return Handle<Tag> {static_cast<uint32_t>(_items.size() - 1)};
    }
    PooledType Get(Handle<Tag> handle) const {
        return handle.id < _items.size() ? _items[handle.id] : nullptr;
    }
    PooledType Remove(Handle<Tag> handle) {
        PooledType n = Get(handle);
        if (n) {
            _items[handle.id] = nullptr;
            _free.push_back(handle.id);
        }
        return n;
    }
};


#endif //VOXEL_RAYTRACER_RESOURCEPOOL_H