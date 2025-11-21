#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Configurações da Árvore B
#define T 3
#define MAX_CHAVES (2*T-1)
#define MAX_FILHOS (2*T)


typedef struct {
    double matricula_aluno;
    char nome_aluno[200];
} Aluno;

typedef struct {
    char codigo_disciplina[200]; // CHAVE: STRING
    char nome_disciplina[200];
} Disciplina;

typedef struct {
    int id_matricula; // CHAVE: INT
    Aluno aluno;
    Disciplina disciplina;
    float media_final;
    int ano_letivo;
    int semestre_letivo;
} Matricula;

typedef struct {
    Aluno chaves[MAX_CHAVES];
    long filhos[MAX_FILHOS];
    int n;
    bool folha;
} NoBAlunos;

typedef struct {
    Disciplina chaves[MAX_CHAVES];
    long filhos[MAX_FILHOS];
    int n;
    bool folha;
} NoBDisciplinas;

typedef struct {
    Matricula chaves[MAX_CHAVES];
    long filhos[MAX_FILHOS];
    int n;
    bool folha;
} NoBMatriculas;


typedef struct {
    FILE *arquivo;
    long offset_raiz;
} ArvoreBAlunos, ArvoreBDisciplinas, ArvoreBMatriculas;


long escrever_no(FILE *fp, void *no, size_t tamanho) {
    fseek(fp, 0, SEEK_END);
    long offset = ftell(fp);
    if (fwrite(no, tamanho, 1, fp) != 1) return -1;
    fflush(fp);
    return offset;
}

void ler_no(FILE *fp, void *no, size_t tamanho, long offset) {
    if (offset == -1) return;
    fseek(fp, offset, SEEK_SET);
    fread(no, tamanho, 1, fp);
}

void sobrescrever_no(FILE *fp, void *no, size_t tamanho, long offset) {
    if (offset == -1) return;
    fseek(fp, offset, SEEK_SET);
    if (fwrite(no, tamanho, 1, fp) != 1) {
        perror("Erro de sobrescrita no disco");
    }
    fflush(fp);
}

#define CRIAR_ARVORE_INICIO(TipoNo, TipoArvore, nome_funcao) \
TipoArvore* nome_funcao(const char *nome_arquivo) { \
    TipoArvore *arvore = malloc(sizeof(TipoArvore)); \
    arvore->arquivo = fopen(nome_arquivo, "r+b"); \
    if (!arvore->arquivo) arvore->arquivo = fopen(nome_arquivo, "w+b"); \
    if (!arvore->arquivo) { perror("Erro ao abrir arquivo"); exit(1); } \
    fseek(arvore->arquivo, 0, SEEK_END); \
    if (ftell(arvore->arquivo) == 0) { \
        TipoNo raiz = { .n = 0, .folha = true }; \
        for (int i = 0; i < MAX_FILHOS; i++) raiz.filhos[i] = -1; \
        arvore->offset_raiz = escrever_no(arvore->arquivo, &raiz, sizeof(TipoNo)); \
    } else { arvore->offset_raiz = 0; } \
    return arvore; \
}


CRIAR_ARVORE_INICIO(NoBAlunos, ArvoreBAlunos, criar_arvore_alunos)

bool buscar_aluno(ArvoreBAlunos *arvore, long offset_no, double matricula, Aluno *resultado) {
    if (offset_no == -1) return false;
    NoBAlunos no;
    ler_no(arvore->arquivo, &no, sizeof(NoBAlunos), offset_no);

    int i = 0;
    while (i < no.n && matricula > no.chaves[i].matricula_aluno) i++;

    if (i < no.n && matricula == no.chaves[i].matricula_aluno) {
        if (resultado) *resultado = no.chaves[i];
        return true;
    }
    if (no.folha) return false;
    return buscar_aluno(arvore, no.filhos[i], matricula, resultado);
}

