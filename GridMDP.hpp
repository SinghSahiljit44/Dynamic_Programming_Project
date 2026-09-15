#pragma once

#include "Types.hpp"
#include <vector>

// --- Funzioni di Transizione e Modellazione MDP ---

bool isValid(const Grid& grid, int r, int c, int N);
Position getNextState(const Grid& grid, Position s, Action a, int N);
double getReward(Position s, Action a, Position s_next, Position G);

// --- Algoritmi di Risoluzione (Value Iteration) ---

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
