#include <iostream>
#include <fstream>
#include <vector>
#include <string>

void saveGridToFile(const std::string& filename, int N, std::pair<int,int> S, std::pair<int,int> G, const std::vector<std::vector<int>>& grid) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Errore nella creazione di " << filename << "\n";
        return;
    }
    outFile << N << "\n";
    outFile << S.first << " " << S.second << "\n";
    outFile << G.first << " " << G.second << "\n";

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            outFile << grid[r][c] << (c == N - 1 ? "" : " ");
        }
        outFile << "\n";
    }
    outFile.close();
    std::cout << "Generato: " << filename << " (N = " << N << ")\n";
}

// 1. Serpentina / Snake
void generateSnake(int N, const std::string& filename) {
    std::vector<std::vector<int>> grid(N, std::vector<int>(N, 0));
    for (int r = 2; r < N - 2; r += 3) {
        bool open_left = ((r / 3) % 2 == 0);
        for (int c = 0; c < N; ++c) {
            if ((open_left && c > 1) || (!open_left && c < N - 2)) {
                grid[r][c] = 1;
            }
        }
    }
    saveGridToFile(filename, N, {0, 0}, {N - 1, N - 1}, grid);
}

// 2. Stanze suddivise (Griglia K x K)
void generateRooms(int N, int rooms_per_side, const std::string& filename) {
    std::vector<std::vector<int>> grid(N, std::vector<int>(N, 0));
    int step = N / rooms_per_side;

    for (int i = 1; i < rooms_per_side; ++i) {
        int wall_pos = i * step;
        for (int k = 0; k < N; ++k) {
            grid[wall_pos][k] = 1;
            grid[k][wall_pos] = 1;
        }
    }
    // Apertura porte nei muri
    for (int i = 0; i < rooms_per_side; ++i) {
        for (int j = 0; j < rooms_per_side; ++j) {
            if (i < rooms_per_side - 1) grid[(i + 1) * step][j * step + step / 2] = 0;
            if (j < rooms_per_side - 1) grid[i * step + step / 2][(j + 1) * step] = 0;
        }
    }
    saveGridToFile(filename, N, {0, 0}, {N - 1, N - 1}, grid);
}

// 3. Spirale Concentrica
void generateSpiral(int N, const std::string& filename) {
    std::vector<std::vector<int>> grid(N, std::vector<int>(N, 0));
    int layers = N / 4;
    for (int l = 1; l <= layers; ++l) {
        int top = 2 * l, bottom = N - 1 - 2 * l;
        int left = 2 * l, right = N - 1 - 2 * l;
        
        // CONDIFIX: Blocca la spirale prima che soffochi il Goal centrale
        if (top >= bottom - 1 || left >= right - 1) break;

        for (int c = left; c <= right; ++c) grid[top][c] = 1;
        for (int r = top; r <= bottom; ++r) grid[r][right] = 1;
        for (int c = right; c >= left; --c) grid[bottom][c] = 1;
        for (int r = bottom; r >= top + 2; --r) grid[r][left] = 1;
    }
    saveGridToFile(filename, N, {0, 0}, {N / 2, N / 2}, grid);
}

// 4. Labirinto a Pettine
void generateComb(int N, const std::string& filename) {
    std::vector<std::vector<int>> grid(N, std::vector<int>(N, 0));
    for (int c = 2; c < N - 2; c += 2) {
        bool from_top = ((c / 2) % 2 == 0);
        for (int r = 0; r < N; ++r) {
            if ((from_top && r < N - 2) || (!from_top && r > 1)) {
                grid[r][c] = 1;
            }
        }
    }
    saveGridToFile(filename, N, {0, 0}, {0, N - 1}, grid);
}

// 5. Anelli Concentrici
void generateRings(int N, const std::string& filename) {
    std::vector<std::vector<int>> grid(N, std::vector<int>(N, 0));
    int step = 4;
    for (int k = step; k < N / 2 - 1; k += step) {
        int r1 = k, r2 = N - 1 - k;
        int c1 = k, c2 = N - 1 - k;
        for (int i = c1; i <= c2; ++i) { grid[r1][i] = 1; grid[r2][i] = 1; }
        for (int i = r1; i <= r2; ++i) { grid[i][c1] = 1; grid[i][c2] = 1; }
        
        // Porte su lati alternati
        if ((k / step) % 2 == 0) {
            grid[r1][c1 + (c2 - c1) / 2] = 0;
            grid[r2][c1 + (c2 - c1) / 2] = 0;
        } else {
            grid[r1 + (r2 - r1) / 2][c1] = 0;
            grid[r1 + (r2 - r1) / 2][c2] = 0;
        }
    }
    saveGridToFile(filename, N, {0, 0}, {N / 2, N / 2}, grid);
}

// 6. Pilastri Spessi
void generatePillars(int N, const std::string& filename) {
    std::vector<std::vector<int>> grid(N, std::vector<int>(N, 0));
    // CONDIFIX: limite r < N - 3 per non toccare il Goal a (N-1, N-1)
    for (int r = 2; r < N - 3; r += 5) {
        for (int c = 2; c < N - 3; c += 5) {
            for (int dr = 0; dr < 3; ++dr) {
                for (int dc = 0; dc < 3; ++dc) {
                    grid[r + dr][c + dc] = 1;
                }
            }
        }
    }
    saveGridToFile(filename, N, {0, 0}, {N - 1, N - 1}, grid);
}

// 7. Trappole a U
void generateUTraps(int N, const std::string& filename) {
    std::vector<std::vector<int>> grid(N, std::vector<int>(N, 0));
    for (int r = 4; r < N - 4; r += 8) {
        for (int c = 4; c < N - 4; c += 8) {
            for (int i = 0; i < 5; ++i) {
                grid[r + 4][c + i] = 1; // Fondo della U
                grid[r + i][c] = 1;     // Lato sinistro
                grid[r + i][c + 4] = 1; // Lato destro
            }
        }
    }
    saveGridToFile(filename, N, {0, 0}, {N - 1, N - 1}, grid);
}

int main() {
    std::cout << "Avvio generazione istanze di grande dimensione...\n\n";

    generateSnake(20,  "grid_1.txt");
    generateRooms(25,  2, "grid_2.txt");
    generateSpiral(30, "grid_3.txt");
    generateComb(40,   "grid_4.txt");
    generateRings(50,  "grid_5.txt");
    generateRooms(60,  3, "grid_6.txt");
    generatePillars(70, "grid_7.txt");
    generateUTraps(80, "grid_8.txt");
    generateRooms(100, 4, "grid_9.txt");
    generateSnake(120, "grid_10.txt");

    std::cout << "\nTutti i 10 file sono stati generati con successo!\n";
    return 0;
}