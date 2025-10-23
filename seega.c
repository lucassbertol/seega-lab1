#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef _WIN32
#define CLEAR "cls"
#else 
#define CLEAR "clear"
#endif
#define SIZE 5
#define AMARELO "\033[33m"
#define RESET "\033[0m"
#define NEGRITO "\033[1m"
#define AZUL "\033[34m"      
#define VERMELHO "\033[31m"  
#define VERDE "\x1b[32m"
#define MAX_PARTIDAS_RECENTES 5
#define FASE_COLOCACAO 0
#define FASE_MOVIMENTACAO 1

void limpaTela() {
system(CLEAR); 
}

time_t tempoInicial;
time_t tempoPausaInicio;
double tempoPausado = 0.0;
int timerAtivo = 0;
int timerPausado = 0;

void iniciarTimer() {
    time(&tempoInicial);
    tempoPausado = 0.0;
    timerAtivo = 1;
    timerPausado = 0;
}
void pausarTimer() {
    if (timerAtivo && !timerPausado) {
        time(&tempoPausaInicio);
        timerPausado = 1;
    }
}
void despausarTimer() {
    if (timerAtivo && timerPausado) {
        time_t tempoPausaFim;
        time(&tempoPausaFim);
        tempoPausado += difftime(tempoPausaFim, tempoPausaInicio);
        timerPausado = 0;
    }
}
void finalizarTimer() {
    if (timerAtivo) {
        time_t tempoFinal;
        time(&tempoFinal);
        double tempoDecorrido = difftime(tempoFinal, tempoInicial) - tempoPausado;
        int minutos = (int)(tempoDecorrido / 60);
        int segundos = (int)(tempoDecorrido) % 60;
        timerAtivo = 0;
    }
}
void pausarJogo() {
    limpaTela();
    printf(VERDE NEGRITO "\n*** Jogo Pausado ***\n" RESET);
    printf("Pressione ENTER para continuar...\n");
    pausarTimer();
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    getchar(); 
    despausarTimer();
}

typedef struct {
    int total_partidas;
    int vitorias_player1;
    int vitorias_player2;
    int empates;
    double menor_tempo;
    double maior_tempo;
} ModoDados;
typedef struct {
    int modo;           
    int resultado;      
    char vencedor;      
} Partida;
typedef struct {
    ModoDados pvp;               
    ModoDados pvc;               
    Partida ultimas_partidas[MAX_PARTIDAS_RECENTES];
    int index_partida;           
} Historico;

void salvarResultado(int modo, int resultado, double tempo) {
    FILE *arquivo;
    Historico hist;
    arquivo = fopen("historico.txt", "r");
    if (arquivo) {
        fscanf(arquivo, 
               "PvP\nTOTAL PARTIDAS: %d\nVITORIAS PLAYER 1 (O): %d\nVITORIAS PLAYER 2 (X): %d\nEMPATES: %d\nMENOR TEMPO: %lf\nMAIOR TEMPO: %lf\n"
               "PvPC\nTOTAL PARTIDAS: %d\nVITORIAS PLAYER: %d\nVITORIAS COMPUTADOR: %d\nEMPATES: %d\nMENOR TEMPO: %lf\nMAIOR TEMPO: %lf\n"
               "ULTIMAS_PARTIDAS\n",
               &hist.pvp.total_partidas,
               &hist.pvp.vitorias_player1,
               &hist.pvp.vitorias_player2,
               &hist.pvp.empates,
               &hist.pvp.menor_tempo,
               &hist.pvp.maior_tempo,
               &hist.pvc.total_partidas,
               &hist.pvc.vitorias_player1,
               &hist.pvc.vitorias_player2,
               &hist.pvc.empates,
               &hist.pvc.menor_tempo,
               &hist.pvc.maior_tempo);
        for(int i = 0; i < MAX_PARTIDAS_RECENTES; i++) {
            fscanf(arquivo, "MODO: %d RESULTADO: %d VENCEDOR: %c\n",
                   &hist.ultimas_partidas[i].modo,
                   &hist.ultimas_partidas[i].resultado,
                   &hist.ultimas_partidas[i].vencedor);
        }
        fscanf(arquivo, "INDEX_PARTIDA: %d\n", &hist.index_partida);
        fclose(arquivo);
    } else {
        hist.pvp = (ModoDados){0, 0, 0, 0, 999999.0, 0.0};
        hist.pvc = (ModoDados){0, 0, 0, 0, 999999.0, 0.0};
        for(int i = 0; i < MAX_PARTIDAS_RECENTES; i++) {
            hist.ultimas_partidas[i].modo = 0;
            hist.ultimas_partidas[i].resultado = 0;
            hist.ultimas_partidas[i].vencedor = 'N'; 
        }
        hist.index_partida = 0;
    }
    ModoDados *dados = (modo == 1) ? &hist.pvp : &hist.pvc;
    dados->total_partidas++;
    switch(resultado) {
        case 1: dados->vitorias_player1++; break;
        case 2: dados->vitorias_player2++; break;
        case 0: dados->empates++; break;
    }
    if (tempo < dados->menor_tempo || dados->menor_tempo == 999999.0) {
        dados->menor_tempo = tempo;
    }
    if (tempo > dados->maior_tempo) {
        dados->maior_tempo = tempo;
    }
    hist.ultimas_partidas[hist.index_partida].modo = modo;
    hist.ultimas_partidas[hist.index_partida].resultado = resultado;
    if(resultado == 1)
        hist.ultimas_partidas[hist.index_partida].vencedor = 'O';
    else if(resultado == 2)
        hist.ultimas_partidas[hist.index_partida].vencedor = 'X';
    else
        hist.ultimas_partidas[hist.index_partida].vencedor = 'N';
    hist.index_partida = (hist.index_partida + 1) % MAX_PARTIDAS_RECENTES;
    arquivo = fopen("historico.txt", "w");
    if (arquivo) {
        fprintf(arquivo, "PvP\nTOTAL PARTIDAS: %d\nVITORIAS PLAYER 1 (O): %d\nVITORIAS PLAYER 2 (X): %d\nEMPATES: %d\nMENOR TEMPO: %.2lf\nMAIOR TEMPO: %.2lf\n",
                hist.pvp.total_partidas,
                hist.pvp.vitorias_player1,
                hist.pvp.vitorias_player2,
                hist.pvp.empates,
                hist.pvp.menor_tempo,
                hist.pvp.maior_tempo);
        fprintf(arquivo, "PvPC\nTOTAL PARTIDAS: %d\nVITORIAS PLAYER: %d\nVITORIAS COMPUTADOR: %d\nEMPATES: %d\nMENOR TEMPO: %.2lf\nMAIOR TEMPO: %.2lf\n",
                hist.pvc.total_partidas,
                hist.pvc.vitorias_player1,
                hist.pvc.vitorias_player2,
                hist.pvc.empates,
                hist.pvc.menor_tempo,
                hist.pvc.maior_tempo);
        fprintf(arquivo, "ULTIMAS_PARTIDAS\n");
        for(int i = 0; i < MAX_PARTIDAS_RECENTES; i++) {
            fprintf(arquivo, "MODO: %d RESULTADO: %d VENCEDOR: %c\n",
                    hist.ultimas_partidas[i].modo,
                    hist.ultimas_partidas[i].resultado,
                    hist.ultimas_partidas[i].vencedor);
        }
        fprintf(arquivo, "INDEX_PARTIDA: %d\n", hist.index_partida);
        fclose(arquivo);
    }
}

void visaoGeral() {
    printf(NEGRITO AMARELO "\nVisao Geral do Jogo\n\n" RESET);
    printf("O Seega e um jogo de tabuleiro estrategico jogado num tabuleiro de 5x5.\n");
    printf("Cada jogador tem 12 pecas e o objetivo e capturar todas as pecas do adversario.\n");
    printf("O jogo ocorre em duas fases: colocacao e movimento/captura.\n\n");
}

void preparacaoInicial() {
    printf(NEGRITO AMARELO "\nPreparacao Inicial\n\n" RESET);
    printf("Cada jogador escolhe uma cor de peca e recebe 12 pecas.\n");
    printf("O tabuleiro de 5x5 e colocado entre os jogadores, com o espaco central vazio.\n\n");
}

void faseColocacao() {
    printf(NEGRITO AMARELO "\nFase 1: Colocar as Pecas\n\n" RESET);
    printf("Os jogadores alternam turnos colocando duas pecas de sua cor no tabuleiro.\n");
    printf("O espaco central deve permanecer vazio. Nao ha capturas nesta fase.\n\n");
}

void faseMovimentoCaptura() {
    printf(NEGRITO AMARELO "\nFase 2: Movimento e Captura\n\n" RESET);
    printf("Cada jogador move uma de suas pecas para um espaco adjacente vazio.\n");
    printf("Pecas sao capturadas quando ficam flanqueadas por duas pecas adversarias em linha reta.\n");
    printf("O jogador que captura uma peca ganha direito a outro movimento, se for possivel realizar outra captura\n");
    printf("com a ultima peca movida.\n\n");
}

void fimDeJogo() {
    printf(NEGRITO AMARELO "\nComo o Jogo Termina\n\n" RESET);
    printf("O jogo termina quando ha uma vitoria total, que acontece quando um jogador captura todas as pecas do adversario,\n");
    printf("ou quando ha uma pequena vitoria, que acontece quando algum jogador cria uma barreira de 5 pecas\n");
    printf("isolando o adversario em um dos lados, nao importando o numero de capturas realizadas.\n");
    printf("O jogo tambem pode terminar em empate, quando ambos os jogadores tem 3 pecas ou menos\n");
    printf("e nao conseguem realizar mais capturas.\n\n");
}

