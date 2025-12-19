#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct No {
    unsigned char c;
    int freq;
    struct No *esq, *dir;
} No;


No *novoNo(unsigned char c, int freq, No *e, No *d) {
    No *n = (No *)malloc(sizeof(No));
    n->c = c;
    n->freq = freq;
    n->esq = e;
    n->dir = d;
    return n;
}


void contarFrequencias(const char *arquivo, int freq[256]) {
    FILE *f = fopen(arquivo, "rb");
    if (!f) {
        printf("Erro ao abrir %s\n", arquivo);
        exit(1);
    }

    unsigned char c;
    while (fread(&c, 1, 1, f) == 1)
        freq[c]++;

    fclose(f);
}

No *construirHuffman(int freq[256]) {
    No *lista[256];
    int n = 0;

    for (int i = 0; i < 256; i++)
        if (freq[i] > 0)
            lista[n++] = novoNo((unsigned char)i, freq[i], NULL, NULL);

    while (n > 1) {
        int m1 = 0, m2 = 1;
        if (lista[m2]->freq < lista[m1]->freq) {
            int t = m1; m1 = m2; m2 = t;
        }

        for (int i = 2; i < n; i++) {
            if (lista[i]->freq < lista[m1]->freq) {
                m2 = m1;
                m1 = i;
            } else if (lista[i]->freq < lista[m2]->freq) {
                m2 = i;
            }
        }

        No *novo = novoNo(
            0,
            lista[m1]->freq + lista[m2]->freq,
            lista[m1],
            lista[m2]
        );

        lista[m1] = novo;
        lista[m2] = lista[n - 1];
        n--;
    }

    return lista[0];
}


void gerarCodigos(No *raiz, char *codigo, int tam, char *tabela[256]) {
    if (!raiz) return;

    if (!raiz->esq && !raiz->dir) {
        codigo[tam] = '\0';
        tabela[raiz->c] = strdup(codigo);
        return;
    }

    codigo[tam] = '0';
    gerarCodigos(raiz->esq, codigo, tam + 1, tabela);

    codigo[tam] = '1';
    gerarCodigos(raiz->dir, codigo, tam + 1, tabela);
}

void compactar(const char *in, const char *out, char *tabela[256]) {
    FILE *fi = fopen(in, "rb");
    FILE *fo = fopen(out, "w");

    if (!fi || !fo) {
        printf("Erro ao abrir arquivos de compactacao\n");
        exit(1);
    }

    unsigned char c;
    while (fread(&c, 1, 1, fi) == 1)
        fputs(tabela[c], fo);

    fclose(fi);
    fclose(fo);
}

void descompactar(const char *in, const char *out, No *raiz) {
    FILE *fi = fopen(in, "r");
    FILE *fo = fopen(out, "wb");

    if (!fi || !fo) {
        printf("Erro ao abrir arquivos de descompactacao\n");
        exit(1);
    }

    No *p = raiz;
    char bit;

    while ((bit = fgetc(fi)) != EOF) {
        if (bit == '0')
            p = p->esq;
        else
            p = p->dir;

        if (!p->esq && !p->dir) {
            fwrite(&p->c, 1, 1, fo);
            p = raiz;
        }
    }

    fclose(fi);
    fclose(fo);
}

void liberarArvore(No *r) {
    if (!r) return;
    liberarArvore(r->esq);
    liberarArvore(r->dir);
    free(r);
}

int main() {
    int freq[256] = {0};
    char *tabela[256] = {0};
    char codigo[256];

    contarFrequencias("palavras.txt", freq);

    No *raiz = construirHuffman(freq);
    gerarCodigos(raiz, codigo, 0, tabela);

    compactar("palavras.txt", "palavras.huff", tabela);
    descompactar("palavras.huff", "palavras_out.txt", raiz);

    printf("Compactacao e descompactacao concluídas com sucesso.\n");

    liberarArvore(raiz);
    return 0;
}
