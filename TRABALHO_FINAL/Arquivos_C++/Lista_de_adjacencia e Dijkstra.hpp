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
#include <algorithm>

using json = nlohmann::json;

struct Aresta {
    int destino;
    int peso;
};

struct ResultadoDijkstra {
    std::vector<long long> dist;
    std::vector<int> pai;
};


typedef std::pair<long long, int> parDI;

class Grafo {
private:
    int V;
    std::vector<std::list<Aresta>> adj;
    std::vector<long long> indice_para_id;

    
    // Mapeamento: ID do JSON (long long) -> Índice do Vetor (int)
    std::unordered_map<long long, int> id_para_indice;
    int proximo_indice;

    static long long extrairID(const json& valor) {
        if (valor.is_string())
            return std::stoll(valor.get<std::string>());
        return valor.get<long long>();
    }

    inline int extrairPeso(const json& v) {
    if (v.is_array() && !v.empty())
        return (int)v[0].get<double>();

    if (v.is_number())
        return (int)v.get<double>();

    if (v.is_string())
        return (int)std::stod(v.get<std::string>());

    return 1; // fallback
    }



public:

    Grafo(int num_vertices) : V(num_vertices), proximo_indice(0) {
    adj.resize(V);
    indice_para_id.resize(V, -1);
    }


    // Retorna o índice interno (0 a V-1) para um ID do OSMNX
    int obterOuCriarIndice(long long id_original) {
    if (id_para_indice.find(id_original) == id_para_indice.end()) {
        if (proximo_indice >= V) return -1;

        id_para_indice[id_original] = proximo_indice;
        indice_para_id[proximo_indice] = id_original;

        proximo_indice++;
    }
    return id_para_indice[id_original];
    }

    long long obterIdOriginal(int indice) {
    if (indice < 0 || indice >= V) return -1;
    return indice_para_id[indice];

    }


    int obterIndice(long long id_original) {
    auto it = id_para_indice.find(id_original);
    if (it == id_para_indice.end()) return -1;
    return it->second;
    }

    void adicionarNo(long long id) {
        obterOuCriarIndice(id);
    }

    int totalArestas() const {
    int total = 0;
    for (const auto& lista : adj)
        total += lista.size();
    return total;
    }



    void adicionarAresta(long long u_id, long long v_id, int peso) {
        int u = obterOuCriarIndice(u_id);
        int v = obterOuCriarIndice(v_id);

        if (u != -1 && v != -1) {
            adj[u].push_back({v, peso});
        }
    }

    // Dijkstra que retorna as distâncias a partir de um ID original do JSON
    ResultadoDijkstra executarDijkstra(long long id_origem) {
    int origem = obterIndice(id_origem);

    ResultadoDijkstra res;
    res.dist.assign(V, LLONG_MAX);
    res.pai.assign(V, -1);

    if (origem == -1) return res;

    std::priority_queue<parDI, std::vector<parDI>, std::greater<parDI>> pq;

    res.dist[origem] = 0;
    pq.push({0, origem});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > res.dist[u]) continue;

        for (const auto& aresta : adj[u]) {
            if (res.dist[aresta.destino] > res.dist[u] + aresta.peso) {

                res.dist[aresta.destino] = res.dist[u] + aresta.peso;
                res.pai[aresta.destino] = u;

                pq.push({res.dist[aresta.destino], aresta.destino});
            }
        }
    }

    return res;

    }

    // Função estática para carregar o arquivo edges.json
    
    static Grafo* carregarDeJSON(const std::string& caminho_edges, int total_nodos) {
    std::ifstream arquivo(caminho_edges);
    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir edges.json\n";
        return nullptr;
    }

    json dados_json;
    arquivo >> dados_json;

    Grafo* g = new Grafo(total_nodos);

    for (const auto& item : dados_json) {

    if (!item.contains("u") || !item.contains("v") || !item.contains("data"))
        continue;

    const auto& data = item.at("data");

    if (!data.contains("length"))
        continue;

    long long u = extrairID(item.at("u"));
    long long v = extrairID(item.at("v"));
    int peso = (int)data.at("length").get<double>();

    bool oneway = false;
    if (data.contains("oneway"))
        oneway = data.at("oneway").get<bool>();

    g->adicionarAresta(u, v, peso);

    if (!oneway)
        g->adicionarAresta(v, u, peso);
    }
    return g;
}

    std::vector<int> reconstruirCaminho(long long origem_id, long long destino_id, const std::vector<int>& pai) {

    int origem = obterIndice(origem_id);
    int destino = obterIndice(destino_id);

    std::vector<int> caminho;

    if (origem == -1 || destino == -1)
        return caminho;

    for (int v = destino; v != -1; v = pai[v])
        caminho.push_back(v);

    std::reverse(caminho.begin(), caminho.end());

    if (!caminho.empty() && caminho.front() == origem)
        return caminho;

    return {};
}

    std::vector<long long> reconstruirCaminhoIDs(long long origem_id,long long destino_id,const std::vector<int>& pai) {
    auto caminhoIndice = reconstruirCaminho(origem_id, destino_id, pai);

    std::vector<long long> caminhoIDs;

    for (int idx : caminhoIndice)
        caminhoIDs.push_back(obterIdOriginal(idx));

    return caminhoIDs;
}



};

#endif