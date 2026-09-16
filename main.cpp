#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>

#include "Types.hpp"
#include "GridMDP.hpp"
#include "MemoryTracker.hpp"

// Larghezza delle colonne della tabella di benchmark. La somma determina la lunghezza
// delle righe di separazione, che restano così allineate alle colonne anche se una di
// queste viene modificata.
constexpr int COL_N           = 8;
constexpr int COL_VARIANTE    = 14;
constexpr int COL_ITERAZIONI  = 14;
constexpr int COL_TEMPO       = 18;
constexpr int COL_MEMORIA     = 20;
constexpr int TABLE_WIDTH = COL_N + COL_VARIANTE + COL_ITERAZIONI + COL_TEMPO + COL_MEMORIA;

// Struttura per memorizzare le metriche di benchmark
struct BenchmarkResult {
    int iterations{0};
    double time_ms{0.0};
    double memory_extra_kb{0.0};
};

// Funzione generica per misurare l'esecuzione di un algoritmo di risoluzione
template <typename SolverFunc>
BenchmarkResult measurePerformance(SolverFunc solver, ValueMatrix& V, PolicyMatrix& pi) {
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
        std::cerr << "[ERRORE] Impossibile aprire il file di input: " << filename
                  << "\nAssicurati che la cartella 'istanze' contenga tutti i file di input necessari.\n";
        return false;
    }

    if (!(file >> N) || N <= 0) {
        std::cerr << "[ERRORE] " << filename << ": dimensione N mancante o non valida.\n";
        return false;
    }

    if (!(file >> S.r >> S.c >> G.r >> G.c)) {
        std::cerr << "[ERRORE] " << filename << ": coordinate di Start/Goal mancanti o non valide.\n";
        return false;
    }

    grid.assign(N, std::vector<int>(N, 0));

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (!(file >> grid[r][c])) {
                std::cerr << "[ERRORE] " << filename << ": griglia incompleta, attese " << N * N
                          << " celle.\n";
                return false;
            }
            if (grid[r][c] != static_cast<int>(CellType::LIBERA) &&
                grid[r][c] != static_cast<int>(CellType::OSTACOLO)) {
                std::cerr << "[ERRORE] " << filename << ": valore di cella non valido in (" << r
                          << "," << c << "): atteso 0 (LIBERA) o 1 (OSTACOLO).\n";
                return false;
            }
        }
    }

    auto inBounds = [N](Position p) { return p.r >= 0 && p.r < N && p.c >= 0 && p.c < N; };

    if (!inBounds(S) || !inBounds(G)) {
        std::cerr << "[ERRORE] " << filename << ": Start (" << S.r << "," << S.c << ") o Goal ("
                  << G.r << "," << G.c << ") fuori dalla griglia " << N << "x" << N << ".\n";
        return false;
    }

    if (grid[S.r][S.c] == static_cast<int>(CellType::OSTACOLO)) {
        std::cerr << "[ERRORE] " << filename << ": la cella di Start e' un ostacolo.\n";
        return false;
    }

    if (grid[G.r][G.c] == static_cast<int>(CellType::OSTACOLO)) {
        std::cerr << "[ERRORE] " << filename << ": la cella di Goal e' un ostacolo.\n";
        return false;
    }

    return true;
}

void printHeader(std::ostream& os) {
    const std::string titolo = "BENCHMARK SPERIMENTALE: COMPITO 3";
    const int padding = (TABLE_WIDTH - static_cast<int>(titolo.size())) / 2;

    os << std::string(TABLE_WIDTH, '=') << "\n";
    os << std::string(padding > 0 ? padding : 0, ' ') << titolo << "\n";
    os << std::string(TABLE_WIDTH, '=') << "\n";
    os << std::left
       << std::setw(COL_N)          << "N"
       << std::setw(COL_VARIANTE)   << "Variante"
       << std::setw(COL_ITERAZIONI) << "Iterazioni"
       << std::setw(COL_TEMPO)      << "Tempo (ms)"
       << std::setw(COL_MEMORIA)    << "Memoria Extra (KB)" << "\n";
    os << std::string(TABLE_WIDTH, '-') << "\n";
}

