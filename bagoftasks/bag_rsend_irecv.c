#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

#define TAMANHO 500000

int primo (int n) {
    int i;
    for (i = 3; i < (int)(sqrt(n) + 1); i+=2) {
        if(n%i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    double t_inicial, t_final;
    int cont = 0, total = 0;
    int i, n;
    int meu_ranque, num_procs, inicio, dest, raiz=0, tag=1, stop=0;
    MPI_Status estado;
    MPI_Request request;

    if (argc < 2) {
        printf("Entre com o valor do maior inteiro como parâmetro.\n");
        return 0;
    } else {
        n = strtol(argv[1], (char **) NULL, 10);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (num_procs < 2) {
        printf("Este programa deve ser executado com no mínimo dois processos.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return(1);
    }

    // --- SINCRONIZAÇÃO CRÍTICA PARA RSEND ---
    if (meu_ranque != 0) {
        // Escravos já postam o primeiro Irecv antes de tudo
        MPI_Irecv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
    }
    
    // Todos esperam aqui. Isso garante que o Irecv acima foi postado por todos os escravos.
    MPI_Barrier(MPI_COMM_WORLD);

    t_inicial = MPI_Wtime();

    if (meu_ranque == 0) { 
        // Distribuição inicial com Ready Send (Rsend)
        for (dest=1, inicio=3; dest < num_procs; dest++, inicio += TAMANHO) {
            tag = (inicio > n) ? 98 : 1;
            MPI_Rsend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }

        while (stop < (num_procs-1)) {
            // Mestre recebe o resultado
            MPI_Irecv(&cont, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &estado);
            
            total += cont;
            dest = estado.MPI_SOURCE;

            if (inicio > n) {
                tag = 99;
                stop++;
            } else {
                tag = 1;
            }

            // O escravo já postou o Irecv (veja no loop do escravo), então podemos usar Rsend
            MPI_Rsend(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            inicio += TAMANHO;
        }
    } 
    else { 
        // Escravos
        while (1) {
            // O primeiro Irecv foi postado ANTES do Barrier. Os próximos são postados aqui.
            MPI_Wait(&request, &estado);

            if (estado.MPI_TAG == 99) break;

            cont = 0;
            if (estado.MPI_TAG != 98) {
                for (i = inicio; i < (inicio + TAMANHO) && i < n; i+=2) {
                    if (primo(i) == 1) cont++;
                }
            }
            
            // Antes de mandar o resultado, o escravo já abre o Irecv para a PRÓXIMA tarefa.
            // Isso garante que quando o mestre receber o resultado e der um Rsend, o escravo já esteja pronto.
            MPI_Irecv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            
            // Envia o resultado para o mestre (usamos Send comum para o mestre)
            MPI_Send(&cont, 1, MPI_INT, raiz, 1, MPI_COMM_WORLD);
        } 
        t_final = MPI_Wtime();
    }

    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1; 
        printf("Quant. de primos entre 1 e %d: %d \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);           
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return(0);
}