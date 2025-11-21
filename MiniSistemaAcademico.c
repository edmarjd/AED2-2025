#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* -------------------------
   Estruturas de domínio
   ------------------------- */

typedef struct {
    double matricula_aluno;
    char nome_aluno[200];
} Alunos;

typedef struct {
    char codigo_disciplina[200];
    char nome_disciplina[200];
} Disciplinas;

typedef struct {
    int id_matricula;
    Alunos matricula_aluno;
    Disciplinas codigo_disciplina;
    float media_final;
    int ano_letivo;
    int semestre_letivo;
} Matriculas;

/* -------------------------
   Árvore B para ALUNOS
   ------------------------- */

typedef struct BTreeAlunos {
    Alunos *keys;
    struct BTreeAlunos **C;
    int n;
    bool leaf;
} BTreeAlunos;

typedef struct {
    BTreeAlunos *root;
    int t;
} BTreeA;

BTreeAlunos *A_createNode(int t, bool leaf) {
    BTreeAlunos *node = malloc(sizeof(BTreeAlunos));
    node->leaf = leaf;
    node->keys = malloc(sizeof(Alunos) * (2*t - 1));
    node->C = malloc(sizeof(BTreeAlunos*) * (2*t));
    node->n = 0;
    for (int i = 0; i < 2*t; i++) {
        node->C[i] = NULL;
    }
    return node;
}

BTreeA *A_createTree(int t) {
    BTreeA *T = malloc(sizeof(BTreeA));
    T->t = t;
    T->root = A_createNode(t, true);
    return T;
}

typedef struct {
    BTreeAlunos *node;
    int pos;
} A_SearchResult;

A_SearchResult A_search(BTreeAlunos *x, double matricula) {
    int i = 0;
    while (i < x->n && matricula > x->keys[i].matricula_aluno) {
        i++;
    }

    if (i < x->n && matricula == x->keys[i].matricula_aluno) {
        return (A_SearchResult){ x, i };
    }

    if (x->leaf) {
        return (A_SearchResult){ NULL, -1 };
    }

    return A_search(x->C[i], matricula);
}

/* splitChild para Alunos */
void A_splitChild(BTreeAlunos *x, int i, int t) {
    BTreeAlunos *y = x->C[i];
    BTreeAlunos *z = A_createNode(t, y->leaf);
    z->n = t - 1;

    /* copia as últimas t-1 chaves de y para z */
    for (int j = 0; j < t-1; j++) {
        z->keys[j] = y->keys[j + t];
    }

    /* copia os filhos, se houver */
    if (!y->leaf) {
        for (int j = 0; j < t; j++) {
            z->C[j] = y->C[j + t];
        }
    }

    y->n = t - 1;

    /* abre espaço no vetor de filhos de x */
    for (int j = x->n; j >= i+1; j--) {
        x->C[j+1] = x->C[j];
    }
    x->C[i+1] = z;

    /* abre espaço no vetor de chaves de x */
    for (int j = x->n - 1; j >= i; j--) {
        x->keys[j+1] = x->keys[j];
    }

    /* move a chave do meio de y para x */
    x->keys[i] = y->keys[t-1];
    x->n += 1;
}

/* inserção auxiliar em nó não cheio (Alunos) */
void A_insertNonFull(BTreeAlunos *x, Alunos k, int t) {
    int i = x->n - 1;
    if (x->leaf) {
        while (i >= 0 && x->keys[i].matricula_aluno > k.matricula_aluno) {
            x->keys[i+1] = x->keys[i];
            i--;
        }
        x->keys[i+1] = k;
        x->n += 1;
    } else {
        while (i >= 0 && x->keys[i].matricula_aluno > k.matricula_aluno) i--;
        i++;
        if (x->C[i]->n == 2*t - 1) {
            A_splitChild(x, i, t);
            if (x->keys[i].matricula_aluno < k.matricula_aluno) i++;
        }
        A_insertNonFull(x->C[i], k, t);
    }
}

void A_insert(BTreeA *T, Alunos k) {
    BTreeAlunos *r = T->root;
    int t = T->t;
    if (r->n == 2*t - 1) {
        BTreeAlunos *s = A_createNode(t, false);
        T->root = s;
        s->C[0] = r;
        A_splitChild(s, 0, t);
        A_insertNonFull(s, k, t);
    } else {
        A_insertNonFull(r, k, t);
    }
}

/* travessia (in-order) - imprime */
void A_traverse(BTreeAlunos *x) {
    if (!x) return;
    int i;
    for (i = 0; i < x->n; i++) {
        if (!x->leaf) A_traverse(x->C[i]);
        printf("Mat: %.0f | Nome: %s\n", x->keys[i].matricula_aluno, x->keys[i].nome_aluno);
    }
    if (!x->leaf) A_traverse(x->C[i]);
}