void dividir_filho_aluno(FILE *fp, NoBAlunos *pai, int i, long offset_filho, long offset_pai) {
    NoBAlunos filho;
    ler_no(fp, &filho, sizeof(NoBAlunos), offset_filho);

    NoBAlunos novo_filho = { .folha = filho.folha, .n = T - 1 };

    for (int j = 0; j < T - 1; j++) novo_filho.chaves[j] = filho.chaves[j + T];
    if (!filho.folha) {
        for (int j = 0; j < T; j++) novo_filho.filhos[j] = filho.filhos[j + T];
    } else {
        for (int j = 0; j < T; j++) novo_filho.filhos[j] = -1;
    }

    for (int j = pai->n; j >= i + 1; j--) pai->filhos[j + 1] = pai->filhos[j];

    long offset_novo = escrever_no(fp, &novo_filho, sizeof(NoBAlunos));
    pai->filhos[i + 1] = offset_novo;

    for (int j = pai->n - 1; j >= i; j--) pai->chaves[j + 1] = pai->chaves[j];
    pai->chaves[i] = filho.chaves[T - 1];
    pai->n++;

    filho.n = T - 1;
    sobrescrever_no(fp, &filho, sizeof(NoBAlunos), offset_filho);
    sobrescrever_no(fp, pai, sizeof(NoBAlunos), offset_pai);
}

void inserir_nao_cheio_aluno(FILE *fp, long offset_no, Aluno k) {
    NoBAlunos no;
    ler_no(fp, &no, sizeof(NoBAlunos), offset_no);
    int i = no.n - 1;

    if (no.folha) {
        while (i >= 0 && no.chaves[i].matricula_aluno > k.matricula_aluno) {
            no.chaves[i + 1] = no.chaves[i];
            i--;
        }
        no.chaves[i + 1] = k;
        no.n++;
        sobrescrever_no(fp, &no, sizeof(NoBAlunos), offset_no);
    } else {
        while (i >= 0 && no.chaves[i].matricula_aluno > k.matricula_aluno) i--;
        i++; 

        long offset_filho = no.filhos[i];
        NoBAlunos filho;
        ler_no(fp, &filho, sizeof(NoBAlunos), offset_filho);

        if (filho.n == MAX_CHAVES) {
            dividir_filho_aluno(fp, &no, i, offset_filho, offset_no);
        
            ler_no(fp, &no, sizeof(NoBAlunos), offset_no);

            if (k.matricula_aluno > no.chaves[i].matricula_aluno) i++;
        }
        inserir_nao_cheio_aluno(fp, no.filhos[i], k);
    }
}

// Inserção principal
void inserir_aluno(ArvoreBAlunos *arvore, Aluno k) {
    NoBAlunos raiz;
    ler_no(arvore->arquivo, &raiz, sizeof(NoBAlunos), arvore->offset_raiz);

    if (raiz.n == MAX_CHAVES) {
        NoBAlunos nova_raiz = { .folha = false, .n = 0 };
        for (int i = 0; i < MAX_FILHOS; i++) nova_raiz.filhos[i] = -1;

        long offset_antiga_raiz = arvore->offset_raiz;
        arvore->offset_raiz = escrever_no(arvore->arquivo, &nova_raiz, sizeof(NoBAlunos));
        nova_raiz.filhos[0] = offset_antiga_raiz;

        dividir_filho_aluno(arvore->arquivo, &nova_raiz, 0, nova_raiz.filhos[0], arvore->offset_raiz); 

        inserir_nao_cheio_aluno(arvore->arquivo, arvore->offset_raiz, k);
    } else {
        inserir_nao_cheio_aluno(arvore->arquivo, arvore->offset_raiz, k);
    }
}


void percorrer_arvore_alunos(FILE *fp, long offset_no) {
    if (offset_no == -1) return;
    NoBAlunos no;
    ler_no(fp, &no, sizeof(NoBAlunos), offset_no);

    for (int i = 0; i < no.n; i++) {
        if (!no.folha) percorrer_arvore_alunos(fp, no.filhos[i]);
        printf("Mat: %.0f | Nome: %s\n", no.chaves[i].matricula_aluno, no.chaves[i].nome_aluno);
    }
    if (!no.folha) percorrer_arvore_alunos(fp, no.filhos[no.n]);
}


CRIAR_ARVORE_INICIO(NoBDisciplinas, ArvoreBDisciplinas, criar_arvore_disciplinas)

bool buscar_disciplina(ArvoreBDisciplinas *arvore, long offset_no, const char *codigo, Disciplina *resultado) {
    if (offset_no == -1) return false;
    NoBDisciplinas no;
    ler_no(arvore->arquivo, &no, sizeof(NoBDisciplinas), offset_no);

    int i = 0;
    while (i < no.n && strcmp(codigo, no.chaves[i].codigo_disciplina) > 0) i++;

    if (i < no.n && strcmp(codigo, no.chaves[i].codigo_disciplina) == 0) {
        if (resultado) *resultado = no.chaves[i];
        return true;
    }
    if (no.folha) return false;
    return buscar_disciplina(arvore, no.filhos[i], codigo, resultado);
}

