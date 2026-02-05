#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>


//---------Estruturas e funções da Lista de adjacencia----------//

typedef struct no NO;

struct no {
    int rotulo;
    NO *prox;
};

typedef struct grafo *Grafo;
struct grafo {
    NO **A;
    long n;
    long m;
};

Grafo inicializarGrafo(long n);
void inserirGrafo(Grafo G, int V, int W);
void mostrarGrafo(Grafo G);
void imprimirGrafo(Grafo G);
void imprimirArquivoGrafo(Grafo G,FILE *arquivo);
Grafo lerGrafoArquivo(FILE *f);
Grafo liberarGrafo(Grafo G);

//---------Estruturas e funções da Trie----------//
#define N_ALFABETO 26

typedef bool TIPORET;

typedef struct n {
    struct n *filhos[N_ALFABETO];
    TIPORET fim;
} N;

typedef N *PONT;

PONT criaNo();
PONT inicializa();
int mapearIndice(char c);
void insere(PONT raiz, char *chave);
bool busca(PONT raiz, char *chave);
void liberarTrie(PONT raiz);


int main() {


}

/*-------------Inicialização das funções da Lista de adjacencia------------*/

Grafo inicializarGrafo(long n) {
    Grafo G = malloc (sizeof *G);
    G->n = n;
    G->m = 0;
    G->A = malloc (n * sizeof(NO*));
    for (long i = 0; i < n; i++) {
        G->A[i] = NULL;
    }
    return G;
}

void inserirGrafo(Grafo G, int V, int W) {
    NO *p;
    for (p = G->A[V]; p != NULL; p = p->prox) {
        if (p->rotulo == W) {
            return;
        }
    }

    p = malloc (sizeof(NO));
    p->rotulo = W;
    p->prox  = G->A[V];
    G->A[V] = p;
    G->m++;
}

//void mostrarGrafo(Grafo G);
//void imprimirGrafo(Grafo G);
void imprimirArquivoGrafo(Grafo G,FILE *arquivo) {
    NO *p;
    fprintf(arquivo, "%ld %ld\n", G->n, G->m);
    for (long i = 0; i < G->n; i++) {
        for (p = G->A[i]; p!= NULL; p = p->prox) {
            fprintf(arquivo, "%2d ", p->rotulo);
        }
        fprintf(arquivo, "-1");
        fprintf(arquivo, "\n");
    }
}

Grafo lerGrafoArquivo(FILE *f) {
    long  n, m;
    fscanf(f, "%ld %ld", &n, &m);

    Grafo G = inicializarGrafo(n);

    int V, W;

    for ( long i = 0; i < m; i++) {
        fscanf(f, "%d %d", &V, &W);
        inserirGrafo(G, V, W);
    }
    return G;
}

Grafo liberarGrafo(Grafo G) {
    NO *p;
    for (long i = 0; i < G->n; i++) {
        p = G->A[i];
        while (p != NULL) {
            G->A[i] = p;
            p = p->prox;
            free(G->A[i]);
        }
        G->A[i] = NULL;
    }
    free(G->A);
    G->A = NULL;
    free(G);
    return NULL;
}

/*-----------Inicialização de funções da trie------------*/

PONT criaNo() {
    PONT p = (PONT)malloc(sizeof(N));
    if (!p) {
    printf("Erro de memoria\n");
    exit(1);
    }

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
    c = tolower(c);
    if (c < 'a' || c > 'z') return -1;
    return c - 'a';
}


void insere(PONT raiz, char *chave) {

    if (!raiz || !chave) {
        return;
    }

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

void liberarTrie(PONT raiz) {

    if (!raiz) return;

    for (int i = 0; i < N_ALFABETO; i++)
        liberarTrie(raiz->filhos[i]);

    free(raiz);
}

/*---------------algoritmo de dijkstra-----------------*/

void Dijkstra (Grafo G, int origem, int *dist, int pred) {
    int i, *R;
    int V, W, custo, tam_R, min_dist;
    NO *p;

    // inicializando distancias de predecessores
    for (i = 0; i < G->n; i++) {
        dist[i] = _INT_MAX__;
        pred[i] = -1;
    }

    dist[origem] = 0;
    // inicializando conjunto de vertices resolvidos

    R = malloc(G->n * sizeof(int));
    for (i = 0; i < G->n; i++) {
        R[i] = 0;
    }
    tam_R = 0;
    // enquanto não encontrar o caminho minimo para todo vertice
    while 
}
