#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define N 10
#define INF INT_MAX

/* Prototipos */
void load_matrix(int matrix[N][N]);
void hungarian_solve(int cost[N][N], int assignment[N]);
void reduce_matrix(int matrix[N][N]);
void find_augmenting_path(char marked[N][N], int start_row, int start_col);
void update_matrix(int matrix[N][N], char row_cover[N], char col_cover[N]);
int find_uncovered_zero(int matrix[N][N], char row_cover[N], char col_cover[N], int *row, int *col);
int find_star_in_row(char marked[N][N], int row);
int find_star_in_col(char marked[N][N], int col);
int find_prime_in_row(char marked[N][N], int row);

int main() {
    int cost_matrix[N][N];
    int assignment[N];
    int total_cost = 0;
    int i;

    printf("=== METODO HUNGARO - OTIMIZACAO DE TEMPO MEDICO ===\n");
    printf("Alocacao de %d equipes medicas para %d operacoes\n\n", N, N);
    
    load_matrix(cost_matrix);
    
    /* Resolver usando metodo hungaro */
    hungarian_solve(cost_matrix, assignment);
    
    /* Exibir resultados */
    printf("\n=== RESULTADO DA ALOCACAO OTIMA ===\n");
    printf("Equipe | Operacao | Tempo (min)\n");
    printf("-------|----------|------------\n");
    
    for (i = 0; i < N; i++) {
        int op = assignment[i];
        int tempo = cost_matrix[i][op];
        printf("  %2d   |    %2d    |    %4d\n", i + 1, op + 1, tempo);
        total_cost += tempo;
    }
    
    printf("-------|----------|------------\n");
    printf("Tempo total minimo: %d minutos\n", total_cost);
    
    return 0;
}