void dividir_filho_disciplina(FILE *fp, NoBDisciplinas *pai, int i, long offset_filho, long offset_pai) {
    NoBDisciplinas filho;
    ler_no(fp, &filho, sizeof(NoBDisciplinas), offset_filho);

    NoBDisciplinas novo_filho = { .folha = filho.folha, .n = T - 1 };

    for (int j = 0; j < T - 1; j++) novo_filho.chaves[j] = filho.chaves[j + T];
    if (!filho.folha) {
        for (int j = 0; j < T; j++) novo_filho.filhos[j] = filho.filhos[j + T];
    } else {
        for (int j = 0; j < T; j++) novo_filho.filhos[j] = -1;
    }

    for (int j = pai->n; j >= i + 1; j--) pai->filhos[j + 1] = pai->filhos[j];

    long offset_novo = escrever_no(fp, &novo_filho, sizeof(NoBDisciplinas));
    pai->filhos[i + 1] = offset_novo;

    for (int j = pai->n - 1; j >= i; j--) pai->chaves[j + 1] = pai->chaves[j];
    pai->chaves[i] = filho.chaves[T - 1];
    pai->n++;

    filho.n = T - 1;
    sobrescrever_no(fp, &filho, sizeof(NoBDisciplinas), offset_filho);
    sobrescrever_no(fp, pai, sizeof(NoBDisciplinas), offset_pai);
}

void inserir_nao_cheio_disciplina(FILE *fp, long offset_no, Disciplina k) {
    NoBDisciplinas no;
    ler_no(fp, &no, sizeof(NoBDisciplinas), offset_no);
    int i = no.n - 1;

    if (no.folha) {
        while (i >= 0 && strcmp(no.chaves[i].codigo_disciplina, k.codigo_disciplina) > 0) {
            no.chaves[i + 1] = no.chaves[i];
            i--;
        }
        no.chaves[i + 1] = k;
        no.n++;
        sobrescrever_no(fp, &no, sizeof(NoBDisciplinas), offset_no);
    } else {
        while (i >= 0 && strcmp(no.chaves[i].codigo_disciplina, k.codigo_disciplina) > 0) i--;
        i++; 

        long offset_filho = no.filhos[i];
        NoBDisciplinas filho;
        ler_no(fp, &filho, sizeof(NoBDisciplinas), offset_filho);

        if (filho.n == MAX_CHAVES) {
            dividir_filho_disciplina(fp, &no, i, offset_filho, offset_no);
            ler_no(fp, &no, sizeof(NoBDisciplinas), offset_no);

            if (strcmp(k.codigo_disciplina, no.chaves[i].codigo_disciplina) > 0) i++;
        }
        inserir_nao_cheio_disciplina(fp, no.filhos[i], k);
    }
}

// Inserção principal
void inserir_disciplina(ArvoreBDisciplinas *arvore, Disciplina k) {
    NoBDisciplinas raiz;
    ler_no(arvore->arquivo, &raiz, sizeof(NoBDisciplinas), arvore->offset_raiz);

    if (raiz.n == MAX_CHAVES) {
        NoBDisciplinas nova_raiz = { .folha = false, .n = 0 };
        for (int i = 0; i < MAX_FILHOS; i++) nova_raiz.filhos[i] = -1;

        long offset_antiga_raiz = arvore->offset_raiz;
        arvore->offset_raiz = escrever_no(arvore->arquivo, &nova_raiz, sizeof(NoBDisciplinas));
        nova_raiz.filhos[0] = offset_antiga_raiz;

        dividir_filho_disciplina(arvore->arquivo, &nova_raiz, 0, nova_raiz.filhos[0], arvore->offset_raiz); 

        inserir_nao_cheio_disciplina(arvore->arquivo, arvore->offset_raiz, k);
    } else {
        inserir_nao_cheio_disciplina(arvore->arquivo, arvore->offset_raiz, k);
    }
}

// Percorrer a árvore e imprimir (Disciplinas)
void percorrer_arvore_disciplinas(FILE *fp, long offset_no) {
    if (offset_no == -1) return;
    NoBDisciplinas no;
    ler_no(fp, &no, sizeof(NoBDisciplinas), offset_no);

    for (int i = 0; i < no.n; i++) {
        if (!no.folha) percorrer_arvore_disciplinas(fp, no.filhos[i]);
        printf("Codigo: %s | Nome: %s\n", no.chaves[i].codigo_disciplina, no.chaves[i].nome_disciplina);
    }
    if (!no.folha) percorrer_arvore_disciplinas(fp, no.filhos[no.n]);
}

