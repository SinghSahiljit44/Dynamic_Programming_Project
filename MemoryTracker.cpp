#include "MemoryTracker.hpp"
#include <cstdlib>
#include <new>

static std::size_t g_current_allocated_bytes = 0;
static std::size_t g_peak_allocated_bytes = 0;

void* operator new(std::size_t size) {
    std::size_t total_size = size + sizeof(std::size_t);
    void* ptr = std::malloc(total_size);
    if (!ptr) throw std::bad_alloc();
    
    *static_cast<std::size_t*>(ptr) = size;
    g_current_allocated_bytes += size;
    if (g_current_allocated_bytes > g_peak_allocated_bytes) {
        g_peak_allocated_bytes = g_current_allocated_bytes;
    }
    return static_cast<char*>(ptr) + sizeof(std::size_t);
}

void operator delete(void* ptr) noexcept {
    if (!ptr) return;
    void* real_ptr = static_cast<char*>(ptr) - sizeof(std::size_t);
    std::size_t size = *static_cast<std::size_t*>(real_ptr);
    if (g_current_allocated_bytes >= size) {
        g_current_allocated_bytes -= size;
    } else {
        g_current_allocated_bytes = 0;
    }
    std::free(real_ptr);
}

void operator delete(void* ptr, std::size_t size) noexcept {
    ::operator delete(ptr);
}

void* operator new[](std::size_t size) {
    return ::operator new(size);
}

void operator delete[](void* ptr) noexcept {
    ::operator delete(ptr);
}

void operator delete[](void* ptr, std::size_t size) noexcept {
    ::operator delete(ptr);
}

MemoryTracker::MemoryTracker() {
    start_allocated = g_current_allocated_bytes;
    g_peak_allocated_bytes = g_current_allocated_bytes;
}

double MemoryTracker::getPeakAllocatedKB() const {
    std::size_t peak_diff = (g_peak_allocated_bytes > start_allocated) 
                            ? (g_peak_allocated_bytes - start_allocated) 
                            : 0;
    return static_cast<double>(peak_diff) / 1024.0;
}