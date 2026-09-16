#pragma once

#include <cstddef>

// Traccia la memoria heap ausiliaria allocata durante la vita dell'oggetto,
// sostituendo gli operatori globali new/delete (vedi MemoryTracker.cpp).
// Gli oggetti sono annidabili: il picco registrato dal tracker esterno viene
// salvato alla costruzione e ripristinato alla distruzione di quello interno.
struct MemoryTracker {
    MemoryTracker();
    ~MemoryTracker();

    MemoryTracker(const MemoryTracker&) = delete;
    MemoryTracker& operator=(const MemoryTracker&) = delete;

    // Restituisce la memoria ausiliaria di picco allocata nello heap (in KB)
    [[nodiscard]] double getPeakAllocatedKB() const;

private:
    std::size_t start_allocated{0};
    std::size_t saved_peak{0};
};
