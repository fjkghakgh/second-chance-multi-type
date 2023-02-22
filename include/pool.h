#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <new>

class PoolAllocator
{
public:
    PoolAllocator(std::size_t _block_size, std::initializer_list<std::size_t> _sizes);

    void * allocate(std::size_t n);

    void deallocate(const void * ptr);

private:
    struct Slab
    {
        Slab(std::size_t _size, std::size_t _obj_size, std::byte & _ptr);

        std::size_t obj_size;

        std::byte * ptr;

        std::vector<bool> used;
    };

    static bool cmp(const Slab & a, const Slab & b);

    std::size_t block_size;

    std::vector<std::byte> storage;

    std::vector<Slab> slabs;
};
