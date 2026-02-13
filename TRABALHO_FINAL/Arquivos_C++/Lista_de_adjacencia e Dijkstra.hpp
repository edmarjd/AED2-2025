#ifndef GRAFO_HPP
#define GRAFO_HPP

#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <climits>
#include <fstream>
#include <unordered_map>
#include "json.hpp"
#include <algorithm>

using json = nlohmann::json;

//=================================
//STRUCTS
//=================================

struct noGrafo
{
    double lat;
    double lon;
};

struct arestaGrafo
{
    int destino;
    double peso;
};

struct resultadoDijkstra
{
    std::vector<double> dist;  //
    std::vector<int> pai;
};

//par para o priority_queue do dijkstra
//( distancia acc, indice do vertice )
typedef std::pair<double, int> parDI;

class Grafo
{
private:

    //=================================
    // VAR. DE DADOS DO GRAFO
    //=================================

    //lista de adjacencia e nós
    std::vector<std::vector<arestaGrafo>> adjList;
    std::vector<noGrafo> nodeList;
    
    // mapeamento: id OSM (long long) <-> Índice do Vetor (int)
    std::vector<long long> indice_para_id;
    std::unordered_map<long long, int> id_para_indice;

    //variaveis de controle
    int proximo_indice;
    int numArestas;

    //=================================
    // FUNÇÕES AUXILIARES
    //=================================

    // função que extrai o id do json
    static long long extrairID(const json &valor)
    {
        if (valor.is_string())
            return std::stoll(valor.get<std::string>());
        return valor.get<long long>();
    }

public:
    Grafo(){
        proximo_indice = 0;
        numArestas = 0;
    }



    //=================================
    // INSERÇÃO E BUSCA NO GRAFO
    //=================================
    
    //add uma aresta ao grafo
    void adicionarAresta(long long origem_id, long long destino_id, double peso)
    {
        int origem = obterIndice(origem_id);
        int destino = obterIndice(destino_id);

        if (origem != -1 && destino != -1)
        {
            adjList[origem].push_back({destino, peso});
            numArestas++;
        }
    }
    
    //add um nó ao grafo
    void adicionarNo(long long id)
    {
        obterIndice(id);
    }
    

    
    //=================================
    // CONVERSAO DE ID OSM PARA INDICE DAS LISTAS
    //=================================    
    
    // retorna o indice (int) que relaciona idOSM(long long) com idx dos vetores (lista de adj e de nós)
    // caso o int nao exista, cria um novo e redimensiona o grafo
    int obterIndice(long long id_osm)
    {   
        //se esse id ainda nao foi mapeado...   
        if (id_para_indice.find(id_osm) == id_para_indice.end())
        {
            id_para_indice[id_osm] = proximo_indice;
            indice_para_id.push_back(id_osm);

            //aumenta o vetor da lista de adj se necessario
            if(adjList.size() <= proximo_indice){
                adjList.resize(proximo_indice + 1);
                nodeList.resize(proximo_indice + 1);
            }
            proximo_indice++;
        }
        return id_para_indice[id_osm];
    }

    //faz o caminho inverso: int -> long long
    long long obterIdOSM(int indice) const
    {
        if (indice >= 0 && indice < indice_para_id.size()){
            return indice_para_id[indice];
        }
        return -1;
    }

    int buscarIndice(long long id_osm) const {
    auto it = id_para_indice.find(id_osm);
    if (it == id_para_indice.end())
        return -1;
    return it->second;
    }




    //=================================
    // FUNÇOES DO ALGORITMO DE DIJKSTRA
    //================================= 

    // Dijkstra que retorna as distâncias a partir de um ID original do JSON
    resultadoDijkstra executarDijkstra(long long id_origem)
    {
        int origem = obterIndice(id_origem);

        resultadoDijkstra res;
        res.dist.assign(proximo_indice, LLONG_MAX);
        res.pai.assign(proximo_indice, -1);

        if (origem == -1)
            return res;

        std::priority_queue<parDI, std::vector<parDI>, std::greater<parDI>> pq;

        res.dist[origem] = 0;
        pq.push({0, origem});

        while (!pq.empty())
        {
            auto [distancia, u] = pq.top();
            pq.pop();

            if (distancia > res.dist[u])
                continue;

            for (const auto &aresta : adjList[u])
            {
                if (res.dist[aresta.destino] > res.dist[u] + aresta.peso)
                {

                    res.dist[aresta.destino] = res.dist[u] + aresta.peso;
                    res.pai[aresta.destino] = u;

                    pq.push({res.dist[aresta.destino], aresta.destino});
                }
            }
        }

        return res;
    }


    std::vector<int> reconstruirCaminho(long long origem_id, long long destino_id, const std::vector<int> &pai)
    {

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


    std::vector<long long> reconstruirCaminhoIDs(long long origem_id, long long destino_id, const std::vector<int> &pai)
    {
        auto caminhoIndice = reconstruirCaminho(origem_id, destino_id, pai);

        std::vector<long long> caminhoIDs;

        for (int idx : caminhoIndice)
            caminhoIDs.push_back(obterIdOSM(idx));

        return caminhoIDs;
    }



    //=================================
    // FUNÇOES P/ LER JSON
    //=================================    

    //le o arquivo edges.json e carrega no grafo
    void carregarArestas(const std::string &caminho_edges)
    {
        std::ifstream arquivo(caminho_edges);
        if (!arquivo.is_open())
        {
            std::cerr << "Erro ao abrir " << caminho_edges << "\n" ;
            return;
        }

        json dados_json;
        arquivo >> dados_json;


        for (const auto &item : dados_json)
        {

            if (!item.contains("u") || !item.contains("v") || !item.contains("data")){
                continue;
            }

            const auto &data = item.at("data");

            if (!data.contains("length")){
                continue;
            }

            //extraçao de ids
            long long u = extrairID(item.at("u"));
            long long v = extrairID(item.at("v"));
            double peso = data.at("length").get<double>();

            bool oneway = false;
            if (data.contains("oneway"))
                oneway = data.at("oneway").get<bool>();

            this->adicionarAresta(u, v, peso);

            if (!oneway)
                this->adicionarAresta(v, u, peso);
        }
    }

    //le o arquivo nodes.json e carrega no grafo
    void carregarNos(const std::string &caminho_nodes){
        std::ifstream arquivo(caminho_nodes);
        if (!arquivo.is_open()) {
            std::cerr << "Erro ao abrir " << caminho_nodes << "\n";
            return;
        }

        json dados_json;
        arquivo >> dados_json;

        for (const auto &item : dados_json)
        {
            if (!item.contains("id") || !item.contains("y") || !item.contains("x")){
                continue;
            }

            long long id = item.at("id").get<long long>();
            double lat = item.at("y").get<double>();
            double lon = item.at("x").get<double>();
            
            int indice = obterIndice(id);
        
            nodeList[indice] = {lat, lon};
        }
    } 



    //=================================
    // GETTERS
    //=================================  

    //retprna a coordenada do nó com idOSM passado
    noGrafo obterCoordenada(long long id_osm) {
        int idx = obterIndice(id_osm);
        if (idx != -1 && idx < nodeList.size()) {
            return nodeList[idx];
        }
        return {0.0, 0.0}; // Retorno de segurança
    }

    //retorna o total de arestas do grafo
    int totalArestas() const
    {
        return numArestas;
    }

};

#endif