CRIAR_ARVORE_INICIO(NoBMatriculas, ArvoreBMatriculas, criar_arvore_matriculas)

// Busca (Chave: int)
bool buscar_matricula(ArvoreBMatriculas *arvore, long offset_no, int id_matricula, Matricula *resultado) {
    if (offset_no == -1) return false;
    NoBMatriculas no;
    ler_no(arvore->arquivo, &no, sizeof(NoBMatriculas), offset_no);

    int i = 0;
    while (i < no.n && id_matricula > no.chaves[i].id_matricula) i++;

    if (i < no.n && id_matricula == no.chaves[i].id_matricula) {
        if (resultado) *resultado = no.chaves[i];
        return true;
    }
    if (no.folha) return false;
    return buscar_matricula(arvore, no.filhos[i], id_matricula, resultado);
}

// Divide o filho (Chave: int)
void dividir_filho_matricula(FILE *fp, NoBMatriculas *pai, int i, long offset_filho, long offset_pai) {
    NoBMatriculas filho;
    ler_no(fp, &filho, sizeof(NoBMatriculas), offset_filho);

    NoBMatriculas novo_filho = { .folha = filho.folha, .n = T - 1 };

    for (int j = 0; j < T - 1; j++) novo_filho.chaves[j] = filho.chaves[j + T];
    if (!filho.folha) {
        for (int j = 0; j < T; j++) novo_filho.filhos[j] = filho.filhos[j + T];
    } else {
        for (int j = 0; j < T; j++) novo_filho.filhos[j] = -1;
    }

    for (int j = pai->n; j >= i + 1; j--) pai->filhos[j + 1] = pai->filhos[j];

    long offset_novo = escrever_no(fp, &novo_filho, sizeof(NoBMatriculas));
    pai->filhos[i + 1] = offset_novo;

    for (int j = pai->n - 1; j >= i; j--) pai->chaves[j + 1] = pai->chaves[j];
    pai->chaves[i] = filho.chaves[T - 1];
    pai->n++;

    filho.n = T - 1;
    sobrescrever_no(fp, &filho, sizeof(NoBMatriculas), offset_filho);
    sobrescrever_no(fp, pai, sizeof(NoBMatriculas), offset_pai);
}

// Inserção em nó não cheio (Chave: int)
void inserir_nao_cheio_matricula(FILE *fp, long offset_no, Matricula k) {
    NoBMatriculas no;
    ler_no(fp, &no, sizeof(NoBMatriculas), offset_no);
    int i = no.n - 1;

    if (no.folha) {
        while (i >= 0 && no.chaves[i].id_matricula > k.id_matricula) {
            no.chaves[i + 1] = no.chaves[i];
            i--;
        }
        no.chaves[i + 1] = k;
        no.n++;
        sobrescrever_no(fp, &no, sizeof(NoBMatriculas), offset_no);
    } else {
        while (i >= 0 && no.chaves[i].id_matricula > k.id_matricula) i--;
        i++; 

        long offset_filho = no.filhos[i];
        NoBMatriculas filho;
        ler_no(fp, &filho, sizeof(NoBMatriculas), offset_filho);

        if (filho.n == MAX_CHAVES) {
            dividir_filho_matricula(fp, &no, i, offset_filho, offset_no);
            ler_no(fp, &no, sizeof(NoBMatriculas), offset_no);

            if (k.id_matricula > no.chaves[i].id_matricula) i++;
        }
        inserir_nao_cheio_matricula(fp, no.filhos[i], k);
    }
}

// Inserção principal
void inserir_matricula(ArvoreBMatriculas *arvore, Matricula k) {
    NoBMatriculas raiz;
    ler_no(arvore->arquivo, &raiz, sizeof(NoBMatriculas), arvore->offset_raiz);

    if (raiz.n == MAX_CHAVES) {
        NoBMatriculas nova_raiz = { .folha = false, .n = 0 };
        for (int i = 0; i < MAX_FILHOS; i++) nova_raiz.filhos[i] = -1;

        long offset_antiga_raiz = arvore->offset_raiz;
        arvore->offset_raiz = escrever_no(arvore->arquivo, &nova_raiz, sizeof(NoBMatriculas));
        nova_raiz.filhos[0] = offset_antiga_raiz;

        dividir_filho_matricula(arvore->arquivo, &nova_raiz, 0, nova_raiz.filhos[0], arvore->offset_raiz);

        inserir_nao_cheio_matricula(arvore->arquivo, arvore->offset_raiz, k);
    } else {
        inserir_nao_cheio_matricula(arvore->arquivo, arvore->offset_raiz, k);
    }
}

