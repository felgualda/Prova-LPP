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
    long int cont = 0, total = 0;
    int i, n_arg;
    long int n, inicio, recebido;
    int meu_ranque, num_procs, dest, raiz=0, tag=1, stop=0;
    MPI_Status estado;

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
            // Recebe o resultado de qualquer escravo que terminar
            MPI_Recv(&recebido, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
            total += recebido;
            dest = estado.MPI_SOURCE;

            if (inicio > n) {
                tag = 99; // Poison pill (finalização)
                stop++;
            } else {
                tag = 1;
            }

            // Envia a próxima tarefa síncronamente
            MPI_Ssend(&inicio, 1, MPI_LONG, dest, tag, MPI_COMM_WORLD);
            inicio += TAMANHO;
        }
    } 
    else { 
        // --- ESCRAVO ---
        while (1) {
            // Recebe tarefa bloqueante
            MPI_Recv(&inicio, 1, MPI_LONG, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);

            if (estado.MPI_TAG == 99) break;

            cont = 0;
            if (estado.MPI_TAG != 98) {
                for (long int j = inicio; j < (inicio + TAMANHO) && j <= n; j += 2) {
                    if (primo(j)) cont++;
                }
            }
            // Envia resultado síncronamente
            long int resultado_escravo = cont;
            MPI_Ssend(&resultado_escravo, 1, MPI_LONG, raiz, 1, MPI_COMM_WORLD);
        } 
        t_final = MPI_Wtime();
    }

    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        total += 1; // Soma o número 2
        printf("Quant. de primos entre 1 e %ld: %ld \n", n, total);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);           
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return(0);
}