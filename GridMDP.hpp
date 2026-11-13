#pragma once

#include "Types.hpp"
#include <vector>

// --- Funzioni di Transizione e Modellazione MDP ---

bool isValid(const Grid& grid, int r, int c, int N);
Position getNextState(const Grid& grid, Position s, Action a, int N);
double getReward(Position s, Action a, Position s_next, Position G);

// --- Algoritmi di Risoluzione (Value Iteration) ---
//
// Entrambe le varianti restituiscono il numero K di iterazioni del ciclo while.
// Con gamma = 1 l'operatore di Bellman non e' una contrazione: se esiste anche una
// sola cella libera da cui il Goal e' irraggiungibile il suo valore divergerebbe a
// -infinito e il ciclo non terminerebbe mai. Per questo entrambe le funzioni sono
// protette dal limite di sicurezza max_iterations = N^2 + 1 (nessuna istanza con
// Goal raggiungibile da ogni cella puo' richiedere piu' di |S| <= N^2 iterazioni).
// Se il limite viene raggiunto viene segnalato un errore su std::cerr.

int gridValueIteration(
    const Grid& grid,
    int N,
    Position G,
    double gamma,
    double eps,
    ValueMatrix& V,
    PolicyMatrix& pi
);

int gridValueIterationInPlace(
    const Grid& grid,
    int N,
    Position G,
    double gamma,
    double eps,
    ValueMatrix& V,
    PolicyMatrix& pi
);

std::vector<Position> constructOptimalPath(
    Position S,
    Position G,
    const PolicyMatrix& pi,
    const Grid& grid,
    int N
);
