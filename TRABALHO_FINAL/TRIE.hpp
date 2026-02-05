#include <iostream>
#include <string>
#include <vector>
#include <cctype>

class NoTrie {
public:
    NoTrie* filhos[26];
    bool fim;

    // construtor da trie
    NoTrie() {
        fim = false;
        for (int i = 0; i < 26; i++) {
            filhos[i] = nullptr;
        }
    }

    // Destrutor recursivo para liberar memória dos filhos
    ~NoTrie() {
        for (int i = 0; i < 26; i++) {
            delete filhos[i];
        }
    }
};

class Trie {
private:
    NoTrie* raiz;

    // Converte caractere para índice (0-25)
    int obterIndice(char c) {
        return tolower(c) - 'a';
    }

public:
    Trie() {
        raiz = new NoTrie();
    }

    ~Trie() {
        delete raiz; // Libera toda a estrutura recursivamente
    }

    // Insere uma palavra na trie
    void inserir(std::string palavra) {
        NoTrie* atual = raiz;

        for (char c : palavra) {
            int indice = obterIndice(c);

            if (atual->filhos[indice] == nullptr) {
                atual->filhos[indice] = new NoTrie();
            }

            atual = atual->filhos[indice];
        }

        atual->fim = true;
    }

    // Verifica se a palavra existe na trie
    bool buscar(std::string palavra) {
        NoTrie* atual = raiz;

        for (char c : palavra) {
            int indice = obterIndice(c);

            if (atual->filhos[indice] == nullptr)
                return false;

            atual = atual->filhos[indice];
        }

        return atual->fim;
    }

    // Verifica se existe alguma palavra com o prefixo informado
    bool comecaCom(std::string prefixo) {
        NoTrie* atual = raiz;

        for (char c : prefixo) {
            int indice = obterIndice(c);

            if (atual->filhos[indice] == nullptr) {
                return false;
            }

            atual = atual->filhos[indice];
        }

        return true;
    }
};
