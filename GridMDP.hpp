#pragma once

#include "Types.hpp"
#include <vector>

// Type Aliases per leggibilità
using Grid = std::vector<std::vector<int>>;
using ValueMatrix = std::vector<std::vector<double>>;
using PolicyMatrix = std::vector<std::vector<Action>>;

// --- Funzioni di Transizione e Modellazione MDP ---

bool isValid(const Grid& grid, int r, int c, int N);
Position getNextState(const Grid& grid, Position s, Action a, int N);
double getReward(Position s, Action a, Position s_next, Position G);

// --- Algoritmi di Risoluzione (Value Iteration) ---

// Algoritmo 1: Standard Jacobi Value Iteration (restituisce il numero di iterazioni K)
int gridValueIteration(
    const Grid& grid, 
    int N, 
    Position G,
    double gamma, 
    double eps,
    ValueMatrix& V, 
    PolicyMatrix& pi
);

// Algoritmo 1 (Variante): In-Place Gauss-Seidel Value Iteration
int gridValueIterationInPlace(
    const Grid& grid, 
    int N, 
    Position G,
    double gamma, 
    double eps,
    ValueMatrix& V, 
    PolicyMatrix& pi
);

// --- Algoritmo 2: Costruzione della soluzione ottima ---

std::vector<Position> constructOptimalPath(
    Position S, 
    Position G,
    const PolicyMatrix& pi,
    const Grid& grid, 
    int N
);

// --- Utility di Visualizzazione ---

void printValueMatrix(const ValueMatrix& V, int N);
void printPolicyMatrix(const PolicyMatrix& pi, int N);
void printGridWithPath(
    const Grid& grid, 
    int N, 
    const std::vector<Position>& path, 
    Position S, 
    Position G
);