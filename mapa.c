#include "mapa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"

#define MAX_LINHAS 100
#define MAX_COLUNAS 100

static const char* extrairNomeArquivo(const char *caminho) {
    const char *nome = strrchr(caminho, '/');
    if (!nome) nome = strrchr(caminho, '\\');
    return nome ? nome + 1 : caminho;
}

// mapa.c - função carregarMapa atualizada
Mapa *carregarMapa(const char *arquivo) {
    if (!arquivo) {
        fprintf(stderr, "Erro: Nome de arquivo nulo\n");
        return NULL;
    }
    
    FILE *f = fopen(arquivo, "r");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo: %s\n", arquivo);
        return NULL;
    }

    Mapa *m = (Mapa*)malloc(sizeof(Mapa));
    if (!m) {
        fclose(f);
        fprintf(stderr, "Erro de alocação de memória\n");
        return NULL;
    }

    // Inicializar valores
    m->linhas = 0;
    m->colunas = 0;
    m->pontosRestantes = 0;
    m->num_fantasmas = 0;  // Inicializar contador de fantasmas
    
    // Extrair nome do arquivo (sem o caminho)
    const char *nome_arquivo = strrchr(arquivo, '/');
    if (!nome_arquivo) nome_arquivo = strrchr(arquivo, '\\');
    if (nome_arquivo) {
        strncpy(m->nome, nome_arquivo + 1, sizeof(m->nome) - 1);
    } else {
        strncpy(m->nome, arquivo, sizeof(m->nome) - 1);
    }
    m->nome[sizeof(m->nome) - 1] = '\0';

    // Ler dimensões
    char buffer[MAX_COLUNAS + 2];
    int maxColunas = 0;
    int numLinhas = 0;
    
    // Primeira passagem: contar linhas e largura
    while (fgets(buffer, sizeof(buffer), f)) {
        numLinhas++;
        int len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
            buffer[--len] = '\0';
        }
        if (len > maxColunas) maxColunas = len;
    }
    
    m->linhas = numLinhas;
    m->colunas = maxColunas;
    
    if (m->linhas == 0 || m->colunas == 0) {
        fprintf(stderr, "Mapa vazio ou inválido: %s\n", arquivo);
        fclose(f);
        free(m);
        return NULL;
    }
    
    // Voltar ao início
    rewind(f);
    
    // Alocar matriz
    m->matriz = (char**)malloc(sizeof(char*) * m->linhas);
    if (!m->matriz) {
        fclose(f);
        free(m);
        return NULL;
    }
    
    // Inicializar matriz com espaços
    for (int i = 0; i < m->linhas; i++) {
        m->matriz[i] = (char*)malloc(sizeof(char) * (m->colunas + 1));
        if (!m->matriz[i]) {
            for (int j = 0; j < i; j++) free(m->matriz[j]);
            free(m->matriz);
            fclose(f);
            free(m);
            return NULL;
        }
        
        // Preencher com espaços
        for (int j = 0; j < m->colunas; j++) {
            m->matriz[i][j] = ' ';
        }
        m->matriz[i][m->colunas] = '\0';
    }
    
    // Segunda passagem: ler conteúdo e contar fantasmas/pellets
    int linha_atual = 0;
    while (fgets(buffer, sizeof(buffer), f) && linha_atual < m->linhas) {
        int len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
            buffer[--len] = '\0';
        }
        
        // Copiar linha para matriz
        int copyLen = (len < m->colunas) ? len : m->colunas;
        for (int j = 0; j < copyLen; j++) {
            m->matriz[linha_atual][j] = buffer[j];
            
            // Contar pellets e power pellets
            char celula = buffer[j];
            if (celula == '.' || celula == 'O') {
                m->pontosRestantes++;
            }
            // Contar fantasmas
            else if (celula == 'F') {
                if (m->num_fantasmas < 10) { // Máximo 10 fantasmas
                    // Guardar posição do fantasma
                    m->fantasma_linhas[m->num_fantasmas] = linha_atual;
                    m->fantasma_colunas[m->num_fantasmas] = j;
                    m->num_fantasmas++;
                    
                    // Substituir 'F' por espaço vazio na matriz
                    m->matriz[linha_atual][j] = ' ';
                }
            }
        }
        
        linha_atual++;
    }
    
    fclose(f);
    
    // Verificar se leu todas as linhas
    if (linha_atual != m->linhas) {
        fprintf(stderr, "Aviso: Número de linhas lidas diferente do esperado\n");
    }
    
    printf("Mapa carregado: %s (%dx%d)\n", m->nome, m->colunas, m->linhas);
    printf("  Pellets: %d\n", m->pontosRestantes);
    printf("  Fantasmas: %d\n", m->num_fantasmas);
    
    // Mostrar posições dos fantasmas (para debug)
    for (int i = 0; i < m->num_fantasmas; i++) {
        printf("  Fantasma %d: (%d,%d)\n", i+1, 
               m->fantasma_linhas[i], m->fantasma_colunas[i]);
    }
    
    return m;
}

void liberarMapa(Mapa *mapa) {
    if (!mapa) return;
    
    if (mapa->matriz) {
        for (int i = 0; i < mapa->linhas; i++) {
            free(mapa->matriz[i]);
        }
        free(mapa->matriz);
    }
    free(mapa);
}

int contarPontosMapa(Mapa *mapa) {
    if (!mapa) return 0;
    return mapa->pontosRestantes;
}

int verificarCelula(Mapa *mapa, int linha, int coluna, char tipo) {
    if (!mapa || linha < 0 || linha >= mapa->linhas || 
        coluna < 0 || coluna >= mapa->colunas) {
        return 0;
    }
    return (mapa->matriz[linha][coluna] == tipo);
}

void definirCelula(Mapa *mapa, int linha, int coluna, char valor) {
    if (!mapa || linha < 0 || linha >= mapa->linhas || 
        coluna < 0 || coluna >= mapa->colunas) {
        return;
    }
    
    char antigo = mapa->matriz[linha][coluna];
    mapa->matriz[linha][coluna] = valor;
    
    // Atualizar contagem
    if ((antigo == '.' || antigo == 'O') && (valor == ' ')) {
        mapa->pontosRestantes--;
    }
}

void remover_pellet(Mapa *mapa, int linha, int coluna) {
    if (verificarCelula(mapa, linha, coluna, '.')) {
        definirCelula(mapa, linha, coluna, ' ');
    }
}

void remover_power_pellet(Mapa *mapa, int linha, int coluna) {
    if (verificarCelula(mapa, linha, coluna, 'O')) {
        definirCelula(mapa, linha, coluna, ' ');
    }
}