// entidades.c - VERSÃO COMPLETA ATUALIZADA
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "entidades.h"
#include "mapa.h"
#include "colisoes.h"
#include "graficos.h"

// Variáveis globais
PacMan* pacman_global = NULL;
Fantasma* fantasmas_global = NULL;

// Variáveis de velocidade do Pac-Man
static int pacman_speed_factor = 2;
static int pacman_frame_counter = 0;

// Variável de velocidade do Fantasma (fator comum a todos)
static int fantasma_speed_factor = 15;  // Valor maior = mais lento

// ================== FUNÇÕES AUXILIARES ==================

static int contar_fantasmas_lista(Fantasma* lista) {
    int count = 0;
    Fantasma* atual = lista;
    while (atual) {
        count++;
        atual = atual->prox;
    }
    return count;
}

static Fantasma* obter_fantasma_por_indice(Fantasma* lista, int index) {
    Fantasma* atual = lista;
    int contador = 0;
    
    while (atual) {
        if (contador == index) {
            return atual;
        }
        contador++;
        atual = atual->prox;
    }
    
    return NULL;
}

// ================== FUNÇÕES DE CONTROLE DE VELOCIDADE DO PAC-MAN ==================

void set_pacman_speed(int speed) {
    if (speed >= 1 && speed <= 5) {
        pacman_speed_factor = speed;
        printf("Velocidade do Pac-Man ajustada para: %d\n", speed);
    }
}

int get_pacman_speed(void) {
    return pacman_speed_factor;
}

void reset_pacman_speed(void) {
    pacman_speed_factor = 2;
    pacman_frame_counter = 0;
}

void increase_pacman_speed(void) {
    if (pacman_speed_factor > 1) {
        pacman_speed_factor--;
        printf("Pac-Man mais rápido: %d\n", pacman_speed_factor);
    }
}

void decrease_pacman_speed(void) {
    if (pacman_speed_factor < 5) {
        pacman_speed_factor++;
        printf("Pac-Man mais lento: %d\n", pacman_speed_factor);
    }
}

// ================== FUNÇÕES DE CONTROLE DE VELOCIDADE DO FANTASMA ==================

void set_fantasma_speed(int speed) {
    if (speed >= 1 && speed <= 5) {
        fantasma_speed_factor = speed;
        printf("Velocidade do Fantasma ajustada para: %d\n", speed);
    }
}

int get_fantasma_speed(void) {
    return fantasma_speed_factor;
}

void reset_fantasma_speed(void) {
    fantasma_speed_factor = 3;
    // Resetar contadores de todos os fantasmas
    Fantasma* atual = fantasmas_global;
    while (atual) {
        atual->frame_counter = 0;
        atual = atual->prox;
    }
}

void increase_fantasma_speed(void) {
    if (fantasma_speed_factor > 1) {
        fantasma_speed_factor--;
        printf("Fantasma mais rápido: %d\n", fantasma_speed_factor);
    }
}

void decrease_fantasma_speed(void) {
    if (fantasma_speed_factor < 5) {
        fantasma_speed_factor++;
        printf("Fantasma mais lento: %d\n", fantasma_speed_factor);
    }
}

// ================== FUNÇÕES BÁSICAS DE CRIAÇÃO ==================

PacMan* criar_pacman(int linha, int coluna) {
    PacMan* pacman = (PacMan*)malloc(sizeof(PacMan));
    if (!pacman) {
        printf("Erro ao alocar memoria para Pac-Man\n");
        return NULL;
    }
    
    pacman->pos.linha = linha;
    pacman->pos.coluna = coluna;
    pacman->vidas = 3;
    pacman->pontos = 0;
    pacman->power_timer = 0;
    pacman->direcao = 4; // Começa virado para direita
    
    printf("Pac-Man criado em (%d, %d)\n", linha, coluna);
    return pacman;
}

Fantasma* criar_fantasma(int linha, int coluna) {
    Fantasma* fantasma = (Fantasma*)malloc(sizeof(Fantasma));
    if (!fantasma) {
        printf("Erro ao alocar memoria para fantasma\n");
        return NULL;
    }
    
    fantasma->pos.linha = linha;
    fantasma->pos.coluna = coluna;
    fantasma->vulneravel = 0;
    fantasma->timer_vulneravel = 0;
    fantasma->direcao = rand() % 4; // Direção aleatória
    fantasma->frame_counter = 0;    // Contador individual inicializado
    fantasma->prox = NULL;
    
    return fantasma;
}