/* libera nós (Alunos) */
void A_freeNode(BTreeAlunos *x, int t) {
    if (!x) return;
    if (!x->leaf) {
        for (int i = 0; i <= x->n; i++) A_freeNode(x->C[i], t);
    }
    free(x->keys);
    free(x->C);
    free(x);
}
void A_free(BTreeA *T) {
    if (!T) return;
    A_freeNode(T->root, T->t);
    free(T);
}

/* -------------------------
   Árvore B para DISCIPLINAS
   ------------------------- */

typedef struct BTreeDisciplinas {
    Disciplinas *keys;
    struct BTreeDisciplinas **C;
    int n;
    bool leaf;
} BTreeDisciplinas;

typedef struct {
    BTreeDisciplinas *root;
    int t;
} BTreeD;

BTreeDisciplinas *D_createNode(int t, bool leaf) {
    BTreeDisciplinas *node = malloc(sizeof(BTreeDisciplinas));
    node->leaf = leaf;
    node->keys = malloc(sizeof(Disciplinas) * (2*t - 1));
    node->C = malloc(sizeof(BTreeDisciplinas*) * (2*t));
    node->n = 0;
    for (int i = 0; i < 2*t; i++) node->C[i] = NULL;
    return node;
}

BTreeD *D_createTree(int t) {
    BTreeD *T = malloc(sizeof(BTreeD));
    T->t = t;
    T->root = D_createNode(t, true);
    return T;
}

typedef struct {
    BTreeDisciplinas *node;
    int pos;
} D_SearchResult;

D_SearchResult D_search(BTreeDisciplinas *x, const char *codigo) {
    int i = 0;

    while (i < x->n && strcmp(codigo, x->keys[i].codigo_disciplina) > 0) {
        i++;
    }

    if (i < x->n && strcmp(codigo, x->keys[i].codigo_disciplina) == 0) {
        return (D_SearchResult){ x, i };
    }

    if (x->leaf) {
        return (D_SearchResult){ NULL, -1 };
    }

    return D_search(x->C[i], codigo);
}

void D_splitChild(BTreeDisciplinas *x, int i, int t) {
    BTreeDisciplinas *y = x->C[i];
    BTreeDisciplinas *z = D_createNode(t, y->leaf);
    z->n = t - 1;

    for (int j = 0; j < t - 1; j++) {
        z->keys[j] = y->keys[j + t];
    }

    if (!y->leaf) {
        for (int j = 0; j < t; j++) {
            z->C[j] = y->C[j + t];
        }
    }

    y->n = t - 1;

    for (int j = x->n; j >= i + 1; j--) {
        x->C[j + 1] = x->C[j];
    }
    x->C[i + 1] = z;

    for (int j = x->n - 1; j >= i; j--) {
        x->keys[j + 1] = x->keys[j];
    }

    x->keys[i] = y->keys[t - 1];
    x->n++;
}

void D_insertNonFull(BTreeDisciplinas *x, Disciplinas k, int t) {
    int i = x->n - 1;

    if (x->leaf) {
        while (i >= 0 && strcmp(x->keys[i].codigo_disciplina, k.codigo_disciplina) > 0) {
            x->keys[i + 1] = x->keys[i];
            i--;
        }

        x->keys[i + 1] = k;
        x->n++;
    } else {
        while (i >= 0 && strcmp(x->keys[i].codigo_disciplina, k.codigo_disciplina) > 0) {
            i--;
        }
        i++;

        if (x->C[i]->n == 2 * t - 1) {
            D_splitChild(x, i, t);

            if (strcmp(k.codigo_disciplina, x->keys[i].codigo_disciplina) > 0) {
                i++;
            }
        }

        D_insertNonFull(x->C[i], k, t);
    }
}

void D_insert(BTreeD *T, Disciplinas k) {
    BTreeDisciplinas *r = T->root;
    int t = T->t;
    if (r->n == 2*t - 1) {
        BTreeDisciplinas *s = D_createNode(t, false);
        T->root = s;
        s->C[0] = r;
        D_splitChild(s, 0, t);
        D_insertNonFull(s, k, t);
    } else {
        D_insertNonFull(r, k, t);
    }
}

void D_freeNode(BTreeDisciplinas *x, int t) {
    if (!x) return;
    if (!x->leaf) for (int i = 0; i <= x->n; i++) D_freeNode(x->C[i], t);
    free(x->keys);
    free(x->C);
    free(x);
}
void D_free(BTreeD *T) {
    if (!T) return;
    D_freeNode(T->root, T->t);
    free(T);
}

