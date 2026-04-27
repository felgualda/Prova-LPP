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
int cont = 0, total = 0;
long int i, n, recebido;
int meu_ranque, num_procs, inicio, salto, i_arv, etiq =1, destino;
MPI_Status estado;


	if (argc < 2) {
        	printf("Valor inválido! Entre com um valor do maior inteiro\n");
       	 	return 0;
    	} else {
        	n = strtol(argv[1], (char **) NULL, 10);
       	}
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);                            // MUDANÇA********************************
    	t_inicial = MPI_Wtime();
    	inicio = 3 + meu_ranque*2;
    	salto = num_procs*2;
	for (i = inicio; i <= n; i += salto) 
		if(primo(i) == 1) cont++;
		
    long int acumulado = cont; 


    for (i_arv = 1; i_arv < num_procs; i_arv += i_arv) {
        if ((meu_ranque / i_arv) % 2 == 0) {
            destino = meu_ranque + i_arv;
            if (destino < num_procs) {
                MPI_Rsend(&acumulado, 1, MPI_LONG, destino, etiq, MPI_COMM_WORLD);
                MPI_Recv(&recebido, 1, MPI_LONG, destino, etiq, MPI_COMM_WORLD, &estado);
                acumulado += recebido;
            }
        } else {
            destino = meu_ranque - i_arv;
            if (destino >= 0) {
                MPI_Recv(&recebido, 1, MPI_LONG, destino, etiq, MPI_COMM_WORLD, &estado);
                MPI_Rsend(&acumulado, 1, MPI_LONG, destino, etiq, MPI_COMM_WORLD);
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