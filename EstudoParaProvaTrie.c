#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define N_ALFABETO 26

typedef bool TIPORET;

typedef struct no {
    struct no *filhos[N_ALFABETO];
    TIPORET fim;
} NO;

typedef NO *PONT;

PONT criaNo() {
    PONT p = (PONT)malloc(sizeof(NO));
    if (p) {
        p->fim = false;
        for (int i = 0; i < N_ALFABETO; i++) {
            p->filhos[i] = NULL;
        }
    }
    return p;
}

PONT inicializa() {
    return criaNo();
}

int mapearIndice(char c) {
    return ((int)c - (int)'a');
}

void insere(PONT raiz, char *chave) {
    int nivel;
    int compr = strlen(chave);
    PONT p = raiz;

    for (nivel = 0; nivel < compr; nivel++) {
        int i = mapearIndice(chave[nivel]);
        if (i < 0 || i >= N_ALFABETO) continue; // ignora caracteres inválidos

        if (!p->filhos[i])
            p->filhos[i] = criaNo();

        p = p->filhos[i];
    }
    p->fim = true;
}

bool busca(PONT raiz, char *chave) {
    int compr = strlen(chave);
    PONT p = raiz;

    for (int nivel = 0; nivel < compr; nivel++) {
        int i = mapearIndice(chave[nivel]);
        if (i < 0 || i >= N_ALFABETO) return false;

        if (!p->filhos[i])
            return false;

        p = p->filhos[i];
    }
    return p != NULL && p->fim;
}

int main() {
    PONT r = inicializa();

    char palavra[256];
    printf("Digite uma palavra para inserir: ");
    scanf("%255s", palavra);

    insere(r, palavra);

    char palavra1[256];
    printf("Digite uma palavra para buscar: ");
    scanf("%255s", palavra1);

    if (busca(r, palavra1))
        printf("Palavra '%s' encontrada!\n", palavra1);
    else
        printf("Palavra '%s' NAO encontrada.\n", palavra1);

    return 0;
}
