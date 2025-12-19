#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define MAX 300
#define ARQUIVO "qa194.tsp"


typedef struct {
    int id;
    double x, y;
} Cidade;

typedef struct {
    int n;
    double **mat;
} GrafoMatriz;

typedef struct No {
    int v;
    double peso;
    struct No *prox;
} No;

typedef struct {
    int n;
    No **lista;
} GrafoLista;


double distanciaEuclidiana(Cidade a, Cidade b) {
    return sqrt(
        (a.x - b.x) * (a.x - b.x) +
        (a.y - b.y) * (a.y - b.y)
    );
}


int lerArquivoTSP(Cidade cidades[]) {
    FILE *f = fopen(ARQUIVO, "r");
    if (!f) {
        printf("Erro ao abrir o arquivo %s\n", ARQUIVO);
        exit(1);
    }

    char linha[256];

    /* Procurar NODE_COORD_SECTION */
    while (fgets(linha, sizeof(linha), f)) {
        if (strncmp(linha, "NODE_COORD_SECTION", 18) == 0)
            break;
    }

    int n = 0;

    /* Ler coordenadas */
    while (fgets(linha, sizeof(linha), f)) {
        if (strncmp(linha, "EOF", 3) == 0)
            break;

        int id;
        double x, y;

        if (sscanf(linha, "%d %lf %lf", &id, &x, &y) == 3) {
            cidades[n].id = id;
            cidades[n].x = x;
            cidades[n].y = y;
            n++;
        }
    }

    fclose(f);

    if (n == 0) {
        printf("Erro: nenhuma cidade lida!\n");
        exit(1);
    }

    return n;
}

GrafoMatriz *criarMatriz(Cidade c[], int n) {
    GrafoMatriz *g = malloc(sizeof(GrafoMatriz));
    g->n = n;

    g->mat = malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++) {
        g->mat[i] = malloc(n * sizeof(double));
        for (int j = 0; j < n; j++) {
            g->mat[i][j] = (i == j) ? 0.0 : distanciaEuclidiana(c[i], c[j]);
        }
    }
    return g;
}

void removerVerticeMatriz(GrafoMatriz *g, int v) {
    v--; /* índice começa em 0 */

    for (int i = v; i < g->n - 1; i++)
        for (int j = 0; j < g->n; j++)
            g->mat[i][j] = g->mat[i + 1][j];

    for (int j = v; j < g->n - 1; j++)
        for (int i = 0; i < g->n - 1; i++)
            g->mat[i][j] = g->mat[i][j + 1];

    g->n--;
}

GrafoLista *criarLista(Cidade c[], int n) {
    GrafoLista *g = malloc(sizeof(GrafoLista));
    g->n = n;
    g->lista = calloc(n, sizeof(No *));

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            double d = distanciaEuclidiana(c[i], c[j]);

            No *a = malloc(sizeof(No));
            a->v = j;
            a->peso = d;
            a->prox = g->lista[i];
            g->lista[i] = a;

            No *b = malloc(sizeof(No));
            b->v = i;
            b->peso = d;
            b->prox = g->lista[j];
            g->lista[j] = b;
        }
    }
    return g;
}

void removerVerticeLista(GrafoLista *g, int v) {
    v--;

    for (int i = 0; i < g->n; i++) {
        No **p = &g->lista[i];
        while (*p) {
            if ((*p)->v == v) {
                No *aux = *p;
                *p = aux->prox;
                free(aux);
            } else {
                if ((*p)->v > v)
                    (*p)->v--;
                p = &(*p)->prox;
            }
        }
    }

    free(g->lista[v]);
    for (int i = v; i < g->n - 1; i++)
        g->lista[i] = g->lista[i + 1];

    g->n--;
}

int existeArestaMatriz(GrafoMatriz *g, int u, int v) {
    u--; v--;
    if (u < 0 || v < 0 || u >= g->n || v >= g->n)
        return 0;
    return g->mat[u][v] > 0.0;
}

int existeArestaLista(GrafoLista *g, int u, int v) {
    u--; v--;
    if (u < 0 || v < 0 || u >= g->n || v >= g->n)
        return 0;

    No *p = g->lista[u];
    while (p) {
        if (p->v == v)
            return 1;
        p = p->prox;
    }
    return 0;
}

void inserirArestaMatriz(GrafoMatriz *g, int u, int v, double peso) {
    u--; v--;
    if (u < 0 || v < 0 || u >= g->n || v >= g->n)
        return;

    g->mat[u][v] = peso;
    g->mat[v][u] = peso;
}

void inserirArestaLista(GrafoLista *g, int u, int v, double peso) {
    u--; v--;
    if (u < 0 || v < 0 || u >= g->n || v >= g->n)
        return;

    if (existeArestaLista(g, u + 1, v + 1))
        return;

    No *a = malloc(sizeof(No));
    a->v = v;
    a->peso = peso;
    a->prox = g->lista[u];
    g->lista[u] = a;

    No *b = malloc(sizeof(No));
    b->v = u;
    b->peso = peso;
    b->prox = g->lista[v];
    g->lista[v] = b;
}

