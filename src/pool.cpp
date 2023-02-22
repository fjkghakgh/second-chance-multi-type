#include "pool.h"

#include <cassert>
#include <functional>

bool PoolAllocator::cmp(const Slab & a, const Slab & b)
{
    return a.obj_size < b.obj_size;
}

PoolAllocator::PoolAllocator(const std::size_t _block_size, std::initializer_list<std::size_t> _sizes)
    : block_size(_block_size)
    , storage(_block_size * _sizes.size())
{
    slabs.reserve(_sizes.size());
    auto it = _sizes.begin();
    for (std::size_t i = 0; it != _sizes.end(); ++i) {
        slabs.emplace_back(_block_size, *it, storage[i * _block_size]);
        ++it;
    }
    std::sort(slabs.begin(), slabs.end(), PoolAllocator::cmp);
}

PoolAllocator::Slab::Slab(std::size_t _size, std::size_t _obj_size, std::byte & _ptr)
    : obj_size(_obj_size)
    , ptr(&_ptr)
    , used(_size / _obj_size, false)
{
}

void * PoolAllocator::allocate(const std::size_t n)
{
    std::size_t idx = std::lower_bound(slabs.begin(), slabs.end(), Slab(static_cast<std::size_t>(0), n, storage[0]), cmp) - slabs.begin();
    for (std::size_t i = 0; i < slabs[idx].used.size(); ++i) {
        if (!slabs[idx].used[i]) {
            slabs[idx].used[i] = true;
            return (slabs[idx].ptr + (slabs[idx].obj_size * i));
        }
    }
    throw std::bad_alloc{};
}

void PoolAllocator::deallocate(const void * ptr)
{
    auto b_ptr = static_cast<const std::byte *>(ptr);
    std::less_equal<const std::byte *> cmp;
    if (cmp(&storage[0], b_ptr) && cmp(b_ptr, &storage.back())) {
        std::size_t idx = (b_ptr - &storage[0]) / block_size;
        assert((b_ptr - slabs[idx].ptr) % slabs[idx].obj_size == 0);
        slabs[idx].used[(b_ptr - slabs[idx].ptr) / slabs[idx].obj_size] = false;
    }
}