void liberar_fantasmas(Fantasma* lista) {
    Fantasma* atual = lista;
    Fantasma* proximo;
    
    while (atual) {
        proximo = atual->prox;
        free(atual);
        atual = proximo;
    }
    
    printf("Fantasma(s) liberado(s) da memoria\n");
}

Fantasma* adicionar_fantasma(Fantasma* lista, int linha, int coluna) {
    Fantasma* novo = criar_fantasma(linha, coluna);
    if (!novo) return lista;
    
    novo->prox = lista;
    printf("Fantasma adicionado em (%d, %d)\n", linha, coluna);
    return novo;
}

// ================== FUNÇÕES DE INICIALIZAÇÃO ==================

int inicializarEntidades(Mapa* mapa) {
    if (!mapa) {
        printf("Erro: Mapa nulo ao inicializar entidades\n");
        return 0;
    }
    
    // Inicializar random
    srand((unsigned int)time(NULL));
    
    printf("\n=== INICIALIZANDO ENTIDADES ===\n");
    printf("Mapa: %s (%dx%d)\n", mapa->nome, mapa->colunas, mapa->linhas);
    
    // 1. ENCONTRAR POSIÇÃO PARA PAC-MAN
    printf("\n1. Procurando posição para Pac-Man...\n");
    
    int pacman_encontrado = 0;
    int pacman_linha = mapa->linhas / 2;
    int pacman_coluna = mapa->colunas / 2;
    
    // Tentar posição central primeiro
    if (pacman_linha >= 0 && pacman_linha < mapa->linhas &&
        pacman_coluna >= 0 && pacman_coluna < mapa->colunas &&
        mapa->matriz[pacman_linha][pacman_coluna] != '#') {
        
        pacman_encontrado = 1;
        printf("  Posição central válida: (%d,%d)\n", pacman_linha, pacman_coluna);
    }
    
    // Se posição central não for válida, procurar em volta
    if (!pacman_encontrado) {
        printf("  Posição central inválida, procurando alternativas...\n");
        
        int raio = 1;
        int max_raio = (mapa->linhas < mapa->colunas) ? mapa->linhas/2 : mapa->colunas/2;
        
        while (!pacman_encontrado && raio <= max_raio) {
            // Procurar em todas as direções a partir do centro
            for (int i = -raio; i <= raio && !pacman_encontrado; i++) {
                for (int j = -raio; j <= raio && !pacman_encontrado; j++) {
                    // Ignorar posições dentro do raio anterior
                    if (abs(i) == raio || abs(j) == raio) {
                        int linha_teste = mapa->linhas/2 + i;
                        int coluna_teste = mapa->colunas/2 + j;
                        
                        if (linha_teste >= 0 && linha_teste < mapa->linhas &&
                            coluna_teste >= 0 && coluna_teste < mapa->colunas &&
                            mapa->matriz[linha_teste][coluna_teste] != '#') {
                            
                            pacman_linha = linha_teste;
                            pacman_coluna = coluna_teste;
                            pacman_encontrado = 1;
                            printf("  Posição encontrada: (%d,%d) (raio %d)\n", 
                                   pacman_linha, pacman_coluna, raio);
                        }
                    }
                }
            }
            raio++;
        }
    }
    
    // Se ainda não encontrou, procurar em todo o mapa
    if (!pacman_encontrado) {
        printf("  Busca radial falhou, procurando em todo o mapa...\n");
        
        for (int i = 0; i < mapa->linhas && !pacman_encontrado; i++) {
            for (int j = 0; j < mapa->colunas && !pacman_encontrado; j++) {
                if (mapa->matriz[i][j] != '#') {
                    pacman_linha = i;
                    pacman_coluna = j;
                    pacman_encontrado = 1;
                    printf("  Posição encontrada: (%d,%d)\n", pacman_linha, pacman_coluna);
                }
            }
        }
    }
    
    if (!pacman_encontrado) {
        printf("ERRO CRÍTICO: Não foi possível encontrar posição para Pac-Man!\n");
        return 0;
    }
    
    // Criar Pac-Man
    pacman_global = criar_pacman(pacman_linha, pacman_coluna);
    if (!pacman_global) {
        printf("Erro critico: Nao foi possivel criar Pac-Man\n");
        return 0;
    }
    
    printf("  Pac-Man criado em (%d,%d)\n", pacman_linha, pacman_coluna);
    
    // 2. CRIAR FANTASMAS A PARTIR DAS POSIÇÕES DO MAPA
    printf("\n2. Criando fantasmas a partir do arquivo...\n");
    printf("  Total de fantasmas no arquivo: %d\n", mapa->num_fantasmas);
    
    fantasmas_global = NULL;
    int fantasmas_criados = 0;
    
    for (int i = 0; i < mapa->num_fantasmas; i++) {
        int linha = mapa->fantasma_linhas[i];
        int coluna = mapa->fantasma_colunas[i];
        
        // Verificar se a posição é válida
        if (linha >= 0 && linha < mapa->linhas && 
            coluna >= 0 && coluna < mapa->colunas) {
            
            // Verificar se não é parede
            if (mapa->matriz[linha][coluna] == '#') {
                printf("  AVISO: Fantasma %d em posição inválida (parede): (%d,%d)\n", 
                       i+1, linha, coluna);
                continue;
            }
            
            // Verificar se não é a mesma posição do Pac-Man
            if (linha == pacman_linha && coluna == pacman_coluna) {
                printf("  AVISO: Fantasma %d na mesma posição do Pac-Man: (%d,%d)\n",
                       i+1, linha, coluna);
                // Ainda cria, mas em uma posição adjacente
                int encontrado_pos_alternativa = 0;
                int direcoes[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};
                
                for (int d = 0; d < 4 && !encontrado_pos_alternativa; d++) {
                    int novaLinha = linha + direcoes[d][0];
                    int novaColuna = coluna + direcoes[d][1];
                    
                    if (novaLinha >= 0 && novaLinha < mapa->linhas &&
                        novaColuna >= 0 && novaColuna < mapa->colunas &&
                        mapa->matriz[novaLinha][novaColuna] != '#') {
                        
                        linha = novaLinha;
                        coluna = novaColuna;
                        encontrado_pos_alternativa = 1;
                        printf("    Movido para posição alternativa: (%d,%d)\n", linha, coluna);
                    }
                }
                
                if (!encontrado_pos_alternativa) {
                    printf("    Não foi possível encontrar posição alternativa\n");
                    continue;
                }
            }
            
            // Criar fantasma
            fantasmas_global = adicionar_fantasma(fantasmas_global, linha, coluna);
            fantasmas_criados++;
            printf("  Fantasma %d criado em (%d,%d)\n", i+1, linha, coluna);
        } else {
            printf("  AVISO: Posição do fantasma %d fora dos limites: (%d,%d)\n", 
                   i+1, linha, coluna);
        }
    }
    
    // 3. VERIFICAÇÃO FINAL
    printf("\n3. Verificação final:\n");
    
    // Verificar se o Pac-Man está em posição válida
    if (pacman_global->pos.linha < 0 || pacman_global->pos.linha >= mapa->linhas ||
        pacman_global->pos.coluna < 0 || pacman_global->pos.coluna >= mapa->colunas) {
        printf("  ERRO: Pac-Man em posição inválida!\n");
        finalizarEntidades();
        return 0;
    }
    
    if (mapa->matriz[pacman_global->pos.linha][pacman_global->pos.coluna] == '#') {
        printf("  ERRO: Pac-Man dentro de parede!\n");
        finalizarEntidades();
        return 0;
    }
    
    printf("  Pac-Man: OK em (%d,%d)\n", 
           pacman_global->pos.linha, pacman_global->pos.coluna);
    printf("  Fantasmas: %d criados com sucesso\n", fantasmas_criados);
    
    // Verificar colisão inicial entre Pac-Man e fantasmas
    Fantasma* atual = fantasmas_global;
    int fantasma_idx = 0;
    while (atual) {
        if (atual->pos.linha == pacman_global->pos.linha && 
            atual->pos.coluna == pacman_global->pos.coluna) {
            printf("  AVISO: Fantasma %d na mesma posição inicial do Pac-Man!\n", fantasma_idx+1);
        }
        atual = atual->prox;
        fantasma_idx++;
    }
    
    printf("\n=== ENTIDADES INICIALIZADAS ===\n");
    printf("Pac-Man: 1\n");
    printf("Fantasmas: %d\n", fantasmas_criados);
    printf("================================\n\n");
    
    return 1;
}

