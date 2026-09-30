#include <stdio.h>
#include <mpi.h>

int main(void) {
    int rank;
    char name[MPI_MAX_PROCESSOR_NAME];
    int len;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(name, &len);

    if (rank == 0)
        printf("Hello World! From rank %d machine %s\n", rank, name);
    else
        printf("Just a normal process From rank %d machine %s\n", rank, name);

    MPI_Finalize();
    return 0;
}
