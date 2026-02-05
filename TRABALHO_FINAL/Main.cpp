#include <iostream>
#include "Lista_de_adjacencia e Dijkstra.hpp" // Inclui o arquivo que criamos acima
#include "TRIE.hpp" // inclui o arquivo da trie

Grafo* carregarArquivo(const std::string& nome) {
    std::ifstream file(nome);
    if (!file.is_open()) return nullptr;

    int n;
    file >> n;
    Grafo* g = new Grafo(n);

    int u, v, w;
    while (file >> u >> v >> w) {
        g->adicionarAresta(u, v, w);
    }
    file.close();
    return g;
}

int main() {
    std::string nomeArquivo = "grafo.txt";
    Grafo* meuGrafo = carregarArquivo(nomeArquivo);

    if (meuGrafo == nullptr) {
        std::cerr << "Erro ao carregar " << nomeArquivo << "\n";
        return 1;
    }

    std::cout << "Estrutura do Grafo:\n";
    meuGrafo->imprimirLista();

    int inicio = 0;
    std::vector<int> resultados = meuGrafo->executarDijkstra(inicio);

    std::cout << "\nResultados Dijkstra (Partindo de " << inicio << "):\n";
    for (int i = 0; i < resultados.size(); i++) {
        std::cout << "Ate " << i << ": ";
        if (resultados[i] == INT_MAX) std::cout << "Infinito\n";
        else std::cout << resultados[i] << "\n";
    }

    delete meuGrafo; // Limpeza final
    return 0;
}