void percorrer_arvore_matriculas(FILE *fp, long offset_no) {
    if (offset_no == -1) return;
    NoBMatriculas no;
    ler_no(fp, &no, sizeof(NoBMatriculas), offset_no);

    for (int i = 0; i < no.n; i++) {
        if (!no.folha) percorrer_arvore_matriculas(fp, no.filhos[i]);
        printf("ID Mat: %d | Aluno Mat: %.0f\n", no.chaves[i].id_matricula, no.chaves[i].aluno.matricula_aluno);
    }
    if (!no.folha) percorrer_arvore_matriculas(fp, no.filhos[no.n]);
}

//-------------------------------FUNÇÕES PARA ABRIR ARQUIVOS----------------------//

FILE *abrir_csv(const char *nome_arquivo, const char *descricao) {
    FILE *fp = fopen(nome_arquivo, "r");
    if (!fp) {
        perror(descricao);
    }
    return fp;
}

void carregar_alunos_csv(ArvoreBAlunos *arvore_a, const char *nome_arquivo_csv) {
    FILE *arquivoA = abrir_csv(nome_arquivo_csv, "Erro ao abrir CSV de alunos");
    if (!arquivoA) return;

    double mat;
    char nome[200];
    
    while (fscanf(arquivoA, "%lf,%199[^\n]\n", &mat, nome) == 2) {
        Aluno a;
        a.matricula_aluno = mat;
        strcpy(a.nome_aluno, nome);
        
        inserir_aluno(arvore_a, a); 
    }

    fclose(arquivoA);
    printf("Carregamento de Alunos concluído.\n");
}

void carregar_disciplinas_csv(ArvoreBDisciplinas *arvore_d, const char *nome_arquivo_csv) {
    FILE *arquivoD = abrir_csv(nome_arquivo_csv, "Erro ao abrir CSV de disciplinas");
    if (!arquivoD) return;

    char cod[200];
    char nome[200];
    
    while (fscanf(arquivoD, "%199[^,],%199[^\n]\n", cod, nome) == 2) {
        Disciplina d;
        strcpy(d.codigo_disciplina, cod);
        strcpy(d.nome_disciplina, nome);
        
        inserir_disciplina(arvore_d, d);
    }

    fclose(arquivoD);
    printf("Carregamento de Disciplinas concluído.\n");
}

void carregar_matriculas_csv(
    ArvoreBMatriculas *arvore_m, 
    ArvoreBAlunos *arvore_a, 
    ArvoreBDisciplinas *arvore_d, 
    const char *nome_arquivo_csv
) {
    FILE *arquivoM = abrir_csv(nome_arquivo_csv, "Erro ao abrir CSV de matrículas");
    if (!arquivoM) return;
    
    int id;
    double mat_aluno; // Chave de busca para Aluno
    char cod_disciplina[200]; // Chave de busca para Disciplina
    int ano, sem;
    float media;

    while (fscanf(arquivoM, "%d,%lf,%199[^,],%d,%d,%f\n", &id, &mat_aluno, cod_disciplina, &ano, &sem, &media) == 6) {
        Matricula m = {0}; 
        m.id_matricula = id;
        m.ano_letivo = ano;
        m.semestre_letivo = sem;
        m.media_final = media;

        if (buscar_aluno(arvore_a, arvore_a->offset_raiz, mat_aluno, &m.aluno)) {
          
        } else {
            fprintf(stderr, "Aviso: Aluno com matrícula %.0f não encontrado. Matricula ID %d ignorada.\n", mat_aluno, id);
            continue; 
        }
        
      
        if (buscar_disciplina(arvore_d, arvore_d->offset_raiz, cod_disciplina, &m.disciplina)) {
            
        } else {
            fprintf(stderr, "Aviso: Disciplina com código %s não encontrada. Matricula ID %d ignorada.\n", cod_disciplina, id);
            continue; 
        }

        inserir_matricula(arvore_m, m);
    }

    fclose(arquivoM);
    printf("Carregamento de Matrículas concluído.\n");
}

//--------------------------------FUNÇÕES PARA DELETAR-----------------------//



void remover_chave_aluno_no(NoBAlunos *no, int idx_chave) {
    for (int j = idx_chave; j < no->n - 1; j++) {
        no->chaves[j] = no->chaves[j + 1];
    }
    no->n--;
}