void removerArestaMatriz(GrafoMatriz *g, int u, int v) {
    u--; v--;
    if (u < 0 || v < 0 || u >= g->n || v >= g->n)
        return;

    g->mat[u][v] = 0.0;
    g->mat[v][u] = 0.0;
}

void removerArestaLista(GrafoLista *g, int u, int v) {
    u--; v--;
    if (u < 0 || v < 0 || u >= g->n || v >= g->n)
        return;

    No **p = &g->lista[u];
    while (*p) {
        if ((*p)->v == v) {
            No *aux = *p;
            *p = aux->prox;
            free(aux);
            break;
        }
        p = &(*p)->prox;
    }

    p = &g->lista[v];
    while (*p) {
        if ((*p)->v == u) {
            No *aux = *p;
            *p = aux->prox;
            free(aux);
            break;
        }
        p = &(*p)->prox;
    }
}


void relatorio() {
    printf("\n===== RELATÓRIO DE COMPLEXIDADE =====\n");

    printf("\nMATRIZ DE ADJACÊNCIA\n");
    printf("Espaço: O(n²)\n");
    printf("Busca de aresta: O(1)\n");
    printf("Inserção/remoção de vértice: O(n²)\n");

    printf("\nLISTA DE ADJACÊNCIA\n");
    printf("Espaço: O(n + m)\n");
    printf("Busca de aresta: O(n)\n");
    printf("Inserção/remoção de vértice: O(n + m)\n");

    printf("\nConclusão:\n");
    printf("Grafos densos → matriz de adjacência\n");
    printf("Grafos esparsos → lista de adjacência\n");
}

void menu() {
    printf("\n=========== MENU ===========\n");
    printf("1 - Buscar aresta (MATRIZ)\n");
    printf("2 - Buscar aresta (LISTA)\n");
    printf("3 - Mostrar distancia entre duas cidades\n");
    printf("4 - Inserir aresta\n");
    printf("5 - Remover aresta\n");
    printf("6 - Remover vertice\n");
    printf("7 - Relatorio de complexidade\n");
    printf("0 - Sair\n");
    printf("============================\n");
    printf("Escolha uma opcao: ");
}



int main() {
    Cidade cidades[MAX];

    int n = lerArquivoTSP(cidades);
    printf("Cidades lidas: %d\n", n);

    GrafoMatriz *gm = criarMatriz(cidades, n);
    GrafoLista *gl = criarLista(cidades, n);

    int opcao;
    int u, v;

    do {
        menu();
        scanf("%d", &opcao);

        switch (opcao) {

        case 1:  /* BUSCA MATRIZ */
            printf("Digite u e v: ");
            scanf("%d %d", &u, &v);

            if (existeArestaMatriz(gm, u, v))
                printf("Aresta EXISTE na matriz.\n");
            else
                printf("Aresta NAO existe na matriz.\n");
            break;

        case 2:  /* BUSCA LISTA */
            printf("Digite u e v: ");
            scanf("%d %d", &u, &v);

            if (existeArestaLista(gl, u, v))
                printf("Aresta EXISTE na lista.\n");
            else
                printf("Aresta NAO existe na lista.\n");
            break;

        case 3:  /* DISTÂNCIA ENTRE CIDADES */
            printf("Digite as cidades u e v: ");
            scanf("%d %d", &u, &v);

            if (u < 1 || v < 1 || u > n || v > n) {
                printf("Vertices invalidos.\n");
            } else {
                double d = distanciaEuclidiana(cidades[u - 1], cidades[v - 1]);
                printf("Distancia entre cidade %d e %d: %.2f\n", u, v, d);
            }
            break;

        case 4:
            printf("Digite u e v: ");
            scanf("%d %d", &u, &v);

            inserirArestaMatriz(gm, u, v,
                distanciaEuclidiana(cidades[u - 1], cidades[v - 1]));
            inserirArestaLista(gl, u, v,
                distanciaEuclidiana(cidades[u - 1], cidades[v - 1]));

            printf("Aresta inserida com sucesso.\n");
            break;

        case 5:
            printf("Digite u e v: ");
            scanf("%d %d", &u, &v);

            removerArestaMatriz(gm, u, v);
            removerArestaLista(gl, u, v);

            printf("Aresta removida com sucesso.\n");
            break;

        case 6:
            printf("Digite o vertice v: ");
            scanf("%d", &v);

            removerVerticeMatriz(gm, v);
            removerVerticeLista(gl, v);

            printf("Vertice removido com sucesso.\n");
            break;

        case 7:  
            relatorio();
            break;

        case 0:
            printf("Encerrando o programa...\n");
            break;

        default:
            printf("Opcao invalida!\n");
        }

    } while (opcao != 0);

    return 0;
}