void resumoCaptura() {
    printf(NEGRITO AMARELO "\nResumo das Regras de Captura\n\n" RESET);
    printf("Capturas sao feitas ao flanquear uma peca adversaria entre duas pecas suas na horizontal ou vertical.\n");
    printf("A peca no centro do tabuleiro nao pode ser capturada.\n");
    printf("Capturas simultaneas ocorrem quando a movimentacao de uma peca permite capturar duas ou tres\n");
    printf("pecas do adversario. As capturas tambem pode ser consecutivas. Apos fazer uma captura,\n"); 
    printf("o jogador pode - na mesma jogada e com a mesma peca - continuar capturando ate nao haver mais possibilidades\n\n");
}

void menuAjuda(){
    int xajuda;
    int yajuda;
printf(NEGRITO AMARELO "\n=== MENU DE AJUDA ===\n\n" RESET);
printf("1. Visao geral\n\n");
printf("2. Preparacao inicial\n\n");
printf("3. Fase de colocar pecas\n\n");
printf("4. Fase de movimento e captura\n\n");
printf("5. Resumo captura\n\n");
printf("6. Fim de jogo\n\n");
printf("0. Voltar ao main menu\n\n");
printf(NEGRITO "Escolha uma opcao de ajuda: " RESET);
scanf("%d", &xajuda);
limpaTela();
    switch(xajuda){
        case 1:
            visaoGeral();
            printf(NEGRITO "Digite 1 para voltar ou 0 para ir para o main menu: " RESET);
            scanf("%d", &yajuda);
            limpaTela();
            if (yajuda == 1) {
                menuAjuda();
            }
            else if (yajuda == 0) {
                menu();
            }
            else {
                printf("Opcao invalida, retornando ao main menu...");
                sleep(2);
                menu();
            }
            break;
        case 2:
            preparacaoInicial();
            printf(NEGRITO "Digite 1 para voltar ou 0 para ir para o main menu: " RESET);
            scanf("%d", &yajuda);
            limpaTela();
            if (yajuda == 1) {
                menuAjuda();
            }
            else if (yajuda == 0) {
                menu();
            }
            else {
                printf("Opcao invalida, retornando ao main menu...");
                sleep(2);
                menu();
            }
            break;
        case 3:
            faseColocacao();
            printf(NEGRITO "Digite 1 para voltar ou 0 para ir para o main menu: " RESET);
            scanf("%d", &yajuda);
            limpaTela();
            if (yajuda == 1) {
                menuAjuda();
            }
            else if (yajuda == 0) {
                menu();
            }
            else {
                printf("Opcao invalida, retornando ao main menu...");
                sleep(2);
                menu();
            }
            break;
        case 4:
            faseMovimentoCaptura();
            printf(NEGRITO "Digite 1 para voltar ou 0 para ir para o main menu: " RESET);
            scanf("%d", &yajuda);
            limpaTela();
            if (yajuda == 1) {
                menuAjuda();
            }
            else if (yajuda == 0) {
                menu();
            }
            else {
                printf("Opcao invalida, retornando ao main menu...");
                sleep(2);
                menu();
            }
            break;
        case 5:
            resumoCaptura();
            printf(NEGRITO "Digite 1 para voltar ou 0 para ir para o main menu: " RESET);
            scanf("%d", &yajuda);
            limpaTela();
            if (yajuda == 1) {
                menuAjuda();
            }
            else if (yajuda == 0) {
                menu();
            }
            else {
                printf("Opcao invalida, retornando ao main menu...");
                sleep(2);
                menu();
            }
            break;
        case 6:
            fimDeJogo();
            printf(NEGRITO "Digite 1 para voltar ou 0 para ir para o main menu: " RESET);
            scanf("%d", &yajuda);
            limpaTela();
            if (yajuda == 1) {
                menuAjuda();
            }
            else if (yajuda == 0) {
                menu();
            }
            else {
                printf("Opcao invalida, retornando ao main menu...");
                sleep(2);
                menu();
            }
            break;
        case 0:
            limpaTela();
            menu();
            break;
    default:
        limpaTela();
        menuAjuda();
        break;
    }
}

void menu() {
    bool carregarJogoTxt(void);
    bool carregarJogoPvPC(void);
    int opcao;
    printf(NEGRITO AMARELO "\n=== SEEGA ===\n\n" RESET);
    printf(NEGRITO "Menu: \n\n" RESET);
    printf("1. Jogar\n\n");
    printf("2. Carregar partidas salvas\n\n");
    printf("3. Historico\n\n");
    printf("4. Ajuda\n\n");
    printf("5. Sair\n\n");
    printf(NEGRITO "Selecione uma opcao: " RESET);
    scanf("%d", &opcao);
    limpaTela();
    switch (opcao) {
        case 1:
        printf(NEGRITO AMARELO "=== Modo de jogo ===\n\n" RESET);
        printf("1. Player x Player\n\n");
        printf("2. Player x Computador\n\n");
        printf("0. Voltar ao menu\n\n");
        printf(NEGRITO "Esolha o modo de jogo que deseja jogar: " RESET);
        int modo1;
        scanf("%d", &modo1);
        switch (modo1) {
            case 1:
                limpaTela();
                iniciarJogo();
                break;
            case 2:
                limpaTela();
                iniciarJogoPc();
                break;
            case 0:
                limpaTela();
                menu();
                break;
            default:
                printf("\nOpcao invalida. Retornando ao menu...\n");
                Sleep(2000); 
                limpaTela();
                menu();
                break;
        }
        case 2:
            printf(NEGRITO AMARELO "=== Carregar partidas salvas ===\n\n" RESET);
            printf("1. Player x Player\n\n");
            printf("2. Player x Computador\n\n");
            printf("0. Voltar ao menu\n\n");
            printf(NEGRITO "Esolha o modo de jogo que deseja carregar: " RESET);
            int modo;
            scanf("%d", &modo);
            switch (modo) {
                case 1:
                    if (carregarJogoTxt()) {
                        iniciarJogo();
                    }
                    break;
                case 2:
                    if (carregarJogoPvPC()) {
                        iniciarJogoPc();
                    }
                    break;
                case 0:
                    limpaTela();
                    menu();
                    break;
                default:
                    printf("\nOpcao invalida. Retornando ao menu...\n");
                    Sleep(2000);
                    limpaTela();
                    menu();
                    break;
            }
        case 3:
            limpaTela();
            mostrarHistorico();
            break;
        case 4: 
            limpaTela();
            menuAjuda();
            break;
        case 5: 
            limpaTela();
            break;
        default:
            menu();
            break;
    }
}

void mostrarHistorico() {
    FILE *arquivo;
    Historico hist;
    arquivo = fopen("historico.txt", "r");
    if (!arquivo) {
        printf(NEGRITO VERMELHO "Sem partidas anteriores registradas!\n" RESET);
        printf("Jogue algumas partidas para visualizar o historico.\n\n");
        Sleep(2000);
        limpaTela();
        menu();
        return;
    }
    fscanf(arquivo, 
           "PvP\nTOTAL PARTIDAS: %d\nVITORIAS PLAYER 1 (O): %d\nVITORIAS PLAYER 2 (X): %d\nEMPATES: %d\nMENOR TEMPO: %lf\nMAIOR TEMPO: %lf\n"
           "PvPC\nTOTAL PARTIDAS: %d\nVITORIAS PLAYER: %d\nVITORIAS COMPUTADOR: %d\nEMPATES: %d\nMENOR TEMPO: %lf\nMAIOR TEMPO: %lf\n"
           "ULTIMAS_PARTIDAS\n",
           &hist.pvp.total_partidas,
           &hist.pvp.vitorias_player1,
           &hist.pvp.vitorias_player2,
           &hist.pvp.empates,
           &hist.pvp.menor_tempo,
           &hist.pvp.maior_tempo,
           &hist.pvc.total_partidas,
           &hist.pvc.vitorias_player1,
           &hist.pvc.vitorias_player2,
           &hist.pvc.empates,
           &hist.pvc.menor_tempo,
           &hist.pvc.maior_tempo);
    for(int i = 0; i < MAX_PARTIDAS_RECENTES; i++) {
        fscanf(arquivo, "MODO: %d RESULTADO: %d VENCEDOR: %c\n",
               &hist.ultimas_partidas[i].modo,
               &hist.ultimas_partidas[i].resultado,
               &hist.ultimas_partidas[i].vencedor);
    }
    fscanf(arquivo, "INDEX_PARTIDA: %d\n", &hist.index_partida);
    fclose(arquivo);
    printf(NEGRITO AMARELO "\n=== HISTORICO DE PARTIDAS ===\n\n" RESET);
    printf(NEGRITO "== PLAYER VS PLAYER ==\n" RESET);
    printf("TOTAL PARTIDAS: %d\n", hist.pvp.total_partidas);
    printf(AZUL "VITORIAS PLAYER 1 (O): %d\n" RESET, hist.pvp.vitorias_player1);
    printf(VERMELHO "VITORIAS PLAYER 2 (X): %d\n" RESET, hist.pvp.vitorias_player2);
    printf("EMPATES: %d\n", hist.pvp.empates);
    if (hist.pvp.total_partidas > 0) {
        int minutos_pvp_menor = (int)(hist.pvp.menor_tempo / 60);
        double segundos_pvp_menor = hist.pvp.menor_tempo - (minutos_pvp_menor * 60.0);
        printf("MENOR TEMPO: %d minutos e %.2lf segundos\n", minutos_pvp_menor, segundos_pvp_menor);
        int minutos_pvp_maior = (int)(hist.pvp.maior_tempo / 60);
        double segundos_pvp_maior = hist.pvp.maior_tempo - (minutos_pvp_maior * 60.0);
        printf("MAIOR TEMPO: %d minutos e %.2lf segundos\n", minutos_pvp_maior, segundos_pvp_maior);
    }
    printf(NEGRITO "\n--------------------------------\n" RESET);
    printf(NEGRITO "\n== PLAYER VS COMPUTADOR ==\n" RESET);
    printf("TOTAL PARTIDAS: %d\n", hist.pvc.total_partidas);
    printf(AZUL "VITORIAS PLAYER: %d\n" RESET, hist.pvc.vitorias_player1);
    printf(VERMELHO "VITORIAS COMPUTADOR: %d\n" RESET, hist.pvc.vitorias_player2);
    printf("EMPATES: %d\n", hist.pvc.empates);
    if (hist.pvc.total_partidas > 0) {
        int minutos_pvc_menor = (int)(hist.pvc.menor_tempo / 60);
        double segundos_pvc_menor = hist.pvc.menor_tempo - (minutos_pvc_menor * 60.0);
        printf("MENOR TEMPO: %d minutos e %.2lf segundos\n", minutos_pvc_menor, segundos_pvc_menor);
        int minutos_pvc_maior = (int)(hist.pvc.maior_tempo / 60);
        double segundos_pvc_maior = hist.pvc.maior_tempo - (minutos_pvc_maior * 60.0);
        printf("MAIOR TEMPO: %d minutos e %.2lf segundos\n", minutos_pvc_maior, segundos_pvc_maior);
    }
    printf(NEGRITO "\n--------------------------------\n" RESET);
    printf(NEGRITO AMARELO "\n=== ULTIMAS 5 PARTIDAS ===\n\n" RESET);
    for(int i = 0; i < MAX_PARTIDAS_RECENTES; i++) {
        int pos = (hist.index_partida + i) % MAX_PARTIDAS_RECENTES;
        Partida p = hist.ultimas_partidas[pos];
        if(p.modo == 0 && p.resultado == 0 && p.vencedor == 'N') {
            continue;
        }
        printf("Modo: %s\n", (p.modo == 1) ? "Player vs Player" : "Player vs Computador");
        if(p.resultado == 1)
            printf("Resultado:" AZUL " Vitoria Player 1 (O)\n" RESET);
        else if(p.resultado == 2)
            printf("Resultado:" VERMELHO " Vitoria Player 2 (X)\n" RESET);
        else
            printf("Resultado: Empate\n");
        printf("\n");
    }
    printf(VERDE NEGRITO "\nAperte ENTER para voltar ao menu...\n" RESET);
    getchar(); 
    getchar(); 
    limpaTela();
    menu();
}

