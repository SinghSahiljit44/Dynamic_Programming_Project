# Il Robot nel GridWorld

Progetto di Algoritmi e Strutture Dati (A.A. 2025/2026) — Università degli Studi di Brescia.

Il problema del robot in un GridWorld viene formalizzato come MDP deterministico e risolto
con la programmazione dinamica (Value Iteration), confrontando la variante sincrona
*standard* con la variante asincrona *in-place* su tempo, iterazioni e memoria ausiliaria.

## Struttura del progetto

| Percorso | Contenuto |
|---|---|
| `Types.hpp` | Tipi di base: `CellType`, `Action`, `Position`, alias delle matrici |
| `GridMDP.hpp` / `.cpp` | Modello MDP e i due algoritmi di Value Iteration + ricostruzione del cammino |
| `MemoryTracker.hpp` / `.cpp` | Sostituzione di `new`/`delete` per misurare la memoria ausiliaria di picco |
| `main.cpp` | Caricamento istanze, benchmark, salvataggio dei risultati |
| `GenerateInstance.cpp` | Programma separato che genera le istanze in `istanze/` (già eseguito) |
| `istanze/` | Le 10 griglie di prova, da `N = 20` a `N = 120` |
| `soluzioni/` | Uscite del programma: `V*`, `π*`, cammino e tabella di benchmark |
| `relazione/` | Sorgenti LaTeX della relazione |

## Compilazione ed esecuzione

L'eseguibile di risoluzione richiede tre file. **Le ottimizzazioni sono necessarie**: senza,
i tempi misurati sono dominati dalle astrazioni della libreria standard e non riflettono
l'andamento asintotico.

```bash
g++ -std=c++17 -O2 -o gridmdp main.cpp GridMDP.cpp MemoryTracker.cpp
./gridmdp
```

Su MSVC l'equivalente è una build **Release**:

```
cl /std:c++17 /O2 /EHsc main.cpp GridMDP.cpp MemoryTracker.cpp /Fe:gridmdp.exe
```

Il programma va lanciato dalla radice del progetto: legge da `istanze/` e scrive in
`soluzioni/`. Produce, per ognuna delle 10 istanze, `soluzione_std_grid_X.txt` e
`soluzione_ip_grid_X.txt` (matrice `V*`, politica `π*`, griglia con il cammino e sequenza
delle coordinate), più la tabella riassuntiva `risultati_benchmark.txt`.

Per rigenerare le istanze da zero (sovrascrive `istanze/`):

```bash
g++ -std=c++17 -O2 -o genera GenerateInstance.cpp && ./genera
```

## Compilazione della relazione

```bash
cd relazione && pdflatex main.tex && pdflatex main.tex
```

I dati sperimentali citati nella relazione — tabelle, coordinate dei grafici e valori
riportati nel testo — sono raccolti in `relazione/dati-benchmark.tex`, generato
automaticamente da uno script di supporto a partire da `soluzioni/risultati_benchmark.txt`:
il file non va quindi modificato a mano.

## Nota sul fattore di sconto

Il progetto usa `γ = 1`, che rende il cammino ottimo esattamente quello di lunghezza minima.
Con `γ = 1` l'operatore di Bellman non è però una contrazione: se un'istanza contenesse una
cella libera da cui il Goal è irraggiungibile, il suo valore divergerebbe a `-∞`. Entrambi
gli algoritmi sono perciò protetti da un limite di `N² + 1` iterazioni e segnalano il caso
su `stderr` anziché entrare in ciclo infinito.

## Autori

Singh Sahiljit (740552), Lublanis Matteo (736418)
