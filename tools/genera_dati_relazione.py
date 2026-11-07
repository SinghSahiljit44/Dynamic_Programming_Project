#!/usr/bin/env python3

import math
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "soluzioni", "risultati_benchmark.txt")
ISTANZE = os.path.join(ROOT, "istanze")
DST = os.path.join(ROOT, "relazione", "dati-benchmark.tex")

ROW = re.compile(r"^\s*(\d+)\s+(Standard|In-Place)\s+(\d+)\s+([\d.]+)\s+([\d.]+)\s*$")

DR = (-1, 1, 0, 0)
DC = (0, 0, 1, -1)


def analizza_istanze():

    from collections import deque

    dati = {}
    for nome in sorted(os.listdir(ISTANZE)) if os.path.isdir(ISTANZE) else []:
        if not nome.startswith("grid_") or not nome.endswith(".txt"):
            continue
        with open(os.path.join(ISTANZE, nome), encoding="utf-8") as f:
            tok = iter(f.read().split())
        try:
            N = int(next(tok))
            next(tok), next(tok)                      # Start: non serve qui
            gr, gc = int(next(tok)), int(next(tok))
            g = [[int(next(tok)) for _ in range(N)] for _ in range(N)]
        except (StopIteration, ValueError):
            print(f"[AVVISO] {nome} illeggibile: saltata.")
            continue

        libere = sum(1 for r in range(N) for c in range(N) if g[r][c] == 0)
        dist = {(gr, gc): 0}
        coda = deque([(gr, gc)])
        while coda:
            r, c = coda.popleft()
            for k in range(4):
                nr, nc = r + DR[k], c + DC[k]
                if 0 <= nr < N and 0 <= nc < N and g[nr][nc] == 0 and (nr, nc) not in dist:
                    dist[(nr, nc)] = dist[(r, c)] + 1
                    coda.append((nr, nc))

        irraggiungibili = libere - len(dist)
        if irraggiungibili:
            print(f"[AVVISO] {nome}: {irraggiungibili} celle libere non raggiungono il Goal; "
                  f"con gamma = 1 la Value Iteration non converge su questa istanza.")
        dati[N] = {"libere": libere, "ecc": max(dist.values()), "irr": irraggiungibili}
    return dati


def it(x, dec):
    return f"{x:.{dec}f}".replace(".", "{,}")


def it_int(n):
    return f"{n:,}".replace(",", ".")


def fit_esponente(xs, ys):
    lx = [math.log(x) for x in xs]
    ly = [math.log(y) for y in ys]
    n = len(lx)
    mx, my = sum(lx) / n, sum(ly) / n
    sxy = sum((a - mx) * (b - my) for a, b in zip(lx, ly))
    sxx = sum((a - mx) ** 2 for a in lx)
    alpha = sxy / sxx
    intercetta = my - alpha * mx
    ss_res = sum((b - (alpha * a + intercetta)) ** 2 for a, b in zip(lx, ly))
    ss_tot = sum((b - my) ** 2 for b in ly)
    r2 = 1.0 - ss_res / ss_tot if ss_tot > 0 else 1.0
    return alpha, r2


