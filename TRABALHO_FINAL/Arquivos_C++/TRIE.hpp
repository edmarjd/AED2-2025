#ifndef TRIE_HPP
#define TRIE_HPP

#include <iostream>
#include <string>
#include <vector>

class NoTrie
{
public:
    NoTrie *filhos[256]; // Suporta ASCII estendido (acentos e símbolos)
    std::vector<long long> ids_nodo;   // ID do OSMNX (ex: 240003004)
    bool ehFim;

    NoTrie()
    {
        ehFim = false;
        for (int i = 0; i < 256; i++)
        {
            filhos[i] = nullptr;
        }
    }

    // Destrutor recursivo para liberar memória automaticamente
    ~NoTrie()
    {
        for (int i = 0; i < 256; i++)
        {
            if (filhos[i])
                delete filhos[i];
        }
    }
};

class Trie
{
private:
    NoTrie *raiz;

public:
    Trie()
    {
        raiz = new NoTrie();
    }

    ~Trie()
    {
        delete raiz; // Aciona o destrutor recursivo do NoTrie
    }

    // Insere uma rua e vincula ao seu ID do mapa
    void inserir(std::string rua, long long id)
    {
        NoTrie *atual = raiz;

        for (char c : rua)
        {
            // Usamos unsigned char para tratar caracteres acentuados corretamente
            unsigned char indice = (unsigned char)c;

            if (atual->filhos[indice] == nullptr)
            {
                atual->filhos[indice] = new NoTrie();
            }
            atual = atual->filhos[indice];
        }

        atual->ehFim = true;
        atual->ids_nodo.push_back(id);
    }

    // Busca o ID de uma rua. Retorna -1 se não encontrar.
    std::vector<long long> buscarID(std::string rua)
    {
        NoTrie *atual = raiz;

        for (char c : rua)
        {
            unsigned char indice = (unsigned char)c;

            if (atual->filhos[indice] == nullptr)
                return {};

            atual = atual->filhos[indice];
        }

        return (atual->ehFim) ? atual->ids_nodo : std::vector<long long>{};
    }

    // Verifica se existe algum nome de rua que começa com o prefixo
    bool comecaCom(std::string prefixo)
    {
        NoTrie *atual = raiz;

        for (char c : prefixo)
        {
            unsigned char indice = (unsigned char)c;

            if (atual->filhos[indice] == nullptr)
            {
                return false;
            }
            atual = atual->filhos[indice];
        }
        return true;
    }

    void coletarSugestoes(NoTrie *no, std::string atual, std::vector<std::string> &resultado)
    {
        if (resultado.size() >= 15) return;

        if (no->ehFim)
            resultado.push_back(atual);

        for (int i = 0; i < 256; i++)
        {
            if (no->filhos[i])
            {
                coletarSugestoes(no->filhos[i], atual + (char)i, resultado);
                if (resultado.size() >= 15) return;
            }
        }
    }

    std::vector<std::string> sugerir(std::string prefixo)
    {
        NoTrie *atual = raiz;

        for (char c : prefixo)
        {
            unsigned char indice = (unsigned char)c;

            if (!atual->filhos[indice])
                return {};

            atual = atual->filhos[indice];
        }

        std::vector<std::string> resultado;
        coletarSugestoes(atual, prefixo, resultado);

        return resultado;
    }
};

#endif