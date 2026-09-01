#pragma once

#include <cstddef>

struct MemoryTracker {
    std::size_t start_allocated{0};

    MemoryTracker();
    
    // Restituisce la memoria ausiliaria di picco allocata nello heap (in KB)
    [[nodiscard]] double getPeakAllocatedKB() const;
};