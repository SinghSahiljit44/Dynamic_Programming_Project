#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>
#include "GridMDP.hpp"

// Funzione per caricare la griglia da file di testo
bool loadGridFromFile(const std::string& filename, int& N, Position& S, Position& G, Grid& grid) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[ERRORE] Impossibile aprire il file di input: " << filename << "\n";
        return false;
    }

    file >> N;
    file >> S.r >> S.c;
    file >> G.r >> G.c;

    grid.assign(N, std::vector<int>(N, 0));
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            file >> grid[r][c];
        }
    }

    file.close();
    return true;
}

int main() {
    const std::vector<std::string> input_files = {
        "grid_1.txt",
        "grid_2.txt",
        "grid_3.txt",
        "grid_4.txt",
        "grid_5.txt",
        "grid_6.txt",
        "grid_7.txt",
        "grid_8.txt",
        "grid_9.txt",
        "grid_10.txt"
    };

    const std::string output_filename = "risultati_benchmark.txt";
    std::ofstream outFile(output_filename);

    if (!outFile.is_open()) {
        std::cerr << "[ERRORE] Impossibile creare il file di output: " << output_filename << "\n";
        return 1;
    }

    const double gamma = 0.95;
    const double eps = 1e-4;

    // Intestazione tabella per stdout e file
    auto printHeader = [](std::ostream& os) {
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
    };

    printHeader(std::cout);
    printHeader(outFile);

    for (const auto& file_path : input_files) {
        int N;
        Position S, G;
        Grid grid;

        if (!loadGridFromFile(file_path, N, S, G, grid)) {
            continue; // Salta il file se non viene trovato
        }

        // ---------------------------------------------------------
        // 1. Esecuzione Variante Standard (Jacobi)
        // ---------------------------------------------------------
        ValueMatrix V_std;
        PolicyMatrix pi_std;

        auto t1_start = std::chrono::high_resolution_clock::now();
        int iters_std = gridValueIteration(grid, N, G, gamma, eps, V_std, pi_std);
        auto t1_end = std::chrono::high_resolution_clock::now();

        double time_std = std::chrono::duration<double, std::milli>(t1_end - t1_start).count();
        double mem_std_kb = (N * N * sizeof(double)) / 1024.0;

        // ---------------------------------------------------------
        // 2. Esecuzione Variante In-Place (Gauss-Seidel)
        // ---------------------------------------------------------
        ValueMatrix V_ip;
        PolicyMatrix pi_ip;

        auto t2_start = std::chrono::high_resolution_clock::now();
        int iters_ip = gridValueIterationInPlace(grid, N, G, gamma, eps, V_ip, pi_ip);
        auto t2_end = std::chrono::high_resolution_clock::now();

        double time_ip = std::chrono::duration<double, std::milli>(t2_end - t2_start).count();
        double mem_ip_kb = sizeof(double) / 1024.0;

        // Lambda per stampare la riga sia a schermo sia su file
        auto printRow = [&](std::ostream& os, const std::string& var, int iters, double time, double mem) {
            os << std::left 
               << std::setw(8)  << N
               << std::setw(14) << var
               << std::setw(14) << iters
               << std::setw(18) << std::fixed << std::setprecision(3) << time
               << std::setw(20) << std::fixed << std::setprecision(2) << mem << "\n";
        };

        printRow(std::cout, "Standard", iters_std, time_std, mem_std_kb);
        printRow(outFile,   "Standard", iters_std, time_std, mem_std_kb);

        printRow(std::cout, "In-Place", iters_ip, time_ip, mem_ip_kb);
        printRow(outFile,   "In-Place", iters_ip, time_ip, mem_ip_kb);

        std::cout << std::string(74, '-') << "\n";
        outFile   << std::string(74, '-') << "\n";
    }

    outFile.close();
    std::cout << "\nRisultati del benchmark salvati con successo in '" << output_filename << "'.\n";

    return 0;
}