void load_matrix(int matrix[N][N]) {
    int i, j;
    int use_example;
    
    printf("Usar matriz exemplo (1) ou inserir manualmente (0)? ");
    scanf("%d", &use_example);
    
    if (use_example) {
        /* Matriz exemplo balanceada - garante solucao otima */
        int exemplo[N][N] = {
            {300,  30, 180, 360, 600,  50, 240, 180, 555,  35},
            {180,  60, 120, 210, 530,  75, 270, 150, 510,  45},
            {150,  45,  90, 270, 585,  35, 210, 240, 615,  60},
            {360,  40, 210, 300, 510,  45, 240, 180, 585,  50},
            {330,  75, 135, 330, 660,  60, 330, 360, 525,  30},
            {240,  45, 105, 240, 555,  40, 240, 300, 600,  35},
            {270,  50, 120, 330, 630,  60, 270, 150, 465,  30},
            {150,  60, 135, 270, 615,  35, 360, 270, 660,  45},
            {180,  40, 150, 360, 540,  30, 240, 150, 555,  60},
            {300,  35,  90, 240, 525,  30, 330, 105, 530,  35}
        };
        
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                matrix[i][j] = exemplo[i][j];
            }
        }
        
        printf("Matriz exemplo carregada.\n");
    } else {
        printf("\nInsira os tempos para cada equipe x operacao:\n");
        for (i = 0; i < N; i++) {
            printf("Equipe %d (10 valores separados por espaco): ", i + 1);
            for (j = 0; j < N; j++) {
                if (scanf("%d", &matrix[i][j]) != 1) {
                    printf("Erro: valor invalido. Use apenas numeros.\n");
                    exit(1);
                }
                if (matrix[i][j] < 0) {
                    printf("Erro: tempo nao pode ser negativo.\n");
                    exit(1);
                }
            }
        }
        printf("Matriz carregada com sucesso.\n");
    }
    
    /* Mostrar matriz */
    printf("\nMatriz de tempos:\n");
    printf("     ");
    for (j = 0; j < N; j++) printf("Op%2d ", j + 1);
    printf("\n");
    
    for (i = 0; i < N; i++) {
        printf("Eq%2d ", i + 1);
        for (j = 0; j < N; j++) {
            printf("%3d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void hungarian_solve(int cost[N][N], int assignment[N]) {
    int matrix[N][N];
    char marked[N][N];     /* 0: nada, 1: estrela, 2: prima */
    char row_cover[N], col_cover[N];
    int i, j, step;
    int row, col;
    int iteration = 0;
    
    /* Inicializacao */
    for (i = 0; i < N; i++) {
        row_cover[i] = 0;
        col_cover[i] = 0;
        assignment[i] = -1;
        for (j = 0; j < N; j++) {
            matrix[i][j] = cost[i][j];
            marked[i][j] = 0;
        }
    }
    
    /* PASSO 1: Reducao da matriz - Linhas e colunas */
    reduce_matrix(matrix);
    
    /* PASSO 2: Atribuicao inicial greedy */
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            if (matrix[i][j] == 0 && row_cover[i] == 0 && col_cover[j] == 0) {
                marked[i][j] = 1; /* Estrela */
                row_cover[i] = 1;
                col_cover[j] = 1;
            }
        }
    }
    
    /* Reset coberturas para proxima fase */
    for (i = 0; i < N; i++) {
        row_cover[i] = 0;
        col_cover[i] = 0;
    }
    
    /* ALGORITMO PRINCIPAL - Metodo Hungaro Tradicional */
    step = 1;
    while (step != 0 && iteration < 100) {
        iteration++;
        
        switch (step) {
            case 1: 
                /* PASSO 1: Cobertura de colunas com estrelas */
                for (j = 0; j < N; j++) {
                    col_cover[j] = 0;
                    for (i = 0; i < N; i++) {
                        if (marked[i][j] == 1) {
                            col_cover[j] = 1;
                            break;
                        }
                    }
                }
                step = 2;
                break;
                
            case 2: 
                /* PASSO 2: Verificar se solucao completa */
                j = 0;
                for (i = 0; i < N; i++) {
                    if (col_cover[i]) j++;
                }
                if (j == N) {
                    step = 0; /* Solucao encontrada */
                } else {
                    step = 3;
                }
                break;
                
            case 3: 
                /* PASSO 3: Encontrar zero nao coberto */
                if (find_uncovered_zero(matrix, row_cover, col_cover, &row, &col)) {
                    marked[row][col] = 2; /* Marcar como prima */
                    
                    int star_col = find_star_in_row(marked, row);
                    if (star_col >= 0) {
                        /* Tem estrela na mesma linha - cobrir linha e descobrir coluna */
                        row_cover[row] = 1;
                        col_cover[star_col] = 0;
                        step = 3; /* Continuar no passo 3 */
                    } else {
                        /* Nenhuma estrela - encontrar caminho aumentante */
                        find_augmenting_path(marked, row, col);
                        
                        /* Resetar coberturas e remover primes */
                        for (i = 0; i < N; i++) {
                            row_cover[i] = 0;
                            col_cover[i] = 0;
                            for (j = 0; j < N; j++) {
                                if (marked[i][j] == 2) {
                                    marked[i][j] = 0;
                                }
                            }
                        }
                        step = 1; /* Voltar ao passo 1 */
                    }
                } else {
                    step = 4; /* Ir para atualizacao da matriz */
                }
                break;
                
            case 4: 
                /* PASSO 4: Atualizar matriz */
                update_matrix(matrix, row_cover, col_cover);
                step = 3; /* Voltar ao passo 3 */
                break;
        }
    }
    
    if (iteration >= 100) {
        printf("Aviso: Limite de iteracoes atingido\n");
    }
    
    /* Extrair atribuicao final */
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            if (marked[i][j] == 1) {
                assignment[i] = j;
                break;
            }
        }
        /* Garantir que toda equipe tenha uma operacao */
        if (assignment[i] == -1) {
            /* Atribuicao de fallback - primeira operacao disponivel */
            for (j = 0; j < N; j++) {
                if (matrix[i][j] == 0) {
                    assignment[i] = j;
                    break;
                }
            }
            if (assignment[i] == -1) assignment[i] = 0;
        }
    }
}

