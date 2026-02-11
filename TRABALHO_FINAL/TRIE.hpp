#include <iostream>
#include <string>
#include <vector>

class NoTrie {
public:
    NoTrie* filhos[256]; // Suporta ASCII estendido (acentos e símbolos)
    long long id_nodo;    // ID do OSMNX (ex: 240003004)
    bool ehFim;

    NoTrie() {
        ehFim = false;
        id_nodo = -1; // -1 indica que não há ID vinculado a este nó
        for (int i = 0; i < 256; i++) {
            filhos[i] = nullptr;
        }
    }

    // Destrutor recursivo para liberar memória automaticamente
    ~NoTrie() {
        for (int i = 0; i < 256; i++) {
            if (filhos[i]) delete filhos[i];
        }
    }
};

class Trie {
private:
    NoTrie* raiz;

public:
    Trie() {
        raiz = new NoTrie();
    }

    ~Trie() {
        delete raiz; // Aciona o destrutor recursivo do NoTrie
    }

    // Insere uma rua e vincula ao seu ID do mapa
    void inserir(std::string rua, long long id) {
        NoTrie* atual = raiz;

        for (char c : rua) {
            // Usamos unsigned char para tratar caracteres acentuados corretamente
            unsigned char indice = (unsigned char)c;

            if (atual->filhos[indice] == nullptr) {
                atual->filhos[indice] = new NoTrie();
            }
            atual = atual->filhos[indice];
        }

        atual->ehFim = true;
        atual->id_nodo = id; // Guarda o ID no final da palavra
    }

    // Busca o ID de uma rua. Retorna -1 se não encontrar.
    long long buscarID(std::string rua) {
        NoTrie* atual = raiz;

        for (char c : rua) {
            unsigned char indice = (unsigned char)c;

            if (atual->filhos[indice] == nullptr)
                return -1;

            atual = atual->filhos[indice];
        }

        return (atual->ehFim) ? atual->id_nodo : -1;
    }

    // Verifica se existe algum nome de rua que começa com o prefixo
    bool comecaCom(std::string prefixo) {
        NoTrie* atual = raiz;

        for (char c : prefixo) {
            unsigned char indice = (unsigned char)c;

            if (atual->filhos[indice] == nullptr) {
                return false;
            }
            atual = atual->filhos[indice];
        }
        return true;
    }
};