// ================== FUNÇÕES DE MOVIMENTO ==================

int mover_pacman_com_colisao(PacMan* pacman, Mapa* mapa, Fantasma* lista_fantasmas, int direcao) {
    // Controle de velocidade
    pacman_frame_counter++;
    if (pacman_frame_counter < pacman_speed_factor) {
        return 1; // Ainda não é hora de mover
    }
    pacman_frame_counter = 0;
    
    if (!pacman || !mapa) return 0;
    
    int linha_original = pacman->pos.linha;
    int coluna_original = pacman->pos.coluna;
    
    // Calcular nova posição
    switch(direcao) {
        case 1: // Cima
            pacman->pos.linha--;
            break;
        case 2: // Baixo
            pacman->pos.linha++;
            break;
        case 3: // Esquerda
            pacman->pos.coluna--;
            break;
        case 4: // Direita
            pacman->pos.coluna++;
            break;
        default:
            return 0; // Direção inválida
    }
    
    // Verificar teleporte horizontal (túneis)
    if (pacman->pos.coluna < 0) {
        pacman->pos.coluna = mapa->colunas - 1;
    } else if (pacman->pos.coluna >= mapa->colunas) {
        pacman->pos.coluna = 0;
    }
    
    pacman->direcao = direcao;
    
    // 1. Primeiro verifica colisão com parede/portal
    if (pacman->pos.linha < 0 || pacman->pos.linha >= mapa->linhas) {
        pacman->pos.linha = linha_original;
        pacman->pos.coluna = coluna_original;
        return 0;
    }
    
    char elemento = mapa->matriz[pacman->pos.linha][pacman->pos.coluna];
    
    if (elemento == '#') { // Parede
        pacman->pos.linha = linha_original;
        pacman->pos.coluna = coluna_original;
        return 0;
    }
    
    if (elemento == 'T') { // Portal
        processar_portal(mapa, &pacman->pos.linha, &pacman->pos.coluna, direcao);
        printf("Portal!\n");
        return 3;
    }
    
    // 2. Verificar colisão com pellets
    if (elemento == '.') {
        remover_pellet(mapa, pacman->pos.linha, pacman->pos.coluna);
        pacman->pontos += 10;
        mapa->pontosRestantes--;
        printf("Pellet! +10 pontos\n");
        return 2;
    }
    
    if (elemento == 'O') {
        remover_power_pellet(mapa, pacman->pos.linha, pacman->pos.coluna);
        pacman->pontos += 50;
        pacman->power_timer = 480;
        mapa->pontosRestantes--;
        
        // Tornar todos os fantasmas vulneráveis
        Fantasma* atual = lista_fantasmas;
        while (atual) {
            atual->vulneravel = 1;
            atual->timer_vulneravel = 480;
            atual = atual->prox;
        }
        
        printf("Power Pellet! Fantasmas vulneráveis!\n");
        return 4;
    }
    
    // 3. VERIFICAÇÃO DE COLISÃO COM FANTASMAS (MELHORADA)
    Fantasma* atual = lista_fantasmas;
    int fantasma_idx = 0;
    
    while (atual) {
        if (atual->pos.linha == pacman->pos.linha && 
            atual->pos.coluna == pacman->pos.coluna) {
            
            if (atual->vulneravel) {
                // Pac-Man come o fantasma
                pacman->pontos += 200;
                
                // Reposiciona fantasma
                atual->pos.linha = 1;
                atual->pos.coluna = 1;
                atual->vulneravel = 0;
                atual->timer_vulneravel = 0;
                
                printf("Fantasma comido! +200 pontos\n");
                return 5;
            } else {
                // Colisão com fantasma normal - PERDE VIDA
                pacman->vidas--;
                
                // Volta para posição original
                pacman->pos.linha = linha_original;
                pacman->pos.coluna = coluna_original;
                
                printf("COLISÃO COM FANTASMA! Vidas restantes: %d\n", pacman->vidas);
                
                if (pacman->vidas <= 0) {
                    return 6; // Game Over
                }
                return 7; // Perdeu vida mas ainda tem vidas
            }
        }
        atual = atual->prox;
        fantasma_idx++;
    }
    
    // Movimento bem-sucedido sem colisão
    return 1;
}

