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
    
    std::cout << "Carregando base de dados de Pelotas..." << std::endl;
    
    // 1. Carrega os nomes das ruas na Trie
    carregarDados(trieRuas, "label_to_nodes.json");

    // 2. Carrega o Grafo (ajuste o total de nodos conforme sua base)
    // Dica: use um valor ligeiramente maior que o número de entradas em nodes.json
    int total_estimado_nodos = 50000; 
    Grafo* cidade = Grafo::carregarDeJSON("edges.json", total_estimado_nodos);
    std::cout << "Total vertices: " << cidade->totalArestas() << std::endl;



    for (auto& par : id_para_nome) {
        cidade->adicionarNo(par.first);
    }


    if (!cidade) {
        std::cerr << "Erro ao carregar malha viaria!" << std::endl;
        return 1;
    }

    std::string nomeOrigem, nomeDestino;
    
    // 3. Interface de busca
    std::cout << "\n--- Navegador GPS Pelotas ---\n";
    std::cout << "Origem (Intersecao): ";
    std::getline(std::cin, nomeOrigem);
    
    std::cout << "Destino (Intersecao): ";
    std::getline(std::cin, nomeDestino);

    long long idOrigem = trieRuas.buscarID(nomeOrigem);
    long long idDestino = trieRuas.buscarID(nomeDestino);

    if (idOrigem != -1 && idDestino != -1) {
        // 4. Processamento da Rota
        ResultadoDijkstra resultado = cidade->executarDijkstra(idOrigem);
        
        // No Dijkstra, precisamos saber o índice do destino para ver o resultado
        int idxDestino = cidade->obterIndice(idDestino);

        if (idxDestino == -1) {
            std::cout << "Destino nao existe no grafo\n";
            return 0;
        }


        if (resultado.dist[idxDestino] == LLONG_MAX) {
        std::cout << "\nNao existe caminho entre esses locais." << std::endl;
        } else {
        std::cout << "\nRota encontrada!" << std::endl;
        std::cout << "Distancia total: " << resultado.dist[idxDestino] << " metros." << std::endl;
        }
    } else {
        std::cout << "\nErro: Uma ou ambas as ruas nao foram encontradas na base." << std::endl;
    }

    delete cidade;
    return 0;
}