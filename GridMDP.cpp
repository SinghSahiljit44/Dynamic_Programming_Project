#include "GridMDP.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <limits>
#include <iomanip>

// Spostamenti relativi (dr, dc) per NORD, SUD, EST, OVEST
constexpr int DR[4] = {-1, 1, 0, 0};
constexpr int DC[4] = {0, 0, 1, -1};

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

double getReward(Position s, Action a, Position s_next, Position G) {
    if (s_next == s) return -10.0;     // Urto contro ostacolo o bordo
    if (s_next == G) return 100.0;     // Raggiungimento della cella Goal
    return -1.0;                       // Passo su cella libera
}

int gridValueIteration(
    const Grid& grid, int N, Position G,
    double gamma, double eps,
    ValueMatrix& V, PolicyMatrix& pi) 
{
    V.assign(N, std::vector<double>(N, 0.0));
    pi.assign(N, std::vector<Action>(N, Action::NONE));

    double delta = std::numeric_limits<double>::infinity();
    int iterations = 0;

    while (delta >= eps) {
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
    V.assign(N, std::vector<double>(N, 0.0));
    pi.assign(N, std::vector<Action>(N, Action::NONE));

    double delta = std::numeric_limits<double>::infinity();
    int iterations = 0;

    while (delta >= eps) {
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

    int steps = 0;
    int max_steps = N * N;

    while (s_curr != G && steps < max_steps) {
        Action a_star = pi[s_curr.r][s_curr.c];
        if (a_star == Action::NONE) {
            std::cerr << "[ERRORE] Azione ottima non definita per la cella (" << s_curr.r << ", " << s_curr.c << ")\n";
            return {};
        }

        Position s_next = getNextState(grid, s_curr, a_star, N);
        if (s_next == s_curr) {
            std::cerr << "[ERRORE] Stallo rilevato nella cella (" << s_curr.r << ", " << s_curr.c << ")\n";
            return {};
        }

        path.push_back(s_next);
        s_curr = s_next;
        steps++;
    }

    if (s_curr != G) {
        std::cerr << "[ERRORE] Goal non raggiunto entro il numero massimo di passi (" << max_steps << ")\n";
        return {};
    }

    return path;
}

void printValueMatrix(const ValueMatrix& V, int N) {
    std::cout << std::fixed << std::setprecision(2);
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            std::cout << std::setw(8) << V[r][c] << " ";
        }
        std::cout << "\n";
    }
}

void printPolicyMatrix(const PolicyMatrix& pi, int N) {
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            // actionToString restituisce un std::string_view
            std::cout << "  " << actionToString(pi[r][c]) << "  ";
        }
        std::cout << "\n";
    }
}

void printGridWithPath(const Grid& grid, int N, const std::vector<Position>& path, Position S, Position G) {
    std::vector<std::vector<char>> display(N, std::vector<char>(N, '.'));

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (grid[r][c] == static_cast<int>(CellType::OSTACOLO)) display[r][c] = '#';
        }
    }

    for (const auto& p : path) {
        display[p.r][p.c] = '*';
    }

    display[S.r][S.c] = 'S';
    display[G.r][G.c] = 'G';

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            std::cout << display[r][c] << " ";
        }
        std::cout << "\n";
    }
}