void mover_fantasma_com_colisao(Fantasma* fantasma, Mapa* mapa, Fantasma* lista_fantasmas, PacMan* pacman) {
    if (!fantasma || !mapa) return;
    
    // Controle de velocidade do fantasma (contador individual)
    fantasma->frame_counter++;
    
    // Verificar velocidade do fantasma (fantasma vulnerável é mais lento)
    int speed_factor = fantasma->vulneravel ? 5 : fantasma_speed_factor;
    
    if (fantasma->frame_counter < speed_factor) {
        return; // Ainda não é hora de mover
    }
    fantasma->frame_counter = 0; // Resetar contador individual
    
    int linha_original = fantasma->pos.linha;
    int coluna_original = fantasma->pos.coluna;
    
    // Tenta mover na direção atual
    switch(fantasma->direcao) {
        case DIRECAO_CIMA:     fantasma->pos.linha--; break;
        case DIRECAO_BAIXO:    fantasma->pos.linha++; break;
        case DIRECAO_ESQUERDA: fantasma->pos.coluna--; break;
        case DIRECAO_DIREITA:  fantasma->pos.coluna++; break;
    }
    
    // Verificar teleporte horizontal (túneis)
    if (fantasma->pos.coluna < 0) {
        fantasma->pos.coluna = mapa->colunas - 1;
    } else if (fantasma->pos.coluna >= mapa->colunas) {
        fantasma->pos.coluna = 0;
    }
    
    // Verificar se a nova posição é válida
    if (fantasma->pos.linha < 0 || fantasma->pos.linha >= mapa->linhas) {
        // Fora dos limites verticais - volta
        fantasma->pos = (Posicao){linha_original, coluna_original};
        fantasma->direcao = rand() % 4;
        return;
    }
    
    char elemento = mapa->matriz[fantasma->pos.linha][fantasma->pos.coluna];
    
    // Verificar parede
    if (elemento == '#') {
        // Colisão com parede - volta e escolhe nova direção
        fantasma->pos = (Posicao){linha_original, coluna_original};
        
        // Tenta encontrar direção livre
        int direcoes[4] = {DIRECAO_CIMA, DIRECAO_BAIXO, DIRECAO_ESQUERDA, DIRECAO_DIREITA};
        
        for (int i = 0; i < 4; i++) {
            int nova_dir = direcoes[i];
            
            // Pula a direção oposta à atual (evita oscilação)
            if ((fantasma->direcao == DIRECAO_CIMA && nova_dir == DIRECAO_BAIXO) ||
                (fantasma->direcao == DIRECAO_BAIXO && nova_dir == DIRECAO_CIMA) ||
                (fantasma->direcao == DIRECAO_ESQUERDA && nova_dir == DIRECAO_DIREITA) ||
                (fantasma->direcao == DIRECAO_DIREITA && nova_dir == DIRECAO_ESQUERDA)) {
                continue;
            }
            
            // Testa a direção
            Posicao teste = (Posicao){linha_original, coluna_original};
            switch(nova_dir) {
                case DIRECAO_CIMA:    teste.linha--; break;
                case DIRECAO_BAIXO:   teste.linha++; break;
                case DIRECAO_ESQUERDA: teste.coluna--; break;
                case DIRECAO_DIREITA:  teste.coluna++; break;
            }
            
            // Verifica teleporte horizontal
            if (teste.coluna < 0) teste.coluna = mapa->colunas - 1;
            if (teste.coluna >= mapa->colunas) teste.coluna = 0;
            
            // Verifica se é válida
            if (teste.linha >= 0 && teste.linha < mapa->linhas &&
                teste.coluna >= 0 && teste.coluna < mapa->colunas &&
                mapa->matriz[teste.linha][teste.coluna] != '#') {
                
                fantasma->direcao = nova_dir;
                fantasma->pos = teste;
                return;
            }
        }
        
        // Se não encontrou direção válida, fica parado
        fantasma->direcao = rand() % 4;
    }
    // Verificar portal
    else if (elemento == 'T') {
        processar_portal(mapa, &fantasma->pos.linha, &fantasma->pos.coluna, fantasma->direcao);
    }
    // Movimento bem sucedido
    else {
        // Verifica colisão com outros fantasmas
        Fantasma* atual = lista_fantasmas;
        while (atual) {
            if (atual != fantasma && 
                atual->pos.linha == fantasma->pos.linha && 
                atual->pos.coluna == fantasma->pos.coluna) {
                
                // Colisão com outro fantasma - volta e muda direção
                fantasma->pos = (Posicao){linha_original, coluna_original};
                fantasma->direcao = rand() % 4;
                return;
            }
            atual = atual->prox;
        }
    }
}