void printRow(std::ostream& os, int N, const std::string& variant, const BenchmarkResult& res) {
    os << std::left
       << std::setw(COL_N)          << N
       << std::setw(COL_VARIANTE)   << variant
       << std::setw(COL_ITERAZIONI) << res.iterations
       << std::setw(COL_TEMPO)      << std::fixed << std::setprecision(3) << res.time_ms
       << std::setw(COL_MEMORIA)    << std::fixed << std::setprecision(2) << res.memory_extra_kb << "\n";
}

// Funzione di utilità per salvare la soluzione completa di una griglia su un file dedicato
void saveSolutionToFile(
    const std::string& filename,
    const std::string& variant_name,
    int N, Position S, Position G,
    const Grid& grid,
    const ValueMatrix& V,
    const PolicyMatrix& pi,
    const std::vector<Position>& path)
{
    std::ofstream os(filename);
    if (!os.is_open()) {
        std::cerr << "[ERRORE] Impossibile salvare la soluzione su " << filename << "\n";
        return;
    }

    auto isObstacle = [&](int r, int c) {
        return grid[r][c] == static_cast<int>(CellType::OSTACOLO);
    };

    os << "========================================================================================\n";
    os << "   RISOLUZIONE MDP E CAMMINATA OTTIMA (" << variant_name << "): " << filename << "\n";
    os << "   Dimensioni Griglia: " << N << "x" << N << " | Start: (" << S.r << "," << S.c
       << ") | Goal: (" << G.r << "," << G.c << ")\n";
    os << "========================================================================================\n\n";

    // 1. Matrice dei Valori V* ('#' marca le celle ostacolo, prive di valore)
    os << "--- MATRICE DEI VALORI V* ('#'=Ostacolo) ---\n";
    os << std::fixed << std::setprecision(2);
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (isObstacle(r, c)) {
                os << std::setw(8) << "#" << " ";
            } else {
                os << std::setw(8) << V[r][c] << " ";
            }
        }
        os << "\n";
    }

    // 2. Politica Ottima pi* ('#'=Ostacolo, 'G'=stato terminale, privo di azione)
    os << "\n--- POLITICA OTTIMA pi* ('#'=Ostacolo, 'G'=Goal) ---\n";
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (isObstacle(r, c)) {
                os << "  #  ";
            } else if (Position{r, c} == G) {
                os << "  G  ";
            } else {
                os << "  " << actionToString(pi[r][c]) << "  ";
            }
        }
        os << "\n";
    }

    // 3. Griglia con il Percorso del Robot
    os << "\n--- GRIGLIA CON PERCORSO ROBOT ('S'=Start, 'G'=Goal, '*'=Percorso, '#'=Ostacolo) ---\n";
    std::vector<std::vector<char>> display(N, std::vector<char>(N, '.'));

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (isObstacle(r, c)) display[r][c] = '#';
        }
    }

    for (const auto& p : path) {
        display[p.r][p.c] = '*';
    }

    display[S.r][S.c] = 'S';
    display[G.r][G.c] = 'G';

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            os << display[r][c] << " ";
        }
        os << "\n";
    }

    // 4. Lista dettagliata delle coordinate del cammino
    const std::size_t cells = path.size();
    const std::size_t moves = (cells > 0) ? cells - 1 : 0;
    os << "\n--- SEQUENZA COORDINATE DEL PERCORSO (" << moves << " passi, " << cells
       << " celle) ---\n";
    for (size_t i = 0; i < path.size(); ++i) {
        os << "(" << path[i].r << "," << path[i].c << ")" << (i + 1 == path.size() ? "" : " -> ");
        if ((i + 1) % 10 == 0) os << "\n";
    }
    os << "\n";

    os.close();
}

