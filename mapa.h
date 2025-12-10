#ifndef MAPA_H
#define MAPA_H

// Definições atualizadas
typedef struct {
    char **matriz;
    int linhas;
    int colunas;
    int pontosRestantes;  // pellets restantes
    char nome[100];       // nome do arquivo
    
    // Para armazenar posições dos fantasmas
    int num_fantasmas;
    int fantasma_linhas[10];  // máximo 10 fantasmas
    int fantasma_colunas[10];
} Mapa;

// Funções principais
Mapa* criar_mapa(void);
void liberar_mapa(Mapa* mapa);
int carregar_mapa(Mapa* mapa, const char* caminho_arquivo);
void imprimir_mapa(Mapa* mapa);

// Funções utilitárias
char elemento_no(Mapa* mapa, int linha, int coluna);
void remover_pellet(Mapa* mapa, int linha, int coluna);
void remover_power_pellet(Mapa* mapa, int linha, int coluna);

// Novas funções para compatibilidade
Mapa* carregarMapa(const char* arquivo);
void liberarMapa(Mapa* mapa);
int contarPontosMapa(Mapa* mapa);
int verificarCelula(Mapa* mapa, int linha, int coluna, char tipo);
void definirCelula(Mapa* mapa, int linha, int coluna, char valor);

// Função auxiliar para debug
void mostrar_posicoes_fantasmas(Mapa* mapa);

#endif