void remover_chave_disciplina_no(NoBDisciplinas *no, int idx_chave) {
    for (int j = idx_chave; j < no->n - 1; j++) {
        no->chaves[j] = no->chaves[j + 1];
    }
    no->n--;
}


void remover_chave_matricula_no(NoBMatriculas *no, int idx_chave) {
    for (int j = idx_chave; j < no->n - 1; j++) {
        no->chaves[j] = no->chaves[j + 1];
    }
    no->n--;
}

bool buscar_e_remover_matricula_no(FILE *fp, long offset_no, int id_matricula) {
    if (offset_no == -1) return false;

    NoBMatriculas no;
    ler_no(fp, &no, sizeof(NoBMatriculas), offset_no);
    int i = 0;

    while (i < no.n && id_matricula > no.chaves[i].id_matricula) i++;

    if (i < no.n && id_matricula == no.chaves[i].id_matricula) {
        if (no.folha) {
        
            remover_chave_matricula_no(&no, i);
            sobrescrever_no(fp, &no, sizeof(NoBMatriculas), offset_no);
            return true;
        } else {
            
            remover_chave_matricula_no(&no, i);
            sobrescrever_no(fp, &no, sizeof(NoBMatriculas), offset_no);
            return true;
        }
    }

   
    if (no.folha) return false;
    return buscar_e_remover_matricula_no(fp, no.filhos[i], id_matricula);
}


void deletar_matricula(ArvoreBMatriculas *arvore, int id_matricula) {
    if (buscar_e_remover_matricula_no(arvore->arquivo, arvore->offset_raiz, id_matricula)) {
        printf("Matricula ID %d deletada com sucesso.\n", id_matricula);
    } else {
        printf("Erro: Matricula ID %d não encontrada para deleção.\n", id_matricula);
    }
}


void deletar_matriculas_por_aluno(FILE *fp_m, long offset_no, double matricula_aluno) {
    if (offset_no == -1) return;

    NoBMatriculas no;
    ler_no(fp_m, &no, sizeof(NoBMatriculas), offset_no);
    bool no_modificado = false;

    
    if (!no.folha) {
        for (int i = 0; i <= no.n; i++) {
            deletar_matriculas_por_aluno(fp_m, no.filhos[i], matricula_aluno);
        }
    }

    
    int i = 0;
    while (i < no.n) {
        if (no.chaves[i].aluno.matricula_aluno == matricula_aluno) {
           
            printf("  -> Deletando Matrícula ID %d em cascata.\n", no.chaves[i].id_matricula);
            remover_chave_matricula_no(&no, i);
            no_modificado = true;
            
        } else {
            i++;
        }
    }

  
    if (no_modificado) {
        sobrescrever_no(fp_m, &no, sizeof(NoBMatriculas), offset_no);
    }
}


void deletar_aluno(ArvoreBAlunos *arvore_a, ArvoreBMatriculas *arvore_m, double matricula) {
  
    NoBAlunos raiz;
    ler_no(arvore_a->arquivo, &raiz, sizeof(NoBAlunos), arvore_a->offset_raiz);
    

    int i = 0;
    while (i < raiz.n && matricula > raiz.chaves[i].matricula_aluno) i++;

    if (i < raiz.n && matricula == raiz.chaves[i].matricula_aluno) {
        if (raiz.folha) {
            remover_chave_aluno_no(&raiz, i);
            sobrescrever_no(arvore_a->arquivo, &raiz, sizeof(NoBAlunos), arvore_a->offset_raiz);
            printf("Aluno %.0f deletado.\n", matricula);
          
            printf("Iniciando exclusão em cascata de Matrículas associadas...\n");
            deletar_matriculas_por_aluno(arvore_m->arquivo, arvore_m->offset_raiz, matricula);
            printf("Exclusão em cascata concluída.\n");
            
            return;
        }
    }
    
    printf("Erro: Aluno %.0f não encontrado para deleção ou deleção em nó interno não suportada.\n", matricula);
}


void deletar_matriculas_por_disciplina(FILE *fp_m, long offset_no, const char *codigo_disciplina) {
    if (offset_no == -1) return;

    NoBMatriculas no;
    ler_no(fp_m, &no, sizeof(NoBMatriculas), offset_no);
    bool no_modificado = false;

   
    if (!no.folha) {
        for (int i = 0; i <= no.n; i++) {
            deletar_matriculas_por_disciplina(fp_m, no.filhos[i], codigo_disciplina);
        }
    }

   
    int i = 0;
    while (i < no.n) {
        if (strcmp(no.chaves[i].disciplina.codigo_disciplina, codigo_disciplina) == 0) {
           
            printf("  -> Deletando Matrícula ID %d em cascata.\n", no.chaves[i].id_matricula);
            remover_chave_matricula_no(&no, i);
            no_modificado = true;
    
        } else {
            i++; 
        }
    }

    if (no_modificado) {
        sobrescrever_no(fp_m, &no, sizeof(NoBMatriculas), offset_no);
    }
}



