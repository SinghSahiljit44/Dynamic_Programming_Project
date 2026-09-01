#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>
#include <functional>

#include "GridMDP.hpp"
#include "MemoryTracker.hpp"

// Struttura per memorizzare le metriche di benchmark
struct BenchmarkResult {
    int iterations{0};
    double time_ms{0.0};
    double memory_extra_kb{0.0};
};

// Funzione generica per misurare l'esecuzione di un algoritmo di risoluzione
template <typename SolverFunc>
BenchmarkResult measurePerformance(SolverFunc solver, int N) {
    ValueMatrix V(N, std::vector<double>(N, 0.0));
    PolicyMatrix pi(N, std::vector<Action>(N, Action::NONE));

    MemoryTracker mem_tracker;
    auto start_time = std::chrono::high_resolution_clock::now();

    int iters = solver(V, pi);

    auto end_time = std::chrono::high_resolution_clock::now();

    double elapsed_time = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    double peak_memory = mem_tracker.getPeakAllocatedKB();

    return {iters, elapsed_time, peak_memory};
}

bool loadGridFromFile(const std::string& filename, int& N, Position& S, Position& G, Grid& grid) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[ERRORE] Impossibile aprire il file di input: " << filename << "\n";
        return false;
    }

    file >> N >> S.r >> S.c >> G.r >> G.c;
    grid.assign(N, std::vector<int>(N, 0));

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            file >> grid[r][c];
        }
    }

    return true;
}

void printHeader(std::ostream& os) {
    os << "========================================================================================\n";
    os << "                        BENCHMARK SPERIMENTALE: COMPITO 3                               \n";
    os << "========================================================================================\n";
    os << std::left 
       << std::setw(8)  << "N"
       << std::setw(14) << "Variante"
       << std::setw(14) << "Iterazioni"
       << std::setw(18) << "Tempo (ms)"
       << std::setw(20) << "Memoria Extra (KB)" << "\n";
    os << std::string(74, '-') << "\n";
}

void printRow(std::ostream& os, int N, const std::string& variant, const BenchmarkResult& res) {
    os << std::left 
       << std::setw(8)  << N
       << std::setw(14) << variant
       << std::setw(14) << res.iterations
       << std::setw(18) << std::fixed << std::setprecision(3) << res.time_ms
       << std::setw(20) << std::fixed << std::setprecision(2) << res.memory_extra_kb << "\n";
}

int main() {
    const std::vector<std::string> input_files = {
        "grid_1.txt", "grid_2.txt", "grid_3.txt", "grid_4.txt", "grid_5.txt",
        "grid_6.txt", "grid_7.txt", "grid_8.txt", "grid_9.txt", "grid_10.txt"
    };

    const std::string output_filename = "risultati_benchmark.txt";
    std::ofstream outFile(output_filename);

    if (!outFile.is_open()) {
        std::cerr << "[ERRORE] Impossibile creare il file di output: " << output_filename << "\n";
        return 1;
    }

    constexpr double gamma = 0.95;
    constexpr double eps = 1e-4;

    printHeader(std::cout);
    printHeader(outFile);

    for (const auto& file_path : input_files) {
        int N;
        Position S, G;
        Grid grid;

        if (!loadGridFromFile(file_path, N, S, G, grid)) {
            continue;
        }

        //Misurazione Variante Standard (
        auto res_std = measurePerformance([&](ValueMatrix& V, PolicyMatrix& pi) {
            return gridValueIteration(grid, N, G, gamma, eps, V, pi);
        }, N);

        //Misurazione Variante In-Place 
        auto res_ip = measurePerformance([&](ValueMatrix& V, PolicyMatrix& pi) {
            return gridValueIterationInPlace(grid, N, G, gamma, eps, V, pi);
        }, N);

        // Output dei risultati
        printRow(std::cout, N, "Standard", res_std);
        printRow(outFile,   N, "Standard", res_std);

        printRow(std::cout, N, "In-Place", res_ip);
        printRow(outFile,   N, "In-Place", res_ip);

        std::cout << std::string(74, '-') << "\n";
        outFile   << std::string(74, '-') << "\n";
    }

    outFile.close();
    std::cout << "\nRisultati del benchmark salvati con successo in '" << output_filename << "'.\n";

    return 0;
}