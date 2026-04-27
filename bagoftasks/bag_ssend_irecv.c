#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

#define TAMANHO 500000

int primo (long int n) {
    long int i;
    for (i = 3; i < (long int)(sqrt(n) + 1); i+=2) {
        if(n%i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    double t_inicial, t_final;
    long int cont = 0, total = 0;
    int i;
    long int n, inicio, recebido;
    int meu_ranque, num_procs, dest, raiz=0, tag=1, stop=0;
    
    MPI_Status estado;
    MPI_Request request; // Necessário para gerenciar o Irecv

    if (argc < 2) {
        if (meu_ranque == 0) printf("Entre com o valor do maior inteiro como parâmetro.\n");
        return 0;
    } else {
        n = strtol(argv[1], (char **) NULL, 10);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (num_procs < 2) {
        if (meu_ranque == 0) printf("Este programa exige pelo menos 2 processos.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return(1);
    }

    t_inicial = MPI_Wtime();

    if (meu_ranque == 0) { 
        // --- MESTRE: Distribuição Inicial ---
        for (dest=1, inicio=3; dest < num_procs; dest++, inicio += TAMANHO) {
            tag = (inicio > n) ? 98 : 1;
            MPI_Ssend(&inicio, 1, MPI_LONG, dest, tag, MPI_COMM_WORLD);
        }

        // --- MESTRE: Gerenciamento da Bolsa ---
        while (stop < (num_procs-1)) {
            // Posta o recebimento não-bloqueante
            MPI_Irecv(&recebido, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            
            // Bloqueia até que a mensagem de algum escravo chegue
            MPI_Wait(&request, &estado);
            
            total += recebido;
            dest = estado.MPI_SOURCE;

            if (inicio > n) {
                tag = 99; // Finalização
                stop++;
            } else {
                tag = 1;
            }

            // Envia a próxima tarefa
            MPI_Ssend(&inicio, 1, MPI_LONG, dest, tag, MPI_COMM_WORLD);
            inicio += TAMANHO;
        }
    } 
    else { 
        // --- ESCRAVO ---
        while (1) {
            // Escravo posta o Irecv para receber a tarefa (inicio)
            MPI_Irecv(&inicio, 1, MPI_LONG, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            
            // Espera a tarefa chegar para começar a computar
            MPI_Wait(&request, &estado);

            if (estado.MPI_TAG == 99) break;

            cont = 0;
            if (estado.MPI_TAG != 98) {
                for (long int j = inicio; j < (inicio + TAMANHO) && j <= n; j += 2) {
                    if (primo(j)) cont++;
                }
            }
            
            // Envia o resultado de volta de forma síncrona
            MPI_Ssend(&cont, 1, MPI_LONG, raiz, 1, MPI_COMM_WORLD);
        } 
        t_final = MPI_Wtime();
    }

    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1; // Soma o 2
        printf("Quant. de primos entre 1 e %ld: %ld \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);           
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return(0);
}