void registrarVitoria(int modo, int vencedor) {
    time_t tempoFinal;
    time(&tempoFinal);
    double tempoTotal = difftime(tempoFinal, tempoInicial) - tempoPausado;
    salvarResultado(modo, vencedor, tempoTotal);
}

void inicializarTabuleiro(char tabuleiro[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (i == 2 && j == 2) 
                tabuleiro[i][j] = '#'; 
            else
                tabuleiro[i][j] = '+'; 
        }
    }
}

void exibirTabuleiro(char tabuleiro[SIZE][SIZE]) {
    printf(NEGRITO AMARELO "   1 2 3 4 5\n" RESET);
    for (int i = 0; i < SIZE; i++) {
        printf(NEGRITO AMARELO "%c " RESET, 'A' + i);
        for (int j = 0; j < SIZE; j++) {
            if (tabuleiro[i][j] == 'O') {
                printf(AZUL " O" RESET); 
            } else if (tabuleiro[i][j] == 'X') {
                printf(VERMELHO " X" RESET); 
            } else {
                printf(" %c", tabuleiro[i][j]); 
            }
        }
        printf("\n");
    }
}

int tabuleiroCheio(char tabuleiro[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (tabuleiro[i][j] == '+') {
                return 0; 
            }
        }
    }
    return 1; 
}

bool salvarJogoTxt(char tabuleiro[SIZE][SIZE], char turnoAtual) {
    FILE *arquivo = fopen("jogo_save.txt", "w");
    if (arquivo == NULL) {
        limpaTela();
        menu();
        return false;
    }
    fprintf(arquivo, "TABULEIRO\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fprintf(arquivo, "%c", tabuleiro[i][j]);
        }
        fprintf(arquivo, "\n");
    }
    fprintf(arquivo, "TEMPO\n%ld\n", time(NULL) - tempoInicial);
    fprintf(arquivo, "TURNO\n%c\n", turnoAtual);
    fclose(arquivo);
    return true;
    exit(0);
}

bool salvarJogoPvPC(char tabuleiro[SIZE][SIZE]) {
    FILE *arquivo = fopen("jogo_save_pvpc.txt", "w");
    if (arquivo == NULL) {
        limpaTela();
        menu();
        return false;
    }
    fprintf(arquivo, "TABULEIRO\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fprintf(arquivo, "%c", tabuleiro[i][j]);
        }
        fprintf(arquivo, "\n");
    }
    fprintf(arquivo, "TEMPO\n%ld\n", time(NULL) - tempoInicial);
    fclose(arquivo);
    return true;
}

int posicaoParaIndices(char linha, int coluna, int *x, int *y) {
    linha = toupper(linha);
    *x = linha - 'A';
    *y = coluna - 1; 
    if (*x < 0 || *x >= SIZE || *y < 0 || *y >= SIZE) {
        return 0;
    }
    return 1; 
}

int posicaoDisponivel(char tabuleiro[SIZE][SIZE], int x, int y) {
    return tabuleiro[x][y] == '+';
}

int verificarMovimento(char tabuleiro[SIZE][SIZE], int i, int j, char jogadorAtual) {
    char adversario = (jogadorAtual == 'X') ? 'O' : 'X';
    if (j - 1 >= 0 && j + 1 < SIZE && tabuleiro[i][j - 1] == adversario && tabuleiro[i][j + 1] == adversario) {
        return 0; 
    }
    if (i - 1 >= 0 && i + 1 < SIZE && tabuleiro[i - 1][j] == adversario && tabuleiro[i + 1][j] == adversario) {
        return 0; 
    }
    return 1; 
}

int verificarMovimentosLivres(char tabuleiro[SIZE][SIZE], char jogador) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (tabuleiro[i][j] == jogador) {
                if ((i > 0 && tabuleiro[i - 1][j] == '+') ||  
                    (i < SIZE - 1 && tabuleiro[i + 1][j] == '+') ||  
                    (j > 0 && tabuleiro[i][j - 1] == '+') ||  
                    (j < SIZE - 1 && tabuleiro[i][j + 1] == '+')) {  
                    return 1;  
                }
            }
        }
    }
    return 0;  
}

int verificarCaptura(char tabuleiro[SIZE][SIZE], char jogadorAtual, int x, int y) {
    char adversario = (jogadorAtual == 'X') ? 'O' : 'X';
    int capturaRealizada = 0;
    int direcoes[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
    for (int d = 0; d < 4; d++) {
        int adjX = x + direcoes[d][0];
        int adjY = y + direcoes[d][1];
        if (adjX < 0 || adjX >= SIZE || adjY < 0 || adjY >= SIZE)
            continue;
        if (adjX == SIZE / 2 && adjY == SIZE / 2 && tabuleiro[adjX][adjY] == adversario)
            continue;
        if (tabuleiro[adjX][adjY] == adversario) {
            int fimX = adjX + direcoes[d][0];
            int fimY = adjY + direcoes[d][1];
            if (fimX < 0 || fimX >= SIZE || fimY < 0 || fimY >= SIZE)
                continue;
            if (tabuleiro[fimX][fimY] == jogadorAtual) {
                tabuleiro[adjX][adjY] = '+';
                capturaRealizada = 1;
            }
        }
    }
    return capturaRealizada;
}

int checarVitoriaTotalPvP(char tabuleiro[SIZE][SIZE]) {
    int pecasO = 0;
    int pecasX = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (tabuleiro[i][j] == 'O') {
                pecasO++;
            } else if (tabuleiro[i][j] == 'X') {
                pecasX++;
            }
        }
    }
    if (pecasO == 0) {
    printf(VERMELHO NEGRITO "\nJogador 2 (X) venceu com VITORIA TOTAL!\nTodas as pecas do Jogador 1 (O) foram capturadas.\n" RESET);
    finalizarTimer();
    registrarVitoria(1, 2);
    int opcao;
    do {
        printf(VERDE NEGRITO "\nDigite 1 para jogar novamente ou 0 para voltar ao menu principal: " RESET);
        scanf("%d", &opcao);
        if(opcao != 0 && opcao != 1) {
            printf(VERMELHO "Opção inválida! Tente novamente.\n" RESET);
        }
    } while(opcao != 0 && opcao != 1);
    limpaTela();
    if(opcao == 1) {
        iniciarJogo();
    } else {
        menu();
    }
    return 1;
} else if (pecasX == 0) {
    printf(AZUL NEGRITO "\nJogador 1 (O) venceu com VITORIA TOTAL!\nTodas as pecas do Jogador 2 (X) foram capturadas.\n" RESET);
    finalizarTimer();
    registrarVitoria(1, 1);
    int opcao;
    do {
        printf(VERDE NEGRITO "\nDigite 1 para jogar novamente ou 0 para voltar ao menu principal: " RESET);
        scanf("%d", &opcao);
        if(opcao != 0 && opcao != 1) {
            printf(VERMELHO "Opção inválida! Tente novamente.\n" RESET);
        }
    } while(opcao != 0 && opcao != 1);
    limpaTela();
    if(opcao == 1) {
        iniciarJogo();
    } else {
        menu();
    }
    return 1;
}
return 0;
}

