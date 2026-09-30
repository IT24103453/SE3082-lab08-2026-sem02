// SumMPI.cc
// Exercise 2 - Parallelize adding up numbers 1 to 10,000,000 using MPI.
//
// Approach: the total range [1, N] is split into (roughly) equal
// contiguous chunks, one per process. Each process sums its own chunk
// locally, then MPI_Reduce combines all the partial sums into a single
// total on rank 0. Wall-clock time (from MPI_Init to the reduce
// finishing) is printed on rank 0 so it can be captured for the
// Exercise 4 Time-vs-Processors / Speedup graphs.
//
// Build: mpicxx -O2 SumMPI.cc -o SumMPI
// Run:   mpirun --oversubscribe -np <P> ./SumMPI

#include <mpi.h>
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const long long N = 10000000LL; // 1 .. 10,000,000

    double t_start = MPI_Wtime();

    // Split N items as evenly as possible across `size` processes.
    long long base = N / size;
    long long rem  = N % size;

    // Ranks < rem get one extra element so the split stays balanced.
    long long my_count = base + (rank < rem ? 1 : 0);
    long long my_start  = rank * base + (rank < rem ? rank : rem) + 1; // first number for this rank
    long long my_end    = my_start + my_count - 1;                    // last number for this rank

    long long local_sum = 0;
    for (long long i = my_start; i <= my_end; ++i) {
        local_sum += i;
    }

    long long total_sum = 0;
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    double t_end = MPI_Wtime();
    double local_elapsed = t_end - t_start;
    double max_elapsed;
    MPI_Reduce(&local_elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        long long expected = N * (N + 1) / 2;
        std::cout << "Processes: " << size
                  << "  Sum(1.." << N << ") = " << total_sum
                  << "  (expected " << expected << ")"
                  << "  Time: " << max_elapsed << " s\n";
    }

    MPI_Finalize();
    return 0;
}
