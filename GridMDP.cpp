#include "GridMDP.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <limits>
#include <iomanip>

// Spostamenti relativi per NORD, SUD, EST, OVEST (da leggere per colonna, es Nord è dato da DR[0], DC[0])
constexpr int DR[4] = {-1, 1, 0, 0};
constexpr int DC[4] = {0, 0, 1, -1};

namespace {

// Azzera le tabelle V e pi senza allocare vettori temporanei: usare
// V.assign(N, std::vector<double>(N, 0.0)) costruirebbe una riga temporanea di N
// double, che falserebbe la misura della memoria ausiliaria della variante in-place
// facendola apparire Theta(N) anziché Theta(1).
void resetTables(int N, ValueMatrix& V, PolicyMatrix& pi) {
    V.resize(N);
    pi.resize(N);
    for (int r = 0; r < N; ++r) {
        V[r].resize(N);
        pi[r].resize(N);
        std::fill(V[r].begin(), V[r].end(), 0.0);
        std::fill(pi[r].begin(), pi[r].end(), Action::NONE);
    }
}

void reportNonConvergenza(const char* variante, long long max_iterations) {
    std::cerr << "[ERRORE] " << variante << ": nessuna convergenza entro " << max_iterations
              << " iterazioni.\n         Con gamma = 1 cio' accade quando esiste una cella libera "
                 "da cui il Goal e' irraggiungibile\n         (il suo valore diverge a -infinito).\n";
}

} // namespace

bool isValid(const Grid& grid, int r, int c, int N) {
    return (r >= 0 && r < N && c >= 0 && c < N && grid[r][c] != static_cast<int>(CellType::OSTACOLO));
}

Position getNextState(const Grid& grid, Position s, Action a, int N) {
    int nr = s.r + DR[static_cast<int>(a)];
    int nc = s.c + DC[static_cast<int>(a)];
    if (isValid(grid, nr, nc, N)) {
        return {nr, nc};
    }
    return s; // Urto: il robot rimane nella cella corrente
}

// L'azione a non compare nel corpo perché il premio dipende da s ed s' = delta(s, a);
// il parametro è mantenuto per aderenza alla definizione R(s, a) data nella relazione.
double getReward(Position s, [[maybe_unused]] Action a, Position s_next, Position G) {
    if (s_next == s) return -10.0;     // Urto contro ostacolo o bordo
    if (s_next == G) return 100.0;     // Raggiungimento della cella Goal
    return -1.0;                       // Passo su cella libera
}

int gridValueIteration(
    const Grid& grid, int N, Position G,
    double gamma, double eps,
    ValueMatrix& V, PolicyMatrix& pi)
{
    resetTables(N, V, pi);

    double delta = std::numeric_limits<double>::infinity();
    int iterations = 0;
    const long long max_iterations = 1LL * N * N + 1;

    while (delta >= eps) {
        if (iterations >= max_iterations) {
            reportNonConvergenza("GRID-VALUE-ITERATION", max_iterations);
            break;
        }

        delta = 0.0;
        ValueMatrix V_old = V;
        iterations++;

        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                Position s = {r, c};
                if (grid[r][c] == static_cast<int>(CellType::OSTACOLO) || s == G) continue;

                double max_q = -std::numeric_limits<double>::infinity();
                Action best_a = Action::NONE;

                for (int a = 0; a < 4; ++a) {
                    Action action = static_cast<Action>(a);
                    Position s_prime = getNextState(grid, s, action, N);
                    double R = getReward(s, action, s_prime, G);

                    double q = R + gamma * V_old[s_prime.r][s_prime.c];
                    if (q > max_q) {
                        max_q = q;
                        best_a = action;
                    }
                }

                V[r][c] = max_q;
                pi[r][c] = best_a;
                delta = std::max(delta, std::abs(V_old[r][c] - V[r][c]));
            }
        }
    }
    return iterations;
}

int gridValueIterationInPlace(
    const Grid& grid, int N, Position G,
    double gamma, double eps,
    ValueMatrix& V, PolicyMatrix& pi)
{
    resetTables(N, V, pi);

    double delta = std::numeric_limits<double>::infinity();
    int iterations = 0;
    const long long max_iterations = 1LL * N * N + 1;

    while (delta >= eps) {
        if (iterations >= max_iterations) {
            reportNonConvergenza("GRID-VALUE-ITERATION-IN-PLACE", max_iterations);
            break;
        }

        delta = 0.0;
        iterations++;

        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                Position s = {r, c};
                if (grid[r][c] == static_cast<int>(CellType::OSTACOLO) || s == G) continue;

                double v_old = V[r][c];
                double max_q = -std::numeric_limits<double>::infinity();
                Action best_a = Action::NONE;

                for (int a = 0; a < 4; ++a) {
                    Action action = static_cast<Action>(a);
                    Position s_prime = getNextState(grid, s, action, N);
                    double R = getReward(s, action, s_prime, G);

                    double q = R + gamma * V[s_prime.r][s_prime.c];
                    if (q > max_q) {
                        max_q = q;
                        best_a = action;
                    }
                }

                V[r][c] = max_q;
                pi[r][c] = best_a;
                delta = std::max(delta, std::abs(v_old - V[r][c]));
            }
        }
    }
    return iterations;
}

std::vector<Position> constructOptimalPath(
    Position S, Position G,
    const PolicyMatrix& pi,
    const Grid& grid, int N)
{
    std::vector<Position> path;
    Position s_curr = S;
    path.push_back(s_curr);

    long long steps = 0;
    const long long max_steps = 1LL * N * N;

    while (s_curr != G && steps < max_steps) {
        Action a_star = pi[s_curr.r][s_curr.c];
        if (a_star == Action::NONE) {
            std::cerr << "[ERRORE] Impossibile raggiungere il goal: mossa ottima non definita (" << s_curr.r << ", " << s_curr.c << ")\n";
            return {};
        }

        Position s_next = getNextState(grid, s_curr, a_star, N);
        if (s_next == s_curr) {
            std::cerr << "[ERRORE] La mossa ottima conduce contro un ostacolo o un bordo (" << s_curr.r << ", " << s_curr.c << ")\n";
            return {};
        }

        path.push_back(s_next);
        s_curr = s_next;
        steps++;
    }

    if (s_curr != G) {
        std::cerr << "[ERRORE] Goal non raggiungibile entro il numero massimo di passi consentiti (" << max_steps << ")\n";
        return {};
    }

    return path;
}