def main():
    if not os.path.exists(SRC):
        sys.exit(f"[ERRORE] Manca {SRC}: esegui prima il benchmark (./gridmdp).")

    std, ip, ordine = {}, {}, []
    with open(SRC, encoding="utf-8") as f:
        for line in f:
            m = ROW.match(line)
            if not m:
                continue
            N, variante, K, t, mem = (int(m[1]), m[2], int(m[3]), float(m[4]), float(m[5]))
            (std if variante == "Standard" else ip)[N] = {"K": K, "t": t, "mem": mem}
            if N not in ordine:
                ordine.append(N)

    if not ordine:
        sys.exit(f"[ERRORE] Nessuna riga di dati riconosciuta in {SRC}.")

    mancanti = [N for N in ordine if N not in std or N not in ip]
    if mancanti:
        sys.exit(f"[ERRORE] Dati incompleti per N = {mancanti}.")

    for N in ordine:
        if ip[N]["K"] > std[N]["K"]:
            print(f"[AVVISO] N={N}: K In-Place ({ip[N]['K']}) > K Standard ({std[N]['K']}).")
    lente = [N for N in ordine if ip[N]["t"] > std[N]["t"]]
    if lente:
        print(f"[AVVISO] In-Place piu' lenta della Standard per N = {lente}: "
              f"la Conclusione 3 del Compito 3 va riformulata.")

    pari = [N for N in ordine if ip[N]["K"] == std[N]["K"]]
    ridotte = [N for N in ordine if ip[N]["K"] < std[N]["K"]]

    base = min(ordine)
    top = max(ordine)

    def t_iter(d, N):
        return d[N]["t"] / d[N]["K"]

    rapporto_celle = (top ** 2) / (base ** 2)
    rapporto_tempo = t_iter(std, top) / t_iter(std, base)
    rapporto_mem = std[top]["mem"] / std[base]["mem"]

    alpha_t_std, r2_t_std = fit_esponente(ordine, [t_iter(std, N) for N in ordine])
    alpha_t_ip, r2_t_ip = fit_esponente(ordine, [t_iter(ip, N) for N in ordine])
    alpha_mem, r2_mem = fit_esponente(ordine, [std[N]["mem"] for N in ordine])

    def riduzione(N):
        return 100.0 * (std[N]["K"] - ip[N]["K"]) / std[N]["K"]

    n_grande = max(ridotte) if ridotte else top

    def elenco(ns):
        if not ns:
            return "nessuna"
        if len(ns) == 1:
            return str(ns[0])
        return ", ".join(str(n) for n in ns[:-1]) + " e " + str(ns[-1])

    out = []
    w = out.append
    w("% ============================================================================")
    w("% FILE GENERATO AUTOMATICAMENTE - NON MODIFICARE A MANO")
    w("% Sorgente : soluzioni/risultati_benchmark.txt")
    w("% ============================================================================")
    w("")

    w("\\newcommand{\\benchTabella}{%")
    for N in ordine:
        for etichetta, d in (("Standard", std), ("In-Place", ip)):
            primo = etichetta == "Standard"
            col_n = it_int(N) if primo else ""
            w(f"    {col_n} & {etichetta} & {it_int(N * N)} & {d[N]['K']} & "
              f"{it(d[N]['t'], 3)} & {it(d[N]['mem'], 2)} \\\\"
              + (" \\hline" if not primo else ""))
    w("}")
    w("")

    def coords(d, key, dec=6):
        return " ".join(f"({N}, {d[N][key]:.{dec}f})" for N in ordine)

    def coords_titer(d):
        return " ".join(f"({N}, {t_iter(d, N):.6f})" for N in ordine)

    w("\\newcommand{\\benchCoordTempoIterStd}{" + coords_titer(std) + "}")
    w("\\newcommand{\\benchCoordTempoIterIp}{" + coords_titer(ip) + "}")
    w("\\newcommand{\\benchCoordIterStd}{" + " ".join(f"({N}, {std[N]['K']})" for N in ordine) + "}")
    w("\\newcommand{\\benchCoordIterIp}{" + " ".join(f"({N}, {ip[N]['K']})" for N in ordine) + "}")
    w("\\newcommand{\\benchCoordMemStd}{" + coords(std, "mem", 2) + "}")
    w("\\newcommand{\\benchCoordMemIp}{" + coords(ip, "mem", 2) + "}")
    w("\\newcommand{\\benchAsseX}{" + ", ".join(str(N) for N in ordine) + "}")
    w("")

    w("\\newcommand{\\benchNormTempo}{" + f"{t_iter(std, top):.8f}" + "}")
    w("\\newcommand{\\benchNormMem}{" + f"{std[top]['mem']:.2f}" + "}")
    w("\\newcommand{\\benchNmax}{" + str(top) + "}")
    w("")

    w("% Coppia di riferimento per le verifiche asintotiche")
    w("\\newcommand{\\benchNbase}{" + str(base) + "}")
    w("\\newcommand{\\benchNtop}{" + str(top) + "}")
    w("\\newcommand{\\benchCelleBase}{" + it_int(base * base) + "}")
    w("\\newcommand{\\benchCelleTop}{" + it_int(top * top) + "}")
    w("\\newcommand{\\benchRapportoCelle}{" + it(rapporto_celle, 0) + "}")
    w("")
    w("% Memoria (variante Standard)")
    w("\\newcommand{\\benchMemBase}{" + it(std[base]["mem"], 2) + "}")
    w("\\newcommand{\\benchMemTop}{" + it(std[top]["mem"], 2) + "}")
    w("\\newcommand{\\benchMemTeoricaBase}{" + it(base * base * 8 / 1024, 2) + "}")
    w("\\newcommand{\\benchMemTeoricaTop}{" + it(top * top * 8 / 1024, 2) + "}")
    w("\\newcommand{\\benchMemRapporto}{" + it(rapporto_mem, 2) + "}")
    w("\\newcommand{\\benchMemIpBase}{" + it(ip[base]["mem"], 2) + "}")
    w("\\newcommand{\\benchMemIpTop}{" + it(ip[top]["mem"], 2) + "}")
    w("")
    w("% Tempo per iterazione (variante Standard)")
    w("\\newcommand{\\benchTempoBase}{" + it(std[base]["t"], 3) + "}")
    w("\\newcommand{\\benchTempoTop}{" + it(std[top]["t"], 3) + "}")
    w("\\newcommand{\\benchKbase}{" + str(std[base]["K"]) + "}")
    w("\\newcommand{\\benchKtop}{" + str(std[top]["K"]) + "}")
    w("\\newcommand{\\benchTiterBase}{" + it(t_iter(std, base), 4) + "}")
    w("\\newcommand{\\benchTiterTop}{" + it(t_iter(std, top), 4) + "}")
    w("\\newcommand{\\benchTiterRapporto}{" + it(rapporto_tempo, 2) + "}")
    w("")
    w("% Esponenti stimati con regressione ai minimi quadrati su scala log-log")
    w("\\newcommand{\\benchAlfaTempoStd}{" + it(alpha_t_std, 2) + "}")
    w("\\newcommand{\\benchErreQuadTempoStd}{" + it(r2_t_std, 3) + "}")
    w("\\newcommand{\\benchAlfaTempoIp}{" + it(alpha_t_ip, 2) + "}")
    w("\\newcommand{\\benchErreQuadTempoIp}{" + it(r2_t_ip, 3) + "}")
    w("\\newcommand{\\benchAlfaMem}{" + it(alpha_mem, 2) + "}")
    w("\\newcommand{\\benchErreQuadMem}{" + it(r2_mem, 3) + "}")
    w("")
    w("% Riduzione delle iterazioni grazie all'aggiornamento in-place")
    w("\\newcommand{\\benchKstdBase}{" + str(std[base]["K"]) + "}")
    w("\\newcommand{\\benchKipBase}{" + str(ip[base]["K"]) + "}")
    w("\\newcommand{\\benchRiduzioneBase}{" + it(riduzione(base), 1) + "}")
    w("\\newcommand{\\benchTipBase}{" + it(ip[base]["t"], 3) + "}")
    w("\\newcommand{\\benchNgrande}{" + str(n_grande) + "}")
    w("\\newcommand{\\benchKstdGrande}{" + str(std[n_grande]["K"]) + "}")
    w("\\newcommand{\\benchKipGrande}{" + str(ip[n_grande]["K"]) + "}")
    w("\\newcommand{\\benchRiduzioneGrande}{" + it(riduzione(n_grande), 1) + "}")
    w("\\newcommand{\\benchTempoGrandeStd}{" + it(std[n_grande]["t"], 3) + "}")
    w("\\newcommand{\\benchTempoGrandeIp}{" + it(ip[n_grande]["t"], 3) + "}")
    w("")
    istanze = analizza_istanze()
    if all(N in istanze for N in ordine):
        righe = []
        esatto = True
        for N in ordine:
            e = istanze[N]
            if std[N]["K"] != e["ecc"] + 1:
                esatto = False
            righe.append(f"    {it_int(N)} & {it_int(N * N)} & {it_int(e['libere'])} & "
                         f"{it_int(e['ecc'])} & {it_int(e['ecc'] + 1)} & {it_int(std[N]['K'])} \\\\")
        w("% Verifica sperimentale del limite K <= |S| (ecc(G) = eccentricita' del Goal)")
        w("\\newcommand{\\benchTabellaEcc}{%")
        for r in righe:
            w(r)
        w("}")
        w("\\newcommand{\\benchEccEsatta}{" + ("si" if esatto else "no") + "}")
        w("\\newcommand{\\benchEccMax}{" + it_int(max(istanze[N]["ecc"] for N in ordine)) + "}")
        w("\\newcommand{\\benchLibereMax}{" + it_int(max(istanze[N]["libere"] for N in ordine)) + "}")
        w("")
        if not esatto:
            print("[AVVISO] K_Standard non coincide piu' con ecc(G) + 1 su tutte le istanze: "
                  "rivedere l'osservazione nel Compito 3.")
    else:
        sys.exit("[ERRORE] Impossibile analizzare le istanze in istanze/.")

    w("% Elenchi di istanze")
    w("\\newcommand{\\benchIstanzeKpari}{" + elenco(pari) + "}")
    w("\\newcommand{\\benchIstanzeKridotto}{" + elenco(ridotte) + "}")
    w("\\newcommand{\\benchIstanzeIpLenta}{" + elenco(lente) + "}")
    w("\\newcommand{\\benchNistanze}{" + str(len(ordine)) + "}")
    w("")

    with open(DST, "w", encoding="utf-8") as f:
        f.write("\n".join(out) + "\n")

    print(f"Generato {os.path.relpath(DST, ROOT)} da {os.path.relpath(SRC, ROOT)} "
          f"({len(ordine)} istanze).")


if __name__ == "__main__":
    main()