// Comente TODA a função mover_fantasma_inteligente existente
// e substitua por esta versão SUPER SIMPLES:

// ================== FUNÇÃO DE MOVIMENTO INTELIGENTE MELHORADA ==================

void mover_fantasma_inteligente(Fantasma* fantasma, Mapa* mapa, PacMan* pacman) {
    if (!fantasma || !mapa || !pacman) return;
    
    // CONTROLE DE VELOCIDADE - MOVIMENTO INDEPENDENTE
    fantasma->frame_counter++;
    
    // Aumente este valor para deixar mais lento (15-20 para movimento lento)
    int speed_factor = fantasma->vulneravel ? 10 : 15;
    
    if (fantasma->frame_counter < speed_factor) {
        return; // Ainda não é hora de mover este fantasma
    }
    fantasma->frame_counter = 0;
    
    // OBTER POSIÇÕES VÁLIDAS ADJACENTES
    int direcoes_validas[4] = {0}; // 0=Cima, 1=Baixo, 2=Esquerda, 3=Direita
    int num_direcoes_validas = 0;
    
    // Verificar todas as 4 direções possíveis
    int offsets[4][2] = {
        {-1, 0}, // Cima
        {1, 0},  // Baixo
        {0, -1}, // Esquerda
        {0, 1}   // Direita
    };
    
    int linha_atual = fantasma->pos.linha;
    int coluna_atual = fantasma->pos.coluna;
    
    // Testar cada direção para ver se é válida
    for (int i = 0; i < 4; i++) {
        int novaLinha = linha_atual + offsets[i][0];
        int novaColuna = coluna_atual + offsets[i][1];
        
        // Verificar teleporte horizontal
        if (novaColuna < 0) novaColuna = mapa->colunas - 1;
        if (novaColuna >= mapa->colunas) novaColuna = 0;
        
        // Verificar se está dentro dos limites verticais
        if (novaLinha >= 0 && novaLinha < mapa->linhas) {
            // Verificar se não é parede
            if (mapa->matriz[novaLinha][novaColuna] != '#') {
                direcoes_validas[num_direcoes_validas] = i;
                num_direcoes_validas++;
            }
        }
    }
    
    // Se não há direções válidas, fica parado (raro caso)
    if (num_direcoes_validas == 0) {
        return;
    }
    
    // EVITAR FICAR OSCILANDO (não voltar pela mesma direção)
    int direcao_oposta = -1;
    switch(fantasma->direcao) {
        case DIRECAO_CIMA: direcao_oposta = DIRECAO_BAIXO; break;
        case DIRECAO_BAIXO: direcao_oposta = DIRECAO_CIMA; break;
        case DIRECAO_ESQUERDA: direcao_oposta = DIRECAO_DIREITA; break;
        case DIRECAO_DIREITA: direcao_oposta = DIRECAO_ESQUERDA; break;
    }
    
    // Se há mais de 1 direção válida, remover a direção oposta (evitar oscilação)
    if (num_direcoes_validas > 1) {
        for (int i = 0; i < num_direcoes_validas; i++) {
            if (direcoes_validas[i] == direcao_oposta) {
                // Remover direção oposta
                for (int j = i; j < num_direcoes_validas - 1; j++) {
                    direcoes_validas[j] = direcoes_validas[j + 1];
                }
                num_direcoes_validas--;
                break;
            }
        }
    }
    
    // ESCOLHER MELHOR DIREÇÃO
    int melhor_direcao = direcoes_validas[0]; // Direção padrão (primeira válida)
    
    // Se há múltiplas opções, escolher a melhor
    if (num_direcoes_validas > 1) {
        // Calcular distância para Pac-Man
        int melhor_distancia = -1;
        
        for (int i = 0; i < num_direcoes_validas; i++) {
            int dir = direcoes_validas[i];
            int novaLinha = linha_atual + offsets[dir][0];
            int novaColuna = coluna_atual + offsets[dir][1];
            
            // Ajustar teleporte
            if (novaColuna < 0) novaColuna = mapa->colunas - 1;
            if (novaColuna >= mapa->colunas) novaColuna = 0;
            
            // Calcular distância Manhattan até Pac-Man
            int dist_linha = abs(novaLinha - pacman->pos.linha);
            int dist_coluna = abs(novaColuna - pacman->pos.coluna);
            
            // Ajustar distância para teleportes horizontais
            if (abs(novaColuna - pacman->pos.coluna) > mapa->colunas / 2) {
                dist_coluna = mapa->colunas - dist_coluna;
            }
            
            int distancia_total = dist_linha + dist_coluna;
            
            // SE VULNERÁVEL: Escolhe direção que MAXIMIZA distância (foge)
            // SE NORMAL: Escolhe direção que MINIMIZA distância (persegue)
            if (fantasma->vulneravel) {
                // FUGIR: Quer maior distância
                if (distancia_total > melhor_distancia || melhor_distancia == -1) {
                    melhor_distancia = distancia_total;
                    melhor_direcao = dir;
                }
            } else {
                // PERSEGUIR: Quer menor distância
                if (distancia_total < melhor_distancia || melhor_distancia == -1) {
                    melhor_distancia = distancia_total;
                    melhor_direcao = dir;
                }
            }
        }
    }
    
    // Aplicar movimento escolhido
    fantasma->direcao = melhor_direcao;
    
    int novaLinha = linha_atual + offsets[melhor_direcao][0];
    int novaColuna = coluna_atual + offsets[melhor_direcao][1];
    
    // Aplicar teleporte horizontal
    if (novaColuna < 0) novaColuna = mapa->colunas - 1;
    if (novaColuna >= mapa->colunas) novaColuna = 0;
    
    // Verificar portal vertical (se aplicável)
    if (novaLinha >= 0 && novaLinha < mapa->linhas) {
        char elemento = mapa->matriz[novaLinha][novaColuna];
        
        if (elemento == 'T') {
            processar_portal(mapa, &novaLinha, &novaColuna, melhor_direcao);
        }
    }
    
    // Atualizar posição
    if (novaLinha >= 0 && novaLinha < mapa->linhas) {
        fantasma->pos.linha = novaLinha;
        fantasma->pos.coluna = novaColuna;
    }
}

