#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include "json.hpp"
#include "TRIE.hpp"
#include "Lista_de_adjacencia e Dijkstra.hpp"

using json = nlohmann::json;

// Mapeamento global para traduzir ID -> Nome da Rua no resultado final
std::unordered_map<long long, std::string> id_para_nome;


long long extrairID(nlohmann::json v) {
    if (v.is_array() && !v.empty()) return v[0].get<long long>();
    if (v.is_number()) return v.get<long long>();
    if (v.is_string()) return std::stoll(v.get<std::string>());
    return -1;
}

void carregarDados(Trie& trie, const std::string& arquivo_labels) {
    std::ifstream f(arquivo_labels);
    if (!f.is_open()) {
        std::cerr << "Erro ao abrir: " << arquivo_labels << std::endl;
        return;
    }

    json j;
    try {
        f >> j;
        for (auto& [label, id_valor] : j.items()) {
            // No seu JSON, id_valor é algo como [240003004]
            if (id_valor.is_array() && !id_valor.empty()) {
                // Pegamos o primeiro elemento e convertemos para long long
                long long id = id_valor[0].get<long long>(); 
                trie.inserir(label, id);
                id_para_nome[id] = label; 
            } 
            // Caso o ID venha como número direto em algum outro arquivo
            else if (id_valor.is_number()) {
                long long id = id_valor.get<long long>();
                trie.inserir(label, id);
                id_para_nome[id] = label;
            }
        }
    } catch (json::parse_error& e) {
        std::cerr << "Erro no formato do JSON: " << e.what() << std::endl;
    }
    f.close();
}

int main() {
    Trie trieRuas;
    Grafo cidade;

    std::cout << "Carregando base de dados de Pelotas..." << std::endl;

    // 1. Trie
    carregarDados(trieRuas, "label_to_nodes.json");

    // 2. Grafo
    cidade.carregarNos("nodes.json");
    cidade.carregarArestas("edges.json");

    std::cout << "Grafo carregado com sucesso." << std::endl;

    std::string nomeOrigem, nomeDestino;

    // 3. Interface
    std::cout << "\n--- Navegador GPS Pelotas ---\n";
    std::cout << "Origem (nome da rua): ";
    std::getline(std::cin, nomeOrigem);

    std::cout << "Destino (nome da rua): ";
    std::getline(std::cin, nomeDestino);

    auto idsOrigem = trieRuas.buscarID(nomeOrigem);
    auto idsDestino = trieRuas.buscarID(nomeDestino);

    if (idsOrigem.empty() || idsDestino.empty()) {
        std::cout << "\nErro: Uma ou ambas as ruas nao foram encontradas na base." << std::endl;
        return 0;
    }

    long long idOrigem = idsOrigem[0];
    long long idDestino = idsDestino[0];

    // 4. Dijkstra
    resultadoDijkstra resultado = cidade.executarDijkstra(idOrigem);

    int idxDestino = cidade.buscarIndice(idDestino); // função que NÃO cria nó

    if (idxDestino == -1) {
        std::cout << "Destino nao existe no grafo\n";
        return 0;
    }

    if (resultado.dist[idxDestino] == LLONG_MAX) {
        std::cout << "\nNao existe caminho entre esses locais." << std::endl;
    } else {
        std::cout << "\nRota encontrada!" << std::endl;
        std::cout << "Distancia total: " << resultado.dist[idxDestino] << " metros." << std::endl;

        auto caminho = cidade.reconstruirCaminhoIDs(idOrigem, idDestino, resultado.pai);

        std::cout << "\nCaminho:\n";
        for (auto id : caminho) {
            if (id_para_nome.count(id))
                std::cout << id_para_nome[id] << " -> ";
            else
                std::cout << id << " -> ";
        }
        std::cout << "FIM\n";
    }

    return 0;
}
