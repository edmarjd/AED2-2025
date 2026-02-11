#ifndef GRAFO_HPP
#define GRAFO_HPP

#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <climits>
#include <fstream>
#include <unordered_map>
#include "json.hpp" // Certifique-se de ter o json.hpp na pasta

using json = nlohmann::json;

struct Aresta {
    int destino;
    int peso;
};

typedef std::pair<long long, int> parDI;

class Grafo {
private:
    int V;
    std::vector<std::list<Aresta>> adj;
    
    // Mapeamento: ID do JSON (long long) -> Índice do Vetor (int)
    std::unordered_map<long long, int> id_para_indice;
    int proximo_indice;

    static long long extrairID(const json& valor) {
        if (valor.is_string())
            return std::stoll(valor.get<std::string>());
        return valor.get<long long>();
    }


public:
    Grafo(int num_vertices) : V(num_vertices), proximo_indice(0) {
        adj.resize(V);
    }

    // Retorna o índice interno (0 a V-1) para um ID do OSMNX
    int obterOuCriarIndice(long long id_original) {
        if (id_para_indice.find(id_original) == id_para_indice.end()) {
            if (proximo_indice >= V) return -1; // Proteção contra estouro
            id_para_indice[id_original] = proximo_indice++;
        }
        return id_para_indice[id_original];
    }

    int obterIndice(long long id_original) {
    auto it = id_para_indice.find(id_original);
    if (it == id_para_indice.end()) return -1;
    return it->second;
    }


    void adicionarAresta(long long u_id, long long v_id, int peso) {
        int u = obterOuCriarIndice(u_id);
        int v = obterOuCriarIndice(v_id);

        if (u != -1 && v != -1) {
            adj[u].push_back({v, peso});
        }
    }

    // Dijkstra que retorna as distâncias a partir de um ID original do JSON
    std::vector<long long> executarDijkstra(long long id_origem) {
    int origem = obterIndice(id_origem);
    std::vector<long long> dist(V, LLONG_MAX);

    if (origem == -1) return dist;

    std::priority_queue<parDI, std::vector<parDI>, std::greater<parDI>> pq;
    dist[origem] = 0;
    pq.push({0, origem});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

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


    // Função estática para carregar o arquivo edges.json
    static Grafo* carregarDeJSON(const std::string& caminho_edges, int total_nodos) {
        std::ifstream arquivo(caminho_edges);
        if (!arquivo.is_open()) return nullptr;

        json dados_json;
        arquivo >> dados_json;

        Grafo* g = new Grafo(total_nodos);

        // Percorre as arestas conforme a estrutura do OSMNX
        for (const auto& item : dados_json) {
        // Usa a função de extração para garantir que u, v e peso sejam lidos
        long long u = extrairID(item["u"]);
        long long v = extrairID(item["v"]);
        int peso = (int)item["length"]; 
    
        g->adicionarAresta(u, v, peso);
    }

        return g;
    }
};

#endif