void atualizar_estado_fantasmas(Fantasma* lista, PacMan* pacman) {
    if (!lista) return;
    
    Fantasma* atual = lista;
    while (atual) {
        // Atualizar timer de vulnerabilidade
        if (atual->vulneravel && atual->timer_vulneravel > 0) {
            atual->timer_vulneravel--;
            
            if (atual->timer_vulneravel == 0) {
                atual->vulneravel = 0;
                printf("Fantasma voltou ao normal\n");
            }
        }
        
        // Verificar se power pellet acabou
        if (pacman && pacman->power_timer == 0 && atual->vulneravel) {
            atual->vulneravel = 0;
            atual->timer_vulneravel = 0;
        }
        
        atual = atual->prox;
    }
}

// ================== FUNÇÕES DE DESENHO E ESTADO ==================

// ================== FUNÇÕES DE DESENHO E ESTADO ==================
// Adicione esta função em entidades.c, antes da função atualizarEntidades:

void atualizar_estado_pacman(PacMan* pacman, Mapa* mapa) {
    if (!pacman) return;
    
    // Atualizar timer do power pellet
    if (pacman->power_timer > 0) {
        pacman->power_timer--;
        
        // Piscar quando estiver perto do fim
        if (pacman->power_timer < 120) {
            // Aqui você pode adicionar efeitos visuais se quiser
        }
        
        if (pacman->power_timer == 0) {
            printf("Power pellet acabou!\n");
        }
    }
    
    // Outras atualizações do Pac-Man podem ser adicionadas aqui
    (void)mapa; // Evitar warning de parâmetro não usado
}
void atualizarEntidades(Mapa* mapa) {
    if (!pacman_global || !mapa) return;
    
    // Atualizar estados
    atualizar_estado_pacman(pacman_global, mapa);
    atualizar_estado_fantasmas(fantasmas_global, pacman_global);
    
    // ATUALIZADO: Sempre mover fantasmas independentemente do Pac-Man
    // Fantasmas se movem automaticamente a cada frame
    Fantasma* atual = fantasmas_global;
    while (atual) {
        mover_fantasma_inteligente(atual, mapa, pacman_global);
        atual = atual->prox;
    }
}

