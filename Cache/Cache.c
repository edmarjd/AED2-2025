#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ================= ESTRUTURAS ================= */

typedef struct {
    int validade;
    unsigned int tag;
} Linha;

typedef struct {
    Linha **conjuntos;
    int nsets;
    int assoc;
    int bsize;

    int acessos;
    int hits;
    int miss_total;
    int miss_compulsorio;
    int miss_capacidade;
    int miss_conflito;
} Cache;

/* Cache totalmente associativa auxiliar */
typedef struct {
    int validade;
    unsigned int bloco;
} LinhaTA;

/* ================= HISTÓRICO (MISS COMPULSÓRIO) ================= */

unsigned int *historico;
int hist_count = 0;
int hist_max = 0;

int blocoJaAcessado(unsigned int bloco) {
    for (int i = 0; i < hist_count; i++)
        if (historico[i] == bloco)
            return 1;

    historico[hist_count++] = bloco;
    return 0;
}

/* ================= CACHE TOTALMENTE ASSOCIATIVA ================= */

int cacheTAtemBloco(LinhaTA *ta, int linhas, unsigned int bloco) {
    for (int i = 0; i < linhas; i++)
        if (ta[i].validade && ta[i].bloco == bloco)
            return 1;
    return 0;
}

void inserirCacheTA(LinhaTA *ta, int linhas, unsigned int bloco) {
    int r = rand() % linhas;
    ta[r].validade = 1;
    ta[r].bloco = bloco;
}

/* ================= CACHE PRINCIPAL ================= */

Cache criarCache(int nsets, int assoc, int bsize) {
    Cache c;
    c.nsets = nsets;
    c.assoc = assoc;
    c.bsize = bsize;

    c.acessos = c.hits = c.miss_total = 0;
    c.miss_compulsorio = c.miss_capacidade = c.miss_conflito = 0;

    c.conjuntos = malloc(nsets * sizeof(Linha *));
    for (int i = 0; i < nsets; i++) {
        c.conjuntos[i] = malloc(assoc * sizeof(Linha));
        for (int j = 0; j < assoc; j++)
            c.conjuntos[i][j].validade = 0;
    }
    return c;
}

void acessarCache(Cache *c, unsigned int endereco,
                  LinhaTA *cacheTA, int linhasTA) {

    unsigned int bloco = endereco / c->bsize;
    int indice = bloco % c->nsets;
    unsigned int tag = bloco / c->nsets;

    c->acessos++;

    /* HIT */
    for (int i = 0; i < c->assoc; i++) {
        if (c->conjuntos[indice][i].validade &&
            c->conjuntos[indice][i].tag == tag) {
            c->hits++;
            return;
        }
    }

    /* MISS */
    c->miss_total++;

    if (!blocoJaAcessado(bloco)) {
        c->miss_compulsorio++;
    } else {
        if (cacheTAtemBloco(cacheTA, linhasTA, bloco))
            c->miss_conflito++;
        else
            c->miss_capacidade++;
    }

    /* Atualiza cache totalmente associativa */
    inserirCacheTA(cacheTA, linhasTA, bloco);

    /* Substituição RANDOM */
    int r = rand() % c->assoc;
    if(c->conjuntos[indice][r].validade = 0)
    

    c->conjuntos[indice][r].validade = 1;
    c->conjuntos[indice][r].tag = tag;
}

void imprimirResultados(Cache *c, const char *nome) {
    printf("\n===== Cache de %s =====\n", nome);
    printf("Acessos: %d\n", c->acessos);
    printf("Hits: %d\n", c->hits);
    printf("Miss Total: %d\n", c->miss_total);
    printf("Miss Compulsorio: %d\n", c->miss_compulsorio);
    printf("Miss Conflito: %d\n", c->miss_conflito);
    printf("Miss Capacidade: %d\n", c->miss_capacidade);
    printf("Hit Rate: %.2f%%\n",
           (float)c->hits / c->acessos * 100.0);
    printf("Miss Rate: %.2f%%\n",
           (float)c->miss_total / c->acessos * 100.0);
}

/* ================= MAIN ================= */

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Uso:\n");
        printf("./Cache.exe nsetsI:bsizeI:assocI nsetsD:bsizeD:assocD arquivo\n");
        return 1;
    }

    int nsetsI, bsizeI, assocI;
    int nsetsD, bsizeD, assocD;

    sscanf(argv[1], "%d:%d:%d", &nsetsI, &bsizeI, &assocI);
    sscanf(argv[2], "%d:%d:%d", &nsetsD, &bsizeD, &assocD);

    FILE *f = fopen(argv[3], "r");
    if (!f) {
        printf("Erro ao abrir arquivo\n");
        return 1;
    }

    srand(time(NULL));

    Cache cacheI = criarCache(nsetsI, assocI, bsizeI);
    Cache cacheD = criarCache(nsetsD, assocD, bsizeD);

    int linhasTA_I = nsetsI * assocI;
    int linhasTA_D = nsetsD * assocD;

    LinhaTA *cacheTA_I = malloc(linhasTA_I * sizeof(LinhaTA));
    LinhaTA *cacheTA_D = malloc(linhasTA_D * sizeof(LinhaTA));

    for (int i = 0; i < linhasTA_I; i++) cacheTA_I[i].validade = 0;
    for (int i = 0; i < linhasTA_D; i++) cacheTA_D[i].validade = 0;

    hist_max = 200000;
    historico = malloc(hist_max * sizeof(unsigned int));

    unsigned int endereco;
    int tipo;

    while (fscanf(f, "%u %d", &endereco, &tipo) == 2) {
        if (tipo == 0)
            acessarCache(&cacheI, endereco, cacheTA_I, linhasTA_I);
        else
            acessarCache(&cacheD, endereco, cacheTA_D, linhasTA_D);
    }

    fclose(f);

    imprimirResultados(&cacheI, "Instrucoes");
    imprimirResultados(&cacheD, "Dados");

    return 0;
}