int checarVitoriaTotalPvPC(char tabuleiro[SIZE][SIZE]) {
    int pecasO = 0;
    int pecasX = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (tabuleiro[i][j] == 'O') {
                pecasO++;
            } else if (tabuleiro[i][j] == 'X') {
                pecasX++;
            }
        }
    }
    if (pecasO == 0) {
    printf(VERMELHO NEGRITO "\nPC (X) venceu com VITORIA TOTAL!\nTodas as pecas do Jogador 1 (O) foram capturadas.\n" RESET);
    finalizarTimer();
    registrarVitoria(2, 2);
    int opcao;
    do {
        printf(VERDE NEGRITO "\nDigite 1 para jogar novamente ou 0 para voltar ao menu principal: " RESET);
        scanf("%d", &opcao);
        if(opcao != 0 && opcao != 1) {
            printf(VERMELHO "Opção inválida! Tente novamente.\n" RESET);
        }
    } while(opcao != 0 && opcao != 1);
    limpaTela();
    if(opcao == 1) {
        iniciarJogoPc();
    } else {
        menu();
    }
    return 1;
} else if (pecasX == 0) {
    printf(AZUL NEGRITO "\nJogador 1 (O) venceu com VITORIA TOTAL!\nTodas as pecas do PC (X) foram capturadas.\n" RESET);
    finalizarTimer();
    registrarVitoria(2, 1);
    int opcao;
    do {
        printf(VERDE NEGRITO "\nDigite 1 para jogar novamente ou 0 para voltar ao menu principal: " RESET);
        scanf("%d", &opcao);
        if(opcao != 0 && opcao != 1) {
            printf(VERMELHO "Opção inválida! Tente novamente.\n" RESET);
        }
    } while(opcao != 0 && opcao != 1);
    limpaTela();
    if(opcao == 1) {
        iniciarJogoPc();
    } else {
        menu();
    }
    return 1;
}
return 0;
}

void dfsPequenaVitoria(char tabuleiro[SIZE][SIZE], int visitado[SIZE][SIZE], int x, int y) {
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE)
        return;
    if (visitado[x][y] == 1)
        return;
        
    visitado[x][y] = 1;
    dfsPequenaVitoria(tabuleiro, visitado, x + 1, y);
    dfsPequenaVitoria(tabuleiro, visitado, x - 1, y);
    dfsPequenaVitoria(tabuleiro, visitado, x, y + 1);
    dfsPequenaVitoria(tabuleiro, visitado, x, y - 1);
}

int checarPequenaVitoriaPvP(char tabuleiro[SIZE][SIZE], char jogador) {
    char adversario = (jogador == 'X') ? 'O' : 'X';
    int visitado[SIZE][SIZE];
        for (int i = 0; i < SIZE; i++) {
        int count = 0;
        for (int j = 0; j <= SIZE; j++) {
            if (j < SIZE && tabuleiro[i][j] == jogador) {
                count++;
            } else {
                if (count >= 5) {
                    memset(visitado, 0, sizeof(visitado));
                    for (int k = j - count; k < j; k++) {
                        visitado[i][k] = 1;
                    }
                    dfsPequenaVitoria(tabuleiro, visitado, 0, 0);
                    int adversarioNoLadoVisitado = 0;
                    int adversarioNoLadoNaoVisitado = 0;
                    int jogadorNoLadoVisitado = 0;
                    int jogadorNoLadoNaoVisitado = 0;
                    
                    for (int m = 0; m < SIZE; m++) {
                        for (int n = 0; n < SIZE; n++) {
                            if (tabuleiro[m][n] == adversario) {
                                if (visitado[m][n]) {
                                    adversarioNoLadoVisitado = 1;
                                } else {
                                    adversarioNoLadoNaoVisitado = 1;
                                }
                            } else if (tabuleiro[m][n] == jogador) {
                                if (visitado[m][n]) {
                                    jogadorNoLadoVisitado = 1;
                                } else {
                                    jogadorNoLadoNaoVisitado = 1;
                                }
                            }
                        }
                    }

                    if (((adversarioNoLadoVisitado && !adversarioNoLadoNaoVisitado && 
                          !jogadorNoLadoVisitado && jogadorNoLadoNaoVisitado) ||
                         (adversarioNoLadoNaoVisitado && !adversarioNoLadoVisitado && 
                          !jogadorNoLadoNaoVisitado && jogadorNoLadoVisitado))) {
                        declararPequenaVitoriaPvP(jogador);
                        return 1;
                    }
                }
                count = 0;
            }
        }
    }
        for (int j = 0; j < SIZE; j++) {
        int count = 0;
        for (int i = 0; i <= SIZE; i++) {
            if (i < SIZE && tabuleiro[i][j] == jogador) {
                count++;
            } else {
                if (count >= 5) {
                    memset(visitado, 0, sizeof(visitado));
                    for (int k = i - count; k < i; k++) {
                        visitado[k][j] = 1;
                    }
                    dfsPequenaVitoria(tabuleiro, visitado, 0, 0);
                    
                    int adversarioNoLadoVisitado = 0;
                    int adversarioNoLadoNaoVisitado = 0;
                    int jogadorNoLadoVisitado = 0;
                    int jogadorNoLadoNaoVisitado = 0;
                    
                    for (int m = 0; m < SIZE; m++) {
                        for (int n = 0; n < SIZE; n++) {
                            if (tabuleiro[m][n] == adversario) {
                                if (visitado[m][n]) {
                                    adversarioNoLadoVisitado = 1;
                                } else {
                                    adversarioNoLadoNaoVisitado = 1;
                                }
                            } else if (tabuleiro[m][n] == jogador) {
                                if (visitado[m][n]) {
                                    jogadorNoLadoVisitado = 1;
                                } else {
                                    jogadorNoLadoNaoVisitado = 1;
                                }
                            }
                        }
                    }
                    
                    if (((adversarioNoLadoVisitado && !adversarioNoLadoNaoVisitado && 
                          !jogadorNoLadoVisitado && jogadorNoLadoNaoVisitado) ||
                         (adversarioNoLadoNaoVisitado && !adversarioNoLadoVisitado && 
                          !jogadorNoLadoNaoVisitado && jogadorNoLadoVisitado))) {
                        declararPequenaVitoriaPvP(jogador);
                        return 1;
                    }
                }
                count = 0;
            }
        }
    }
    return 0;
}

int checarPequenaVitoriaPvPC(char tabuleiro[SIZE][SIZE], char jogador) {
    char adversario = (jogador == 'X') ? 'O' : 'X';
    int visitado[SIZE][SIZE];
    
    // Check horizontal lines
    for (int i = 0; i < SIZE; i++) {
        int count = 0;
        for (int j = 0; j <= SIZE; j++) {
            if (j < SIZE && tabuleiro[i][j] == jogador) {
                count++;
            } else {
                if (count >= 5) {
                    memset(visitado, 0, sizeof(visitado));
                    for (int k = j - count; k < j; k++) {
                        visitado[i][k] = 1;
                    }
                    dfsPequenaVitoria(tabuleiro, visitado, 0, 0);
                    
                    int adversarioNoLadoVisitado = 0;
                    int adversarioNoLadoNaoVisitado = 0;
                    int jogadorNoLadoVisitado = 0;
                    int jogadorNoLadoNaoVisitado = 0;
                    
                    for (int m = 0; m < SIZE; m++) {
                        for (int n = 0; n < SIZE; n++) {
                            if (tabuleiro[m][n] == adversario) {
                                if (visitado[m][n]) {
                                    adversarioNoLadoVisitado = 1;
                                } else {
                                    adversarioNoLadoNaoVisitado = 1;
                                }
                            } else if (tabuleiro[m][n] == jogador) {
                                if (visitado[m][n]) {
                                    jogadorNoLadoVisitado = 1;
                                } else {
                                    jogadorNoLadoNaoVisitado = 1;
                                }
                            }
                        }
                    }
                    
                    if (((adversarioNoLadoVisitado && !adversarioNoLadoNaoVisitado && 
                          !jogadorNoLadoVisitado && jogadorNoLadoNaoVisitado) ||
                         (adversarioNoLadoNaoVisitado && !adversarioNoLadoVisitado && 
                          !jogadorNoLadoNaoVisitado && jogadorNoLadoVisitado))) {
                        declararPequenaVitoriaPvPC(jogador);
                        return 1;
                    }
                }
                count = 0;
            }
        }
    }
    
    for (int j = 0; j < SIZE; j++) {
        int count = 0;
        for (int i = 0; i <= SIZE; i++) {
            if (i < SIZE && tabuleiro[i][j] == jogador) {
                count++;
            } else {
                if (count >= 5) {
                    memset(visitado, 0, sizeof(visitado));
                    for (int k = i - count; k < i; k++) {
                        visitado[k][j] = 1;
                    }
                    dfsPequenaVitoria(tabuleiro, visitado, 0, 0);
                    
                    int adversarioNoLadoVisitado = 0;
                    int adversarioNoLadoNaoVisitado = 0;
                    int jogadorNoLadoVisitado = 0;
                    int jogadorNoLadoNaoVisitado = 0;
                    
                    for (int m = 0; m < SIZE; m++) {
                        for (int n = 0; n < SIZE; n++) {
                            if (tabuleiro[m][n] == adversario) {
                                if (visitado[m][n]) {
                                    adversarioNoLadoVisitado = 1;
                                } else {
                                    adversarioNoLadoNaoVisitado = 1;
                                }
                            } else if (tabuleiro[m][n] == jogador) {
                                if (visitado[m][n]) {
                                    jogadorNoLadoVisitado = 1;
                                } else {
                                    jogadorNoLadoNaoVisitado = 1;
                                }
                            }
                        }
                    }
                    
                    if (((adversarioNoLadoVisitado && !adversarioNoLadoNaoVisitado && 
                          !jogadorNoLadoVisitado && jogadorNoLadoNaoVisitado) ||
                         (adversarioNoLadoNaoVisitado && !adversarioNoLadoVisitado && 
                          !jogadorNoLadoNaoVisitado && jogadorNoLadoVisitado))) {
                        declararPequenaVitoriaPvPC(jogador);
                        return 1;
                    }
                }
                count = 0;
            }
        }
    }
    return 0;
}