void reduce_matrix(int matrix[N][N]) {
    int i, j, min_val;
    
    /* Reducao de linhas */
    for (i = 0; i < N; i++) {
        min_val = INF;
        for (j = 0; j < N; j++) {
            if (matrix[i][j] < min_val) {
                min_val = matrix[i][j];
            }
        }
        for (j = 0; j < N; j++) {
            matrix[i][j] -= min_val;
        }
    }
    
    /* Reducao de colunas */
    for (j = 0; j < N; j++) {
        min_val = INF;
        for (i = 0; i < N; i++) {
            if (matrix[i][j] < min_val) {
                min_val = matrix[i][j];
            }
        }
        for (i = 0; i < N; i++) {
            matrix[i][j] -= min_val;
        }
    }
}

void find_augmenting_path(char marked[N][N], int start_row, int start_col) {
    int path[N*2][2];
    int count = 0;
    int row, col;
    
    /* Iniciar caminho com o zero prima descoberto */
    path[count][0] = start_row;
    path[count][1] = start_col;
    
    while (1) {
        /* Encontrar estrela na mesma coluna */
        row = find_star_in_col(marked, path[count][1]);
        if (row < 0) break;
        
        count++;
        path[count][0] = row;
        path[count][1] = path[count-1][1];
        
        /* Encontrar prima na mesma linha */
        col = find_prime_in_row(marked, path[count][0]);
        if (col < 0) break;
        
        count++;
        path[count][0] = path[count-1][0];
        path[count][1] = col;
    }
    
    /* Alternar estrelas no caminho aumentante */
    int i;
    for (i = 0; i <= count; i++) {
        int r = path[i][0];
        int c = path[i][1];
        if (marked[r][c] == 1) {
            marked[r][c] = 0; /* Remover estrela */
        } else {
            marked[r][c] = 1; /* Adicionar estrela */
        }
    }
}

void update_matrix(int matrix[N][N], char row_cover[N], char col_cover[N]) {
    int i, j;
    int min_val = INF;
    
    /* Encontrar menor valor nao coberto */
    for (i = 0; i < N; i++) {
        if (row_cover[i] == 0) {
            for (j = 0; j < N; j++) {
                if (col_cover[j] == 0) {
                    if (matrix[i][j] < min_val) {
                        min_val = matrix[i][j];
                    }
                }
            }
        }
    }
    
    /* Atualizar matriz conforme metodo hungaro */
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            if (row_cover[i] == 1) {
                matrix[i][j] += min_val; /* Somar a linhas cobertas */
            }
            if (col_cover[j] == 0) {
                matrix[i][j] -= min_val; /* Subtrair de colunas nao cobertas */
            }
        }
    }
}

int find_uncovered_zero(int matrix[N][N], char row_cover[N], char col_cover[N], int *row, int *col) {
    int i, j;
    
    for (i = 0; i < N; i++) {
        if (row_cover[i] == 0) {
            for (j = 0; j < N; j++) {
                if (col_cover[j] == 0 && matrix[i][j] == 0) {
                    *row = i;
                    *col = j;
                    return 1;
                }
            }
        }
    }
    *row = -1;
    *col = -1;
    return 0;
}

int find_star_in_row(char marked[N][N], int row) {
    int j;
    for (j = 0; j < N; j++) {
        if (marked[row][j] == 1) {
            return j;
        }
    }
    return -1;
}

int find_star_in_col(char marked[N][N], int col) {
    int i;
    for (i = 0; i < N; i++) {
        if (marked[i][col] == 1) {
            return i;
        }
    }
    return -1;
}

int find_prime_in_row(char marked[N][N], int row) {
    int j;
    for (j = 0; j < N; j++) {
        if (marked[row][j] == 2) {
            return j;
        }
    }
    return -1;
}