void deletar_disciplina(ArvoreBDisciplinas *arvore_d, ArvoreBMatriculas *arvore_m, const char *codigo) {


    NoBDisciplinas raiz;
    ler_no(arvore_d->arquivo, &raiz, sizeof(NoBDisciplinas), arvore_d->offset_raiz);
    
    int i = 0;
    while (i < raiz.n && strcmp(codigo, raiz.chaves[i].codigo_disciplina) > 0) i++;

    if (i < raiz.n && strcmp(codigo, raiz.chaves[i].codigo_disciplina) == 0) {
        if (raiz.folha) {
            remover_chave_disciplina_no(&raiz, i);
            sobrescrever_no(arvore_d->arquivo, &raiz, sizeof(NoBDisciplinas), arvore_d->offset_raiz);
            printf("Disciplina %s deletada.\n", codigo);
            
            printf("Iniciando exclusão em cascata de Matrículas associadas...\n");
            deletar_matriculas_por_disciplina(arvore_m->arquivo, arvore_m->offset_raiz, codigo);
            printf("Exclusão em cascata concluída.\n");
            
            return;
        }
    }
    
    printf("Erro: Disciplina %s não encontrada para deleção ou deleção em nó interno não suportada.\n", codigo);
}



bool buscar_e_atualizar_matricula(FILE *fp, long offset_no, int id_matricula, float nova_media) {
    if (offset_no == -1) {
        return false; 
    }

    NoBMatriculas no;
    ler_no(fp, &no, sizeof(NoBMatriculas), offset_no);
    int i = 0;

    while (i < no.n && id_matricula > no.chaves[i].id_matricula) {
        i++;
    }

  
    if (i < no.n && id_matricula == no.chaves[i].id_matricula) {
    
        printf("Matrícula ID %d encontrada. Média anterior: %.2f\n", id_matricula, no.chaves[i].media_final);
        
        no.chaves[i].media_final = nova_media;
        
        sobrescrever_no(fp, &no, sizeof(NoBMatriculas), offset_no);
        
        printf("Média final atualizada para: %.2f\n", nova_media);
        return true;
    }

    if (no.folha) {
        return false;
    }
    
    return buscar_e_atualizar_matricula(fp, no.filhos[i], id_matricula, nova_media);
}


void atualizar_media_final(ArvoreBMatriculas *arvore, int id_matricula, float nova_media) {
    if (buscar_e_atualizar_matricula(arvore->arquivo, arvore->offset_raiz, id_matricula, nova_media)) {
        printf("Atualização da Matrícula ID %d concluída com sucesso.\n", id_matricula);
    } else {
        printf("Erro: Matrícula ID %d não encontrada para atualização.\n", id_matricula);
    }
}