/* -------------------------
   Árvore B para MATRICULAS
   ------------------------- */

typedef struct BTreeMatriculas {
    Matriculas *keys;
    struct BTreeMatriculas **C;
    int n;
    bool leaf;
} BTreeMatriculas;

typedef struct {
    BTreeMatriculas *root;
    int t;
} BTreeM;

BTreeMatriculas *M_createNode(int t, bool leaf) {
    BTreeMatriculas *node = malloc(sizeof(BTreeMatriculas));
    node->leaf = leaf;
    node->keys = malloc(sizeof(Matriculas) * (2*t - 1));
    node->C = malloc(sizeof(BTreeMatriculas*) * (2*t));
    node->n = 0;
    for (int i = 0; i < 2*t; i++) node->C[i] = NULL;
    return node;
}

BTreeM *M_createTree(int t) {
    BTreeM *T = malloc(sizeof(BTreeM));
    T->t = t;
    T->root = M_createNode(t, true);
    return T;
}

typedef struct {
    BTreeMatriculas *node;
    int pos;
} M_SearchResult;

M_SearchResult M_search(BTreeMatriculas *x, int id_matricula) {
    int i = 0;
    /* compara pelo campo id_matricula dentro de keys[i] */
    while (i < x->n && id_matricula > x->keys[i].id_matricula) i++;

    if (i < x->n && id_matricula == x->keys[i].id_matricula) {
        return (M_SearchResult){ x, i };
    }

    if (x->leaf) return (M_SearchResult){ NULL, -1 };

    return M_search(x->C[i], id_matricula);
}

void M_splitChild(BTreeMatriculas *x, int i, int t) {
    BTreeMatriculas *y = x->C[i];
    BTreeMatriculas *z = M_createNode(t, y->leaf);
    z->n = t - 1;

    for (int j = 0; j < t-1; j++) z->keys[j] = y->keys[j + t];
    if (!y->leaf) {
        for (int j = 0; j < t; j++) z->C[j] = y->C[j + t];
    }
    y->n = t - 1;

    for (int j = x->n; j >= i+1; j--) x->C[j+1] = x->C[j];
    x->C[i+1] = z;

    for (int j = x->n - 1; j >= i; j--) x->keys[j+1] = x->keys[j];
    x->keys[i] = y->keys[t-1];
    x->n++;
}

void M_insertNonFull(BTreeMatriculas *x, Matriculas k, int t) {
    int i = x->n - 1;
    if (x->leaf) {
        while (i >= 0 && x->keys[i].id_matricula > k.id_matricula) {
            x->keys[i+1] = x->keys[i];
            i--;
        }
        x->keys[i+1] = k;
        x->n++;
    } else {
        while (i >= 0 && x->keys[i].id_matricula > k.id_matricula) i--;
        i++;
        if (x->C[i]->n == 2*t - 1) {
            M_splitChild(x, i, t);
            if (x->keys[i].id_matricula < k.id_matricula) i++;
        }
        M_insertNonFull(x->C[i], k, t);
    }
}

void M_insert(BTreeM *T, Matriculas k) {
    BTreeMatriculas *r = T->root;
    int t = T->t;
    if (r->n == 2*t - 1) {
        BTreeMatriculas *s = M_createNode(t, false);
        T->root = s;
        s->C[0] = r;
        M_splitChild(s, 0, t);
        M_insertNonFull(s, k, t);
    } else {
        M_insertNonFull(r, k, t);
    }
}

void M_freeNode(BTreeMatriculas *x, int t) {
    if (!x) return;
    if (!x->leaf) for (int i = 0; i <= x->n; i++) M_freeNode(x->C[i], t);
    free(x->keys);
    free(x->C);
    free(x);
}
void M_free(BTreeM *T) {
    if (!T) return;
    M_freeNode(T->root, T->t);
    free(T);
}

FILE *abrir_arquivo_Aluno_CSV(const char *nome_arquivo, BTreeA *Al) {
    FILE *arquivoA = fopen(nome_arquivo, "r");
    if (!arquivoA) {
        perror("Erro ao abrir CSV de alunos");
        return NULL;
    }

    Alunos aluno;
    while (fscanf(arquivoA, "%lf,%199[^\n]\n", &aluno.matricula_aluno, aluno.nome_aluno) == 2) {
        A_insert(Al, aluno);
    }

    fclose(arquivoA);
    return arquivoA;
}