void desenharEntidades(void) {
    // Esta função é implementada em graficos.c
    // Chamamos as funções específicas
    if (pacman_global) {
        desenhar_pacman(pacman_global);
    }
    
    if (fantasmas_global) {
        desenhar_fantasmas(fantasmas_global);
    }
}

void finalizarEntidades(void) {
    if (pacman_global) {
        free(pacman_global);
        pacman_global = NULL;
        printf("Pac-Man liberado da memoria\n");
    }
    
    if (fantasmas_global) {
        liberar_fantasmas(fantasmas_global);
        fantasmas_global = NULL;
    }
}

void reposicionarEntidades(Mapa* mapa) {
    if (!mapa || !pacman_global) return;
    
    printf("Reposicionando entidades...\n");
    
    // Reposicionar Pac-Man no centro
    pacman_global->pos.linha = mapa->linhas / 2;
    pacman_global->pos.coluna = mapa->colunas / 2;
    
    // Ajustar se for parede
    while (mapa->matriz[pacman_global->pos.linha][pacman_global->pos.coluna] == '#') {
        pacman_global->pos.linha++;
        if (pacman_global->pos.linha >= mapa->linhas) {
            pacman_global->pos.linha = 1;
            pacman_global->pos.coluna++;
        }
    }
    
    // Reposicionar fantasmas em cantos
    Fantasma* atual = fantasmas_global;
    int idx = 0;
    
    while (atual) {
        switch(idx % 4) {
            case 0: // Canto superior esquerdo
                atual->pos.linha = 1;
                atual->pos.coluna = 1;
                break;
            case 1: // Canto superior direito
                atual->pos.linha = 1;
                atual->pos.coluna = mapa->colunas - 2;
                break;
            case 2: // Canto inferior esquerdo
                atual->pos.linha = mapa->linhas - 2;
                atual->pos.coluna = 1;
                break;
            case 3: // Canto inferior direito
                atual->pos.linha = mapa->linhas - 2;
                atual->pos.coluna = mapa->colunas - 2;
                break;
        }
        
        // Resetar estado do fantasma
        atual->vulneravel = 0;
        atual->timer_vulneravel = 0;
        atual->direcao = rand() % 4;
        atual->frame_counter = 0; // Resetar contador individual
        
        atual = atual->prox;
        idx++;
    }
}