void declararPequenaVitoriaPvP(char jogador) {
    int opcao;
    if(jogador == 'X') {
        printf(VERMELHO NEGRITO "\nJogador X ganhou com PEQUENA VITORIA\n" RESET);
        finalizarTimer();
        registrarVitoria(1, 2);
    } else {
        printf(AZUL NEGRITO "\nJogador O ganhou com PEQUENA VITORIA\n" RESET);
        finalizarTimer();
        registrarVitoria(1, 1);
    }
    do {
        printf(VERDE NEGRITO "\nDigite 1 para jogar novamente ou 0 para voltar ao menu principal: " RESET);
        scanf("%d", &opcao);
        if(opcao != 0 && opcao != 1) {
            printf(VERMELHO "Opção inválida! Tente novamente.\n" RESET);
        }
    } while(opcao != 0 && opcao != 1);
    limpaTela();
    if(opcao == 1) {
        iniciarJogo(); 
    } else {
        menu();
    }
}

void declararPequenaVitoriaPvPC(char jogador) {
    int opcao;
    if(jogador == 'X') {
        printf(VERMELHO NEGRITO "\nPC (Jogador X) ganhou com PEQUENA VITORIA\n" RESET);
        finalizarTimer();
        registrarVitoria(2, 2);
    } else {
        printf(AZUL NEGRITO "\nJogador O ganhou com PEQUENA VITORIA\n" RESET);
        finalizarTimer();
        registrarVitoria(2, 1);
    }
    do {
        printf(VERDE NEGRITO "\nDigite 1 para jogar novamente ou 0 para voltar ao menu principal: " RESET);
        scanf("%d", &opcao);
        if(opcao != 0 && opcao != 1) {
            printf(VERMELHO "Opção inválida! Tente novamente.\n" RESET);
        }
    } while(opcao != 0 && opcao != 1);
    limpaTela();
    if(opcao == 1) {
        iniciarJogoPc(); 
    } else {
        menu();
    }
}

bool capturaPossivel(char tabuleiro[SIZE][SIZE], char jogador) {
    char adversario = (jogador == 'X') ? 'O' : 'X';
    int direcoes[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} }; // Cima, Baixo, Esquerda, Direita
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if(tabuleiro[i][j] == jogador) {
                for(int d = 0; d < 4; d++) {
                    int ni = i + direcoes[d][0];
                    int nj = j + direcoes[d][1];
                    if(ni >= 0 && ni < SIZE && nj >= 0 && nj < SIZE && tabuleiro[ni][nj] == '+') {
                        char tabuleiroTemp[SIZE][SIZE];
                        memcpy(tabuleiroTemp, tabuleiro, SIZE * SIZE * sizeof(char));
                        tabuleiroTemp[i][j] = '+';
                        tabuleiroTemp[ni][nj] = jogador;
                        for(int col = 0; col <= SIZE - 3; col++) {
                            if(tabuleiroTemp[ni][col] == jogador &&
                               tabuleiroTemp[ni][col+1] == adversario &&
                               tabuleiroTemp[ni][col+2] == jogador) {
                                return true;
                            }
                        }
                        for(int linha = 0; linha <= SIZE - 3; linha++) {
                            if(tabuleiroTemp[linha][nj] == jogador &&
                               tabuleiroTemp[linha+1][nj] == adversario &&
                               tabuleiroTemp[linha+2][nj] == jogador) {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

int checarEmpatePvP(char tabuleiro[SIZE][SIZE]) {
    int pecasO = 0;
    int pecasX = 0;
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if(tabuleiro[i][j] == 'O') {
                pecasO++;
            } else if(tabuleiro[i][j] == 'X') {
                pecasX++;
            }
        }
    }
    if(pecasO <= 3 && pecasX <= 3) {
        bool capturaO = capturaPossivel(tabuleiro, 'O');
        bool capturaX = capturaPossivel(tabuleiro, 'X');
        if(!capturaO && !capturaX) {
            printf(VERMELHO NEGRITO "\nEmp");
            printf(AZUL NEGRITO "ate!\n\n" RESET);
            finalizarTimer();
            registrarVitoria(1, 0);
            int opcao;
            do {
                printf(VERDE NEGRITO "\nDigite 1 para jogar novamente ou 0 para voltar ao menu principal: " RESET);
                scanf("%d", &opcao);
                if(opcao != 0 && opcao != 1) {
                    printf(VERMELHO "Opção invalida! Tente novamente.\n" RESET);
                }
            } while(opcao != 0 && opcao != 1);
            limpaTela();
            if(opcao == 1) {
                iniciarJogo(); 
            } else {
                menu();
            }
            return 1;
        }
    }
    return 0; 
}

int checarEmpatePvPC(char tabuleiro[SIZE][SIZE]) {
    int pecasO = 0;
    int pecasX = 0;
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if(tabuleiro[i][j] == 'O') {
                pecasO++;
            } else if(tabuleiro[i][j] == 'X') {
                pecasX++;
            }
        }
    }
    if(pecasO <= 3 && pecasX <= 3) {
        bool capturaO = capturaPossivel(tabuleiro, 'O');
        bool capturaX = capturaPossivel(tabuleiro, 'X');
        if(!capturaO && !capturaX) {
            printf(VERMELHO NEGRITO "\nEmp");
            printf(AZUL NEGRITO "ate!\n\n" RESET);
            finalizarTimer();
            registrarVitoria(2, 0);
            int opcao;
            do {
                printf(VERDE NEGRITO "\nDigite 1 para jogar novamente ou 0 para voltar ao menu principal: " RESET);
                scanf("%d", &opcao);
                if(opcao != 0 && opcao != 1) {
                    printf(VERMELHO "Opção invalida! Tente novamente.\n" RESET);
                }
            } while(opcao != 0 && opcao != 1);
            limpaTela();
            if(opcao == 1) {
                iniciarJogo(); 
            } else {
                menu();
            }
            return 1;
        }
    }
    return 0; 
}

void imprimirPossibilidadesMovimento(char tabuleiro[SIZE][SIZE], char jogador) {
    int movimentoEncontrado = 0;
    char tabuleiroCopia[SIZE][SIZE];
    printf(NEGRITO "\nPossibilidades de movimento para o jogador '%c':\n" RESET, jogador);
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if(tabuleiro[i][j] == jogador) {
                if(i > 0 && tabuleiro[i-1][j] == '+') {
                    memcpy(tabuleiroCopia, tabuleiro, SIZE * SIZE * sizeof(char));
                    tabuleiroCopia[i-1][j] = jogador;
                    tabuleiroCopia[i][j] = '+';
                    int resultaEmCaptura = verificarCaptura(tabuleiroCopia, jogador, i-1, j);
                    printf("-> Mover de %c%d para %c%d%s\n", 
                           'A' + i, j+1, 
                           'A' + (i-1), j+1,
                           resultaEmCaptura ? " [Captura] ": "");
                    movimentoEncontrado = 1;
                }
                if(i < SIZE-1 && tabuleiro[i+1][j] == '+') {
                    memcpy(tabuleiroCopia, tabuleiro, SIZE * SIZE * sizeof(char));
                    tabuleiroCopia[i+1][j] = jogador;
                    tabuleiroCopia[i][j] = '+';
                    int resultaEmCaptura = verificarCaptura(tabuleiroCopia, jogador, i+1, j);
                    printf("-> Mover de %c%d para %c%d%s\n", 
                           'A' + i, j+1, 
                           'A' + (i+1), j+1,
                           resultaEmCaptura ? " [Captura] " : "");
                    movimentoEncontrado = 1;
                }
                if(j > 0 && tabuleiro[i][j-1] == '+') {
                    memcpy(tabuleiroCopia, tabuleiro, SIZE * SIZE * sizeof(char));
                    tabuleiroCopia[i][j-1] = jogador;
                    tabuleiroCopia[i][j] = '+';
                    int resultaEmCaptura = verificarCaptura(tabuleiroCopia, jogador, i, j-1);
                    printf("-> Mover de %c%d para %c%d%s\n", 
                           'A' + i, j+1, 
                           'A' + i, j,
                           resultaEmCaptura ? " [Captura] " : "");
                    movimentoEncontrado = 1;
                }
                if(j < SIZE-1 && tabuleiro[i][j+1] == '+') {
                    memcpy(tabuleiroCopia, tabuleiro, SIZE * SIZE * sizeof(char));
                    tabuleiroCopia[i][j+1] = jogador;
                    tabuleiroCopia[i][j] = '+';
                    int resultaEmCaptura = verificarCaptura(tabuleiroCopia, jogador, i, j+1);
                    printf("-> Mover de %c%d para %c%d%s\n", 
                           'A' + i, j+1, 
                           'A' + i, j+2,
                           resultaEmCaptura ? " [Captura] " : "");
                    movimentoEncontrado = 1;
                }
            }
        }
    }
    if(!movimentoEncontrado) {
        printf("Jogador '%c' esta bloqueado!\n", jogador);
        printf("Mova-se para cima de uma peca adversaria em uma posicao adjascente\n");
    }
}