FILE *abrir_arquivo_Disciplina_CSV(const char *nome_arquivo, BTreeD *Ds) {
    FILE *arquivoD = fopen(nome_arquivo, "r");
    if (!arquivoD) {
        perror("Erro ao abrir CSV de disciplinas");
        return NULL;
    }

    Disciplinas disciplina;
    while (fscanf(arquivoD, " %199[^,],%199[^\n]\n", disciplina.codigo_disciplina, disciplina.nome_disciplina) == 2) {
        D_insert(Ds, disciplina);
    }

    fclose(arquivoD);
    return arquivoD;
}

FILE *abrir_arquivo_Matricula_CSV(const char *nome_arquivo, BTreeM *Ma) {
    FILE *arquivoM = fopen(nome_arquivo, "r");
    if (!arquivoM) {
        perror("Erro ao abrir CSV de matriculas");
        return NULL;
    }
    
    int id;
    double mat;
    char cod[200];
    int ano, sem;
    float media;

    while (fscanf(arquivoM, "%d,%lf,%199[^,],%d,%d,%f\n", &id, &mat, cod, &ano, &sem, &media) == 6) {
        Matriculas m;
        m.id_matricula = id;
        m.matricula_aluno.matricula_aluno = mat;
        m.matricula_aluno.nome_aluno[0] = '\0'; // ou buscar no BTree de Alunos
        strcpy(m.codigo_disciplina.codigo_disciplina, cod);
        m.ano_letivo = ano;
        m.semestre_letivo = sem;
        m.media_final = media;

        M_insert(Ma, m);
    }

    fclose(arquivoM);
    return arquivoM;
}

//--------------------Funções para persistencia-------------------------//

void salvarAlunoDisco(Alunos *a) {
    FILE *fp = fopen("alunos.db", "ab");
    if (!fp) { perror("Erro ao abrir arquivo"); return; }
    fwrite(a, sizeof(Alunos), 1, fp);
    fclose(fp);
}

bool carregarAlunosDisco(Alunos **lista, int *qtd) {
    FILE *fp = fopen("alunos.db", "rb");
    if (!fp) return false;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    *qtd = size / sizeof(Alunos);
    *lista = malloc(size);
    fseek(fp, 0, SEEK_SET);
    fread(*lista, sizeof(Alunos), *qtd, fp);
    fclose(fp);
    return true;
}

void salvarDisciplinaDisco(Disciplinas *d) {
    FILE *fp = fopen("disciplinas.db", "ab");
    if (!fp) { perror("Erro ao abrir arquivo"); return; }
    fwrite(d, sizeof(Disciplinas), 1, fp);
    fclose(fp);
}

bool carregarDisciplinasDisco(Disciplinas **lista, int *qtd) {
    FILE *fp = fopen("disciplinas.db", "rb");
    if (!fp) return false;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    *qtd = size / sizeof(Disciplinas);
    *lista = malloc(size);
    fseek(fp, 0, SEEK_SET);
    fread(*lista, sizeof(Disciplinas), *qtd, fp);
    fclose(fp);
    return true;
}

void salvarMatriculaDisco(Matriculas *m) {
    FILE *fp = fopen("matriculas.db", "ab");
    if (!fp) { perror("Erro ao abrir arquivo"); return; }
    fwrite(m, sizeof(Matriculas), 1, fp);
    fclose(fp);
}

bool carregarMatriculasDisco(Matriculas **lista, int *qtd) {
    FILE *fp = fopen("matriculas.db", "rb");
    if (!fp) return false;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    *qtd = size / sizeof(Matriculas);
    *lista = malloc(size);
    fseek(fp, 0, SEEK_SET);
    fread(*lista, sizeof(Matriculas), *qtd, fp);
    fclose(fp);
    return true;
}

//------------------------------FUNÇÕES PARA ABRIR OS ARQUIVOS------------------------------//



int main(void) {
    /* Exemplo simples para a árvore de Alunos */
    BTreeA *Ta = A_createTree(3); /* t = 3 */
    Alunos a1 = {123, "Mario"};
    Alunos a2 = {456, "Ana"};
    Alunos a3 = {222, "Paulo"};
    Alunos a4 = {999, "Julia"};
    Alunos a5 = {111, "Carlos"};

    A_insert(Ta, a1);
    A_insert(Ta, a2);
    A_insert(Ta, a3);
    A_insert(Ta, a4);
    A_insert(Ta, a5);

    printf("===== ARVORE B (Alunos) =====\n");
    A_traverse(Ta->root);

    A_SearchResult r = A_search(Ta->root, 222);
    if (r.node) {
        printf("\nEncontrado: %s (mat %.0f)\n", r.node->keys[r.pos].nome_aluno, r.node->keys[r.pos].matricula_aluno);
    } else {
        printf("\nMatricula 222 nao encontrada.\n");
    }

    A_free(Ta);

    /* Você pode testar D_insert / M_insert similarmente. */

    return 0;
}
