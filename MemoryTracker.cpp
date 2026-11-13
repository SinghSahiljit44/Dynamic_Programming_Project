#include "MemoryTracker.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <new>

static std::size_t g_current_allocated_bytes = 0;
static std::size_t g_peak_allocated_bytes = 0;

// Davanti a ogni blocco restituito all'utente viene riservato un header che ne
// memorizza la dimensione. L'header è grande quanto l'allineamento fondamentale
// (e non sizeof(std::size_t)): spostare il puntatore di soli 8 byte lo renderebbe
// allineato a 8 anziché a alignof(std::max_align_t), violando il contratto di
// operator new per i tipi con requisiti di allineamento più stringenti.
static constexpr std::size_t HEADER_SIZE = alignof(std::max_align_t);
static_assert(HEADER_SIZE >= sizeof(std::size_t), "Header troppo piccolo per la dimensione del blocco");

void* operator new(std::size_t size) {
    void* ptr = std::malloc(size + HEADER_SIZE);
    if (!ptr) throw std::bad_alloc();

    *static_cast<std::size_t*>(ptr) = size;
    g_current_allocated_bytes += size;
    if (g_current_allocated_bytes > g_peak_allocated_bytes) {
        g_peak_allocated_bytes = g_current_allocated_bytes;
    }
    return static_cast<char*>(ptr) + HEADER_SIZE;
}

void operator delete(void* ptr) noexcept {
    if (!ptr) return;
    void* real_ptr = static_cast<char*>(ptr) - HEADER_SIZE;
    std::size_t size = *static_cast<std::size_t*>(real_ptr);
    if (g_current_allocated_bytes >= size) {
        g_current_allocated_bytes -= size;
    } else {
        g_current_allocated_bytes = 0;
    }
    std::free(real_ptr);
}

void operator delete(void* ptr, std::size_t) noexcept {
    ::operator delete(ptr);
}

void* operator new[](std::size_t size) {
    return ::operator new(size);
}

void operator delete[](void* ptr) noexcept {
    ::operator delete(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept {
    ::operator delete(ptr);
}

MemoryTracker::MemoryTracker() {
    start_allocated = g_current_allocated_bytes;
    saved_peak = g_peak_allocated_bytes;
    g_peak_allocated_bytes = g_current_allocated_bytes;
}

MemoryTracker::~MemoryTracker() {
    // Ripristina il picco visto dall'eventuale tracker che racchiude questo.
    g_peak_allocated_bytes = std::max(saved_peak, g_peak_allocated_bytes);
}

double MemoryTracker::getPeakAllocatedKB() const {
    std::size_t peak_diff = (g_peak_allocated_bytes > start_allocated)
                            ? (g_peak_allocated_bytes - start_allocated)
                            : 0;
    return static_cast<double>(peak_diff) / 1024.0;
}