int main(void) {
    
    printf("\n--- INICIALIZANDO ARVORES ---\n");
    remove("alunos.db");
    remove("disciplinas.db");
    remove("matriculas.db");

    ArvoreBAlunos *arvore_a = criar_arvore_alunos("alunos.db");
    ArvoreBDisciplinas *arvore_d = criar_arvore_disciplinas("disciplinas.db");
    ArvoreBMatriculas *arvore_m = criar_arvore_matriculas("matriculas.db");

    printf("\n--- CARREGANDO DADOS ---\n");
    carregar_alunos_csv(arvore_a, "alunos.csv");
    carregar_disciplinas_csv(arvore_d, "disciplinas.csv");
    carregar_matriculas_csv(arvore_m, arvore_a, arvore_d, "matriculas.csv");
    printf("\nCarregamento inicial de dados concluído.\n");

    int op;
    double mat_aluno_busca, mat_aluno_del;
    char cod_disciplina_busca[200], cod_disciplina_del[200];
    int id_matricula_busca, id_matricula_del, id_matricula_upd;
    float nova_media;

    Aluno a_res;
    Disciplina d_res;
    Matricula m_res;

    do {
        printf("\n\n-------------- MENU CRUD -------------\n");
        printf("1. Buscar Aluno (Read)\n");
        printf("2. Buscar Disciplina (Read)\n");
        printf("3. Buscar Matricula (Read)\n");
        printf("--------------------------------------\n");
        printf("4. Deletar Aluno (Cascata)\n");
        printf("5. Deletar Disciplina (Cascata)\n");
        printf("6. Deletar Matricula (Simples)\n");
        printf("--------------------------------------\n");
        printf("7. Mudar Média (Update)\n");
        printf("8. Imprimir Árvore de Alunos\n");
        printf("9. Imprimir Árvore de Matrículas\n");
        printf("0. Sair e Fechar Arquivos\n");
        printf("--------------------------------------\n");
        printf("Digite uma opção válida: ");
        
        if (scanf("%d", &op) != 1) {
            printf("\nOpção inválida. Tente novamente.\n");
    
            while (getchar() != '\n');
            op = -1; 
            continue;
        }

        switch(op) {
            case 1:
                printf("Digite a matrícula do Aluno a buscar (ex: 111): ");
                scanf("%lf", &mat_aluno_busca);
                if (buscar_aluno(arvore_a, arvore_a->offset_raiz, mat_aluno_busca, &a_res)) {
                    printf("Aluno encontrado: %.0f | %s\n", a_res.matricula_aluno, a_res.nome_aluno);
                } else {
                    printf("Aluno com matrícula %.0f não encontrado.\n", mat_aluno_busca);
                }
                break;
            
            case 2:
                printf("Digite o código da Disciplina a buscar (ex: CS101): ");
                scanf("%199s", cod_disciplina_busca);
                if (buscar_disciplina(arvore_d, arvore_d->offset_raiz, cod_disciplina_busca, &d_res)) {
                    printf("Disciplina encontrada: %s | %s\n", d_res.codigo_disciplina, d_res.nome_disciplina);
                } else {
                    printf("Disciplina com código %s não encontrada.\n", cod_disciplina_busca);
                }
                break;
            
            case 3: 
                printf("Digite o ID da Matrícula a buscar (ex: 101): ");
                scanf("%d", &id_matricula_busca);
                if (buscar_matricula(arvore_m, arvore_m->offset_raiz, id_matricula_busca, &m_res)) {
                    printf("Matrícula encontrada: ID %d | Aluno: %.0f (%s) | Disc: %s | Média: %.2f\n", 
                        m_res.id_matricula, m_res.aluno.matricula_aluno, m_res.aluno.nome_aluno, 
                        m_res.disciplina.codigo_disciplina, m_res.media_final);
                } else {
                    printf("Matrícula com ID %d não encontrada.\n", id_matricula_busca);
                }
                break;

            case 4:
                printf("Digite a matrícula do Aluno a DELETAR (ex: 111): ");
                scanf("%lf", &mat_aluno_del);
                deletar_aluno(arvore_a, arvore_m, mat_aluno_del);
                break;

            case 5:
                printf("Digite o código da Disciplina a DELETAR (ex: CS101): ");
                scanf("%199s", cod_disciplina_del);
                deletar_disciplina(arvore_d, arvore_m, cod_disciplina_del);
                break;

            case 6: 
                printf("Digite o ID da Matrícula a DELETAR (ex: 102): ");
                scanf("%d", &id_matricula_del);
                deletar_matricula(arvore_m, id_matricula_del);
                break;

            case 7:
                printf("Digite o ID da Matrícula para mudar a média (ex: 101): ");
                scanf("%d", &id_matricula_upd);
                printf("Digite a NOVA Média Final (ex: 10.0): ");
                scanf("%f", &nova_media);
                atualizar_media_final(arvore_m, id_matricula_upd, nova_media);
                break;

            case 8:
                printf("\n--- ÁRVORE B DE ALUNOS ---\n");
                percorrer_arvore_alunos(arvore_a->arquivo, arvore_a->offset_raiz);
                break;

            case 9:
                printf("\n--- ÁRVORE B DE MATRÍCULAS ---\n");
                percorrer_arvore_matriculas(arvore_m->arquivo, arvore_m->offset_raiz);
                break;
                
            case 0:
                printf("\nEncerrando o programa.\n");
                break;
                
            default:
                printf("\nOpção inválida. Digite um número de 0 a 9.\n");
                break;
        }
    } while (op != 0);

    printf("\nFechando arquivos de banco de dados e liberando memória...\n");
    if (arvore_a) { fclose(arvore_a->arquivo); free(arvore_a); }
    if (arvore_d) { fclose(arvore_d->arquivo); free(arvore_d); }
    if (arvore_m) { fclose(arvore_m->arquivo); free(arvore_m); }

    return 0;
}