void escolherNovaPosicao(char tabuleiro[SIZE][SIZE], char jogador, int xOrigem, int yOrigem, int *xDestino, int *yDestino, int movimentosLivres, char turnoAtual) {
    char linhaDestino;
    int colunaDestino;
    char oponente = (jogador == 'O') ? 'X' : 'O';
    while (1) {
        printf(NEGRITO "Escolha a nova posicao de sua peca: " RESET);
        scanf(" %c%d", &linhaDestino, &colunaDestino);
        linhaDestino = toupper(linhaDestino);
        if (!posicaoParaIndices(linhaDestino, colunaDestino, xDestino, yDestino)) {
            printf("Posicao invalida. Tente novamente: \n");
            turnoAtual = (turnoAtual == 'O') ? 'X' : 'O';
            continue;
        }
        int movimentoAdjacente = ((abs(*xDestino - xOrigem) == 1 && *yDestino == yOrigem) ||
                                  (abs(*yDestino - yOrigem) == 1 && *xDestino == xOrigem));
        if (movimentosLivres) {
            if (tabuleiro[*xDestino][*yDestino] != '+' || !movimentoAdjacente) {
                printf("Movimento invalido. Tente novamente: \n");
                turnoAtual = (turnoAtual == 'O') ? 'X' : 'O';
                continue;
            }
        } else {
            if (tabuleiro[*xDestino][*yDestino] != oponente || !movimentoAdjacente) {
                printf("Posicao invalida. Tente novamente: \n");
                turnoAtual = (turnoAtual == 'O') ? 'X' : 'O';
                continue;
            }
        }
        break;
    }
}

void escolherPecaParaMover(char tabuleiro[SIZE][SIZE], char jogador, int *xOrigem, int *yOrigem, char *turnoAtual, int round, char *nomeJogador, char *corJogador) {
    char linhaOrigem = 'A', linhaDestino = 'A';
    int colunaOrigem = 1, colunaDestino = 1;
    while (1) {  
        printf("%s\n%s - " RESET, corJogador, nomeJogador);
        printf(NEGRITO "Escolha uma peca sua para mover" RESET VERDE
               " { P0 para Pause | S0 para Salvar e sair } " RESET);
        scanf(" %c%d", &linhaOrigem, &colunaOrigem);
        linhaOrigem = toupper(linhaOrigem);
        if (linhaOrigem == 'P' && colunaOrigem == 0) {
            pausarJogo();
            limpaTela();
            printf(NEGRITO AMARELO "Fase de Movimentacao\n\n" RESET);
            exibirTabuleiro(tabuleiro);
            if (round != 0 && linhaOrigem != 'P' && colunaOrigem != 0) {
                printf("\n< %s moveu sua peca de" NEGRITO
                       " %c%d para %c%d >\n" RESET, nomeJogador,
                       linhaOrigem, colunaOrigem, '-', 0);
            }
            imprimirPossibilidadesMovimento(tabuleiro, jogador);
            *turnoAtual = (*turnoAtual == 'O') ? 'X' : 'O';
            continue;
        }
        if (linhaOrigem == 'S' && colunaOrigem == 0) {
            salvarJogoTxt(tabuleiro, jogador);
            limpaTela();
            printf("Salvando jogo...\n");
            Sleep(2000);
            printf(VERDE "Jogo salvo com sucesso!\n" RESET);
            Sleep(2000);
            exit(0);
        }
        if (!posicaoParaIndices(linhaOrigem, colunaOrigem, xOrigem, yOrigem) ||
            tabuleiro[*xOrigem][*yOrigem] != jogador) {
            printf("Posicao invalida ou nao pertence ao jogador. Tente novamente: \n");
            *turnoAtual = (*turnoAtual == 'O') ? 'X' : 'O';
            continue;
        }
        break;
    }
}

void escolherNovaPosicaoPC(char tabuleiro[SIZE][SIZE], char jogador, int xOrigem, int yOrigem, int *xDestino, int *yDestino, int movimentosLivres, char turnoAtual) {
    char linhaDestino;
    int colunaDestino;
    char oponente = (jogador == 'O') ? 'X' : 'O';
    while (1) {
        printf(NEGRITO "Escolha a nova posicao de sua peca: " RESET);
        scanf(" %c%d", &linhaDestino, &colunaDestino);
        linhaDestino = toupper(linhaDestino);
        if (!posicaoParaIndices(linhaDestino, colunaDestino, xDestino, yDestino)) {
            printf("Posicao invalida. Tente novamente: \n");
            turnoAtual = (turnoAtual == 'O') ? 'X' : 'O';
            continue;
        }
        int movimentoAdjacente = ((abs(*xDestino - xOrigem) == 1 && *yDestino == yOrigem) ||
                                  (abs(*yDestino - yOrigem) == 1 && *xDestino == xOrigem));
        if (movimentosLivres) {
            if (tabuleiro[*xDestino][*yDestino] != '+' || !movimentoAdjacente) {
                printf("Movimento invalido. Tente novamente: \n");
                turnoAtual = (turnoAtual == 'O') ? 'X' : 'O';
                continue;
            }
        } else {
            if (tabuleiro[*xDestino][*yDestino] != oponente || !movimentoAdjacente) {
                printf("Posicao invalida. Tente novamente: \n");
                turnoAtual = (turnoAtual == 'O') ? 'X' : 'O';
                continue;
            }
        }
        break;
    }
}

void escolherPecaParaMoverPC(char tabuleiro[SIZE][SIZE], char jogador, int *xOrigem, int *yOrigem, char *turnoAtual, int round, char *nomeJogador, char *corJogador) {
    char linhaOrigem = 'A', linhaDestino = 'A';
    int colunaOrigem = 1, colunaDestino = 1;
    while (1) {  
        printf("%s\n%s - " RESET, corJogador, nomeJogador);
        printf(NEGRITO "Escolha uma peca sua para mover" RESET VERDE
               " { P0 para Pause | S0 para Salvar e sair } " RESET);
        scanf(" %c%d", &linhaOrigem, &colunaOrigem);
        linhaOrigem = toupper(linhaOrigem);
        if (linhaOrigem == 'P' && colunaOrigem == 0) {
            pausarJogo();
            limpaTela();
            printf(NEGRITO AMARELO "Fase de Movimentacao\n\n" RESET);
            exibirTabuleiro(tabuleiro);
            if (round != 0 && linhaOrigem != 'P' && colunaOrigem != 0) {
                printf("\n< %s moveu sua peca de" NEGRITO
                       " %c%d para %c%d >\n" RESET, nomeJogador,
                       linhaOrigem, colunaOrigem, '-', 0);
            }
            imprimirPossibilidadesMovimento(tabuleiro, jogador);
            *turnoAtual = (*turnoAtual == 'O') ? 'X' : 'O';
            continue;
        }
        if (linhaOrigem == 'S' && colunaOrigem == 0) {
            salvarJogoPvPC(tabuleiro);
            limpaTela();
            printf("Salvando jogo...\n");
            Sleep(2000);
            printf(VERDE "Jogo salvo com sucesso!\n" RESET);
            Sleep(2000);
            exit(0);
        }
        if (!posicaoParaIndices(linhaOrigem, colunaOrigem, xOrigem, yOrigem) ||
            tabuleiro[*xOrigem][*yOrigem] != jogador) {
            printf("Posicao invalida ou nao pertence ao jogador. Tente novamente: \n");
            *turnoAtual = (*turnoAtual == 'O') ? 'X' : 'O';
            continue;
        }
        break;
    }
}

