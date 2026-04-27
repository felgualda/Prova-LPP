#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

int primo (long int n) {
	int i;
       
	for (i = 3; i < (int)(sqrt(n) + 1); i+=2) {
			if(n%i == 0) return 0;
	}
	return 1;
}

int main(int argc, char *argv[]) {
double t_inicial, t_final;
int cont = 0, total = 0, pot2, destino, etiq = 1, i_arvore = 0;
long int i, n, recebido;
MPI_Status estado;

int meu_ranque, num_procs, inicio, salto;

	if (argc < 2) {
        	printf("Valor inválido! Entre com um valor do maior inteiro\n");
       	 	return 0;
    	} else {
        	n = strtol(argv[1], (char **) NULL, 10);
       	}
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);	
    	t_inicial = MPI_Wtime();
    	inicio = 3 + meu_ranque*2;
    	salto = num_procs*2;
	for (i = inicio; i <= n; i += salto) 
		if(primo(i) == 1) cont++;
		
    long int acumulado = cont; 
	MPI_Request request; // MUDANÇA********************************


    for (i_arvore = 1; i_arvore < num_procs; i_arvore += i_arvore) {
        if ((meu_ranque / i_arvore) % 2 == 0) {
            destino = meu_ranque + i_arvore;
            if (destino < num_procs) {
                MPI_Ssend(&acumulado, 1, MPI_LONG, destino, etiq, MPI_COMM_WORLD); // MUDANÇA********************************
                MPI_Irecv(&recebido, 1, MPI_LONG, destino, etiq, MPI_COMM_WORLD, &request);
				MPI_Wait(&request, MPI_STATUS_IGNORE); // MUDANÇA********************************
                acumulado += recebido;
            }
        } else {
            destino = meu_ranque - i_arvore;
            if (destino >= 0) {
                MPI_Irecv(&recebido, 1, MPI_LONG, destino, etiq, MPI_COMM_WORLD, &request); // MUDANÇA********************************
                MPI_Ssend(&acumulado, 1, MPI_LONG, destino, etiq, MPI_COMM_WORLD);
				MPI_Wait(&request, MPI_STATUS_IGNORE); // MUDANÇA********************************
                acumulado += recebido;
            }
        }
    }
    total = acumulado;
	
	t_final = MPI_Wtime();
	if (meu_ranque == 0) {
        total += 1;
		printf("Quant. de primos entre 1 e n: %d \n", total);
		printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
	}
	MPI_Finalize();
	return(0);
}