void imprimir_estado(PacMan* pacman, Fantasma* lista) {
    if (!pacman) {
        printf("Pac-Man nao existe!\n");
        return;
    }
    
    printf("\n=== ESTADO DO JOGO ===\n");
    printf("PAC-MAN:\n");
    printf("  Posicao: (%d, %d)\n", pacman->pos.linha, pacman->pos.coluna);
    printf("  Vidas: %d\n", pacman->vidas);
    printf("  Pontos: %d\n", pacman->pontos);
    printf("  Power Timer: %d\n", pacman->power_timer);
    printf("  Direcao: %d\n", pacman->direcao);
    
    printf("\nFANTASMAS (%d no total):\n", contar_fantasmas_lista(lista));
    
    Fantasma* atual = lista;
    int idx = 0;
    while (atual) {
        printf("  Fantasma %d:\n", idx);
        printf("    Posicao: (%d, %d)\n", atual->pos.linha, atual->pos.coluna);
        printf("    Estado: %s\n", atual->vulneravel ? "VULNERAVEL" : "NORMAL");
        printf("    Timer Vulneravel: %d\n", atual->timer_vulneravel);
        printf("    Direcao: %d\n", atual->direcao);
        printf("    Frame Counter: %d\n", atual->frame_counter);
        
        atual = atual->prox;
        idx++;
    }
    printf("======================\n\n");
}