int podeCapturarNovamente(char tabuleiro[SIZE][SIZE], int x, int y, char jogador) {
    char adversario = (jogador == 'O') ? 'X' : 'O';
    int direcoes[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; 
    for (int i = 0; i < 4; i++) {
        int xMov = x + direcoes[i][0];
        int yMov = y + direcoes[i][1];
        if (xMov >= 0 && xMov < SIZE && yMov >= 0 && yMov < SIZE && 
            tabuleiro[xMov][yMov] == '+') {
            for (int j = 0; j < 4; j++) {
                int x1 = xMov + direcoes[j][0];      
                int y1 = yMov + direcoes[j][1];
                int x2 = xMov + 2 * direcoes[j][0];  
                int y2 = yMov + 2 * direcoes[j][1];
                if (x1 >= 0 && x1 < SIZE && y1 >= 0 && y1 < SIZE &&
                    x2 >= 0 && x2 < SIZE && y2 >= 0 && y2 < SIZE) {
                    if (tabuleiro[x1][y1] == adversario && 
                        tabuleiro[x2][y2] == jogador) {
                        if (x1 == SIZE/2 && y1 == SIZE/2) {
                            continue;
                        }
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

void faseMovimentacao(char tabuleiro[SIZE][SIZE], char turnoAtual) {
    tabuleiro[2][2] = '+';
    int round = 0;
    int xOrigem, yOrigem, xDestino, yDestino;
    while (1) {
        char jogador = turnoAtual;
        char *corJogador = (jogador == 'O') ? AZUL NEGRITO : VERMELHO NEGRITO;
        char *nomeJogador = (jogador == 'O') ? "Jogador 1 (O)" : "Jogador 2 (X)";
        limpaTela();
        printf(NEGRITO AMARELO "Fase de Movimentacao\n\n" RESET);
        exibirTabuleiro(tabuleiro);
        int movimentosLivres = verificarMovimentosLivres(tabuleiro, jogador);
        if (round != 0) {
            if (turnoAtual == 'X') {
                printf("\n< Jogador O moveu sua peca de %c%d para %c%d >\n",
                       'A' + xOrigem, yOrigem + 1, 'A' + xDestino, yDestino + 1);
            } else {
                printf("\n< Jogador X moveu sua peca de %c%d para %c%d >\n",
                       'A' + xOrigem, yOrigem + 1, 'A' + xDestino, yDestino + 1);
            }
        }
        imprimirPossibilidadesMovimento(tabuleiro, jogador);
        escolherPecaParaMover(tabuleiro, jogador, &xOrigem, &yOrigem, &turnoAtual, round, nomeJogador, corJogador);
        escolherNovaPosicao(tabuleiro, jogador, xOrigem, yOrigem, &xDestino, &yDestino, movimentosLivres, turnoAtual);
        if (movimentosLivres) {
            tabuleiro[xOrigem][yOrigem] = '+';
            tabuleiro[xDestino][yDestino] = jogador;
        } else {
            tabuleiro[xDestino][yDestino] = jogador;
            tabuleiro[xOrigem][yOrigem] = '+';
        }
        int capturaFeita = verificarCaptura(tabuleiro, jogador, xDestino, yDestino);
        while (capturaFeita && podeCapturarNovamente(tabuleiro, xDestino, yDestino, jogador)) {
            limpaTela();
            printf(NEGRITO AMARELO "Fase de Movimentacao\n\n" RESET);
            exibirTabuleiro(tabuleiro);
            if (turnoAtual == 'X') {
                printf(NEGRITO VERMELHO "\nMovimento adicional concedido com a mesma peca\n\n" RESET);
            } else {
                printf(NEGRITO AZUL "\nMovimento adicional concedido com a mesma peca\n\n" RESET);
            }
            xOrigem = xDestino;
            yOrigem = yDestino;
            movimentosLivres = 1;
            escolherNovaPosicao(tabuleiro, jogador, xOrigem, yOrigem, &xDestino, &yDestino, movimentosLivres, turnoAtual);
            tabuleiro[xOrigem][yOrigem] = '+';
            tabuleiro[xDestino][yDestino] = jogador;
            capturaFeita = verificarCaptura(tabuleiro, jogador, xDestino, yDestino);
        }
        limpaTela();
        exibirTabuleiro(tabuleiro);
        if (checarPequenaVitoriaPvP(tabuleiro, jogador) ||
            checarVitoriaTotalPvP(tabuleiro) ||
            checarEmpatePvP(tabuleiro)) {
            return;
        }
        turnoAtual = (turnoAtual == 'O') ? 'X' : 'O';
        round++;
    }
}

void iniciarJogo() {
    char tabuleiro[SIZE][SIZE];
    inicializarTabuleiro(tabuleiro);
    iniciarTimer();
    char turnoAtual = 'O';
    while (1) {
        printf(NEGRITO AMARELO "Fase de colocacao\n\n" RESET);
        exibirTabuleiro(tabuleiro);
        jogar(tabuleiro,'O');
        if (tabuleiroCheio(tabuleiro)) {
            limpaTela();
            faseMovimentacao(tabuleiro, turnoAtual);
            break;
        }
        jogar(tabuleiro, 'X');
        if (tabuleiroCheio(tabuleiro)) {
            limpaTela();
            faseMovimentacao(tabuleiro, turnoAtual);
            break;
        }
        limpaTela();
    }
}

bool isMiddle(int x, int y) {
    return x == 2 && y == 2;
}

bool verificaSePodeCriarCaptura(char tabuleiro[SIZE][SIZE], int novoX, int novoY) {
    if (novoX == 2 && novoY == 2) return false;
    char tabuleiroTemp[SIZE][SIZE];
    memcpy(tabuleiroTemp, tabuleiro, SIZE * SIZE * sizeof(char));
    tabuleiroTemp[novoX][novoY] = 'X';
    int direcoes[2][2] = { {0, 1}, {1, 0} }; 
    for(int d = 0; d < 2; d++) {
        int dx = direcoes[d][0];
        int dy = direcoes[d][1];
        if (novoX + 2*dx < SIZE && novoY + 2*dy < SIZE &&
            tabuleiroTemp[novoX + dx][novoY + dy] == 'O' &&
            tabuleiroTemp[novoX + 2*dx][novoY + 2*dy] == 'X') {
            if (!isMiddle(novoX, novoY) && !isMiddle(novoX + dx, novoY + dy) && !isMiddle(novoX + 2*dx, novoY + 2*dy))
                return true;
        }
        if (novoX - 2*dx >= 0 && novoY - 2*dy >= 0 &&
            tabuleiroTemp[novoX - dx][novoY - dy] == 'O' &&
            tabuleiroTemp[novoX - 2*dx][novoY - 2*dy] == 'X') {
            if (!isMiddle(novoX, novoY) && !isMiddle(novoX - dx, novoY - dy) && !isMiddle(novoX - 2*dx, novoY - 2*dy))
                return true;
        }
    }
    return false;
}

void jogadaPC(char tabuleiro[SIZE][SIZE]) {
    struct Pos { int x, y; } posicoesPrioritarias[] = {
        {0,0}, {0,4}, {4,0}, {4,4}, {2,2}
    };
    int direcoes[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};
    for (int numJogada = 0; numJogada < 2; numJogada++) {
        bool jogadaFeita = false;
        int x, y;
        for (int i = 0; i < 5 && !jogadaFeita; i++) {
            if (tabuleiro[posicoesPrioritarias[i].x][posicoesPrioritarias[i].y] == '+') {
                x = posicoesPrioritarias[i].x;
                y = posicoesPrioritarias[i].y;
                jogadaFeita = true;
            }
        }
        for (int i = 0; i < SIZE && !jogadaFeita; i++) {
            for (int j = 0; j < SIZE && !jogadaFeita; j++) {
                if (tabuleiro[i][j] == 'O') {
                    for (int d = 0; d < 4; d++) {
                        int novoX = i + direcoes[d][0];
                        int novoY = j + direcoes[d][1];
                        if (novoX >= 0 && novoX < SIZE && novoY >= 0 && novoY < SIZE && 
                            tabuleiro[novoX][novoY] == '+') {
                            x = novoX;
                            y = novoY;
                            jogadaFeita = true;
                            break;
                        }
                    }
                }
            }
        }
        while (!jogadaFeita) {
            x = rand() % SIZE;
            y = rand() % SIZE;
            if (tabuleiro[x][y] == '+') {
                jogadaFeita = true;
            }
        }
        tabuleiro[x][y] = 'X';
        printf(VERMELHO "\nPC colocou X na posicao" RESET " %c%d\n", 'A' + x, y + 1);
        Sleep(2000);
    }
}

int iniciarJogoPc() {
    char tabuleiro[SIZE][SIZE];
    inicializarTabuleiro(tabuleiro);
    iniciarTimer();
    while (1) {
        printf(NEGRITO AMARELO "Fase de colocacao\n\n" RESET);
        exibirTabuleiro(tabuleiro);
        jogar(tabuleiro, 'O');
        if (tabuleiroCheio(tabuleiro)) {
            limpaTela();
            faseMovimentacaoPc(tabuleiro);
            break;
        }
        jogadaPC(tabuleiro);
        Sleep(1000); 
        if (tabuleiroCheio(tabuleiro)) {
            limpaTela();
            faseMovimentacaoPc(tabuleiro);
            break;
        }
        limpaTela();
    }
    return 1;
}

void jogadaPCMovimentacao(char tabuleiro[SIZE][SIZE]) {
    int xOrigem, yOrigem, xDestino, yDestino;
    bool movimentoEncontrado = false;
    int direcoes[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};

    for (int i = 0; i < SIZE && !movimentoEncontrado; i++) {
        for (int j = 0; j < SIZE && !movimentoEncontrado; j++) {
            if (tabuleiro[i][j] == 'X') {
                for (int d = 0; d < 4; d++) {
                    int novoX = i + direcoes[d][0];
                    int novoY = j + direcoes[d][1];
                    if (novoX >= 0 && novoX < SIZE && novoY >= 0 && novoY < SIZE &&
                        tabuleiro[novoX][novoY] == '+') {
                        char tabuleiroTemp[SIZE][SIZE];
                        memcpy(tabuleiroTemp, tabuleiro, SIZE * SIZE * sizeof(char));
                        tabuleiroTemp[i][j] = '+';
                        tabuleiroTemp[novoX][novoY] = 'X';
                        if (verificaSePodeCriarCaptura(tabuleiroTemp, novoX, novoY)) {
                            xOrigem = i;
                            yOrigem = j;
                            xDestino = novoX;
                            yDestino = novoY;
                            movimentoEncontrado = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!movimentoEncontrado) {
        int pecasMoviveis = 0;
        struct { int x, y; } pecas[25];
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (tabuleiro[i][j] == 'X') {
                    for (int d = 0; d < 4; d++) {
                        int novoX = i + direcoes[d][0];
                        int novoY = j + direcoes[d][1];
                        if (novoX >= 0 && novoX < SIZE && novoY >= 0 && novoY < SIZE &&
                            tabuleiro[novoX][novoY] == '+') {
                            pecas[pecasMoviveis].x = i;
                            pecas[pecasMoviveis].y = j;
                            pecasMoviveis++;
                            break;
                        }
                    }
                }
            }
        }
        if (pecasMoviveis > 0) {
            int pecaEscolhida = rand() % pecasMoviveis;
            xOrigem = pecas[pecaEscolhida].x;
            yOrigem = pecas[pecaEscolhida].y;
            for (int d = 0; d < 4; d++) {
                xDestino = xOrigem + direcoes[d][0];
                yDestino = yOrigem + direcoes[d][1];
                if (xDestino >= 0 && xDestino < SIZE && yDestino >= 0 && yDestino < SIZE &&
                    tabuleiro[xDestino][yDestino] == '+') {
                    movimentoEncontrado = true;
                    break;
                }
            }
        }
    }

    if (!movimentoEncontrado) {
        for (int i = 0; i < SIZE && !movimentoEncontrado; i++) {
            for (int j = 0; j < SIZE && !movimentoEncontrado; j++) {
                if (tabuleiro[i][j] == 'X') {
                    for (int d = 0; d < 4; d++) {
                        int novoX = i + direcoes[d][0];
                        int novoY = j + direcoes[d][1];
                        if (novoX >= 0 && novoX < SIZE && novoY >= 0 && novoY < SIZE &&
                            tabuleiro[novoX][novoY] == 'O') {
                            xOrigem = i;
                            yOrigem = j;
                            xDestino = novoX;
                            yDestino = novoY;
                            movimentoEncontrado = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (movimentoEncontrado) {
        tabuleiro[xOrigem][yOrigem] = '+';
        tabuleiro[xDestino][yDestino] = 'X';
        printf(VERMELHO "\nPC moveu de %c%d para %c%d\n" RESET,
               'A' + xOrigem, yOrigem + 1, 'A' + xDestino, yDestino + 1);

        int capturaFeita = verificarCaptura(tabuleiro, 'X', xDestino, yDestino);
        
        if (capturaFeita) {
            printf(NEGRITO VERMELHO "PC capturou uma peca adversaria\n" RESET);

            while (capturaFeita && podeCapturarNovamente(tabuleiro, xDestino, yDestino, 'X')) {
                printf(NEGRITO VERMELHO "\nMovimento adicional concedido com a mesma peca\n" RESET);
                Sleep(1000);
                
                bool capturaAdicionalEncontrada = false;
                for (int d = 0; d < 4 && !capturaAdicionalEncontrada; d++) {
                    int novoX = xDestino + direcoes[d][0];
                    int novoY = yDestino + direcoes[d][1];
                    
                    if (novoX >= 0 && novoX < SIZE && novoY >= 0 && novoY < SIZE &&
                        tabuleiro[novoX][novoY] == '+') {
                        char tabuleiroTemp[SIZE][SIZE];
                        memcpy(tabuleiroTemp, tabuleiro, SIZE * SIZE * sizeof(char));
                        tabuleiroTemp[xDestino][yDestino] = '+';
                        tabuleiroTemp[novoX][novoY] = 'X';
                        
                        if (verificaSePodeCriarCaptura(tabuleiroTemp, novoX, novoY)) {
                            xOrigem = xDestino;
                            yOrigem = yDestino;
                            xDestino = novoX;
                            yDestino = novoY;
                            capturaAdicionalEncontrada = true;
                            
                            tabuleiro[xOrigem][yOrigem] = '+';
                            tabuleiro[xDestino][yDestino] = 'X';
                            printf(VERMELHO "\nPC moveu de %c%d para %c%d\n" RESET,
                                   'A' + xOrigem, yOrigem + 1, 'A' + xDestino, yDestino + 1);
                            
                            capturaFeita = verificarCaptura(tabuleiro, 'X', xDestino, yDestino);
                            if (capturaFeita) {
                                printf(NEGRITO VERMELHO "PC capturou uma peca adversaria\n" RESET);
                            }
                            break;
                        }
                    }
                }
                if (!capturaAdicionalEncontrada) break;
            }
        }
        Sleep(2000);
    }
}

void faseMovimentacaoPc(char tabuleiro[SIZE][SIZE]) {
    tabuleiro[2][2] = '+'; 
    int round = 0;
    int xOrigem, yOrigem, xDestino, yDestino;
    char turnoAtual = 'O'; 
    while (1) {
        while (1) {
            limpaTela();
            printf(NEGRITO AMARELO "Fase de Movimentacao\n\n" RESET);
            exibirTabuleiro(tabuleiro);
            int movimentosLivresJogador1 = verificarMovimentosLivres(tabuleiro, 'O');
            imprimirPossibilidadesMovimento(tabuleiro, 'O');
            char *corJogador = AZUL NEGRITO;
            char *nomeJogador = "Jogador 1 (O)";
            escolherPecaParaMoverPC(tabuleiro, 'O', &xOrigem, &yOrigem, &turnoAtual, round, nomeJogador, corJogador);
            escolherNovaPosicaoPC(tabuleiro, 'O', xOrigem, yOrigem, &xDestino, &yDestino, movimentosLivresJogador1, turnoAtual);
            if (movimentosLivresJogador1) {
                tabuleiro[xOrigem][yOrigem] = '+';    
                tabuleiro[xDestino][yDestino] = 'O';  
            } else {
                tabuleiro[xDestino][yDestino] = 'O'; 
                tabuleiro[xOrigem][yOrigem] = '+';
            }
            
            int capturaFeita = verificarCaptura(tabuleiro, 'O', xDestino, yDestino);
            while (capturaFeita && podeCapturarNovamente(tabuleiro, xDestino, yDestino, 'O')) {
                limpaTela();
                printf(NEGRITO AMARELO "Fase de Movimentacao\n\n" RESET);
                exibirTabuleiro(tabuleiro);
                printf(NEGRITO AZUL "\nMovimento adicional concedido com a mesma peca\n\n" RESET);
                xOrigem = xDestino;
                yOrigem = yDestino;
                movimentosLivresJogador1 = 1;
                escolherNovaPosicao(tabuleiro, 'O', xOrigem, yOrigem, &xDestino, &yDestino, movimentosLivresJogador1, turnoAtual);
                tabuleiro[xOrigem][yOrigem] = '+';
                tabuleiro[xDestino][yDestino] = 'O';
                capturaFeita = verificarCaptura(tabuleiro, 'O', xDestino, yDestino);
            }
            
            limpaTela();
            exibirTabuleiro(tabuleiro);
            if (checarPequenaVitoriaPvPC(tabuleiro, 'O') || 
                checarVitoriaTotalPvPC(tabuleiro) || 
                checarEmpatePvPC(tabuleiro)) {
                return; 
            }
            break;
        }
        round++;
        limpaTela();
        printf(NEGRITO AMARELO "Fase de Movimentacao\n\n" RESET);
        exibirTabuleiro(tabuleiro);
        printf(VERMELHO "\nTurno do PC (X)...\n" RESET);
        Sleep(1000);
        jogadaPCMovimentacao(tabuleiro);
        verificarCaptura(tabuleiro, 'X', xDestino, yDestino);
        limpaTela();
        exibirTabuleiro(tabuleiro);
        if (checarPequenaVitoriaPvPC(tabuleiro, 'X') || 
            checarVitoriaTotalPvPC(tabuleiro) || 
            checarEmpatePvPC(tabuleiro)) {
            return;
        }
        Sleep(1000);
    }
}

void jogar(char tabuleiro[SIZE][SIZE], char turnoAtual) {
    int x, y;
    char linha;
    int coluna;
    for (int i = 0; i < 2; i++) {
        while (1) {
            if (turnoAtual == 'O') {
                printf(AZUL NEGRITO "\nJogador %c, escolha uma posicao para %c" RESET 
                       VERDE " { P0 para Pause } " RESET, turnoAtual, turnoAtual);
            } else {
                printf(VERMELHO NEGRITO "\nJogador %c, escolha uma posicao para %c" RESET 
                       VERDE " { P0 para Pause } " RESET, turnoAtual, turnoAtual);
            }
            while (getchar() != '\n'); 
            scanf("%c%d", &linha, &coluna);
            linha = toupper(linha);
            if (linha == 'P' && coluna == 0) {
                pausarJogo();
                limpaTela();
                printf(NEGRITO AMARELO "Fase de colocacao\n\n" RESET);
                exibirTabuleiro(tabuleiro);
                continue;
            }
            if (posicaoParaIndices(linha, coluna, &x, &y)) {
                if (tabuleiro[x][y] == '+' && tabuleiro[x][y] != turnoAtual) {
                    break;
                } else {
                    printf("Posicao indisponivel. Tente novamente: \n");
                }
            } else {
                printf("Posicao invalida. Tente novamente.\n");
            }
        }
        limpaTela();
        tabuleiro[x][y] = turnoAtual;
        printf(NEGRITO AMARELO "Fase de colocacao\n\n" RESET);
        exibirTabuleiro(tabuleiro);
    }
}

bool carregarJogoTxt() {
    char tabuleiro[SIZE][SIZE];
    char turnoAtual;
    time_t tempoDecorrido;
    FILE *arquivo = fopen("jogo_save.txt", "r");
    if (arquivo == NULL) {
        limpaTela();
        printf(NEGRITO VERMELHO "\nNenhum jogo Player X Player salvo encontrado!\n" RESET);
        Sleep(2000);
        limpaTela();
        menu();
        return false;
    }
    char linha[100];
    fgets(linha, sizeof(linha), arquivo);
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            tabuleiro[i][j] = fgetc(arquivo);
        }
        fgetc(arquivo); 
    }
    fgets(linha, sizeof(linha), arquivo);
    fgets(linha, sizeof(linha), arquivo);
    tempoDecorrido = atol(linha);
    fgets(linha, sizeof(linha), arquivo);
    fgets(linha, sizeof(linha), arquivo);
    turnoAtual = linha[0];
    fclose(arquivo);
    tempoInicial = time(NULL) - tempoDecorrido;
    printf(VERDE "\nJogo carregado com sucesso!\n" RESET);
    printf(AMARELO NEGRITO "Pressione ENTER para continuar: " RESET);
    getchar();
    limpaTela();
    faseMovimentacao(tabuleiro, turnoAtual);
    return true;
}

bool carregarJogoPvPC() {
    char tabuleiro[SIZE][SIZE];
    time_t tempoDecorrido;
    FILE *arquivo = fopen("jogo_save_pvpc.txt", "r");
    if (arquivo == NULL) {
        limpaTela();
        printf(NEGRITO VERMELHO "\nNenhum jogo Player X PC salvo encontrado!\n" RESET);
        Sleep(2000);
        limpaTela();
        menu();
        return false;
    }
    char linha[100];
    fgets(linha, sizeof(linha), arquivo);
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            tabuleiro[i][j] = fgetc(arquivo);
        }
        fgetc(arquivo); 
    }
    fgets(linha, sizeof(linha), arquivo);
    fgets(linha, sizeof(linha), arquivo);
    tempoDecorrido = atol(linha);
    fclose(arquivo);
    tempoInicial = time(NULL) - tempoDecorrido;
    printf(VERDE "\nJogo PvPC carregado com sucesso!\n" RESET);
    printf(AMARELO NEGRITO "Pressione ENTER para continuar: " RESET);
    getchar();
    limpaTela();
    faseMovimentacaoPc(tabuleiro);
    return true;
}

int main() {
    menu();
    return 0;
}