int main() {
    // Percorsi dei file d'istanza presi dalla cartella 'istanze/'
    const std::vector<std::string> input_files = {
        "istanze/grid_1.txt", "istanze/grid_2.txt", "istanze/grid_3.txt", "istanze/grid_4.txt", "istanze/grid_5.txt",
        "istanze/grid_6.txt", "istanze/grid_7.txt", "istanze/grid_8.txt", "istanze/grid_9.txt", "istanze/grid_10.txt"
    };

    // Salva il report del benchmark nella cartella "soluzioni/"
    const std::string output_benchmark = "soluzioni/risultati_benchmark.txt";
    std::ofstream outFile(output_benchmark);

    if (!outFile.is_open()) {
        std::cerr << "[ERRORE] Impossibile creare il file di output: " << output_benchmark
                  << "\nAssicurati che la cartella 'soluzioni' esista nella directory di progetto.\n";
        return 1;
    }

    constexpr double gamma = 1.0;
    constexpr double eps = 1e-4;

    std::cout << "Esecuzione benchmark e generazione soluzioni in corso...\n\n";

    printHeader(outFile);

    for (const auto& file_path : input_files) {
        int N;
        Position S, G;
        Grid grid;

        if (!loadGridFromFile(file_path, N, S, G, grid)) {
            continue;
        }

        ValueMatrix V_std(N, std::vector<double>(N, 0.0));
        PolicyMatrix pi_std(N, std::vector<Action>(N, Action::NONE));

        // Misurazione Variante Standard
        auto res_std = measurePerformance([&](ValueMatrix& V, PolicyMatrix& pi) {
            return gridValueIteration(grid, N, G, gamma, eps, V, pi);
        }, V_std, pi_std);

        ValueMatrix V_ip(N, std::vector<double>(N, 0.0));
        PolicyMatrix pi_ip(N, std::vector<Action>(N, Action::NONE));

        // Misurazione Variante In-Place
        auto res_ip = measurePerformance([&](ValueMatrix& V, PolicyMatrix& pi) {
            return gridValueIterationInPlace(grid, N, G, gamma, eps, V, pi);
        }, V_ip, pi_ip);

        // Scrittura dei risultati nel file di benchmark
        printRow(outFile, N, "Standard", res_std);
        printRow(outFile, N, "In-Place", res_ip);
        outFile << std::string(TABLE_WIDTH, '-') << "\n";

        // 1. Costruzione percorso e verifica - STANDARD
        std::vector<Position> path_std = constructOptimalPath(S, G, pi_std, grid, N);
        bool goal_std = (!path_std.empty() && path_std.back() == G);

        // 2. Costruzione percorso e verifica - IN-PLACE
        std::vector<Position> path_ip = constructOptimalPath(S, G, pi_ip, grid, N);
        bool goal_ip = (!path_ip.empty() && path_ip.back() == G);

        std::string base_filename = file_path.substr(file_path.find_last_of("/\\") + 1);

        // Stampa dettagliata a console dello stato per entrambe le varianti
        std::cout << "Istanza " << base_filename << " (N = " << N << "):\n";
        if (goal_std) {
            std::cout << "  - Standard : [OK] Goal RAGGIUNTO in " << path_std.size() - 1 << " passi.\n";
        } else {
            std::cout << "  - Standard : [AVVISO] Goal NON RAGGIUNTO!\n";
        }

        if (goal_ip) {
            std::cout << "  - In-Place : [OK] Goal RAGGIUNTO in " << path_ip.size() - 1 << " passi.\n";
        } else {
            std::cout << "  - In-Place : [AVVISO] Goal NON RAGGIUNTO!\n";
        }
        std::cout << "\n";

        // Salvataggio dei file per entrambe le soluzioni
        saveSolutionToFile("soluzioni/soluzione_std_" + base_filename, "STANDARD", N, S, G, grid, V_std, pi_std, path_std);
        saveSolutionToFile("soluzioni/soluzione_ip_" + base_filename, "IN-PLACE", N, S, G, grid, V_ip, pi_ip, path_ip);
    }

    outFile.close();

    std::cout << "[COMPLETATO CON SUCCESSO]\n";
    std::cout << "- Istanze lette da: 'istanze/'\n";
    std::cout << "- Tabella benchmark salvata in: 'soluzioni/risultati_benchmark.txt'\n";
    std::cout << "- Soluzioni salvate in: 'soluzioni/soluzione_std_grid_X.txt' e 'soluzioni/soluzione_ip_grid_X.txt'\n";

    return 0;
}
