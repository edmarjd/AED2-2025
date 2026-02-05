#ifndef GRAFO_HPP
#define GRAFO_HPP

#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <climits>
#include <fstream>

struct Aresta {
    int destino;
    int peso;
};

// Par para a fila de prioridade: <distancia, vertice>
typedef std::pair<int, int> parDI;

class Grafo {
private:
    int V;
    std::vector<std::list<Aresta>> adj;

public:
    Grafo(int V) : V(V) {
        adj.resize(V);
    }

    void adicionarAresta(int u, int v, int peso) {
        if (u >= 0 && u < V) {
            adj[u].push_back({v, peso});
        }
    }

    // Algoritmo de Dijkstra isolado
    std::vector<int> executarDijkstra(int origem) {
        // Min-heap para garantir que o menor caminho saia primeiro
        std::priority_queue<parDI, std::vector<parDI>, std::greater<parDI>> pq;
        std::vector<int> dist(V, INT_MAX);

        dist[origem] = 0;
        pq.push({0, origem});

        while (!pq.empty()) {
            int d = pq.top().first;
            int u = pq.top().second;
            pq.pop();

            // Se já encontramos um caminho menor para u, ignoramos esta entrada
            if (d > dist[u]) continue;

            for (const auto& aresta : adj[u]) {
                if (dist[aresta.destino] > dist[u] + aresta.peso) {
                    dist[aresta.destino] = dist[u] + aresta.peso;
                    pq.push({dist[aresta.destino], aresta.destino});
                }
            }
        }
        return dist;
    }

    void imprimirLista() {
        for (int i = 0; i < V; ++i) {
            std::cout << i << ":";
            for (const auto& a : adj[i]) 
                std::cout << " -> (" << a.destino << ", w:" << a.peso << ")";
            std::cout << "\n";
        }
    }

    int getV() const { return V; }
};

#endif