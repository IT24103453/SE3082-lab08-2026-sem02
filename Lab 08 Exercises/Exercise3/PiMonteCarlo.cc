// PiMonteCarlo.cc
// Exercise 3 - Parallelize the Monte Carlo computation of Pi using MPI.
//
// Monte Carlo idea: throw random points into the unit square [0,1]x[0,1]
// and count how many land inside the quarter circle (x^2+y^2<=1).
// pi ~= 4 * (points_inside / total_points).
//
// Design (deliberately explicit master/worker Send/Recv, NOT MPI_Reduce):
// this is what lets Exercise 6 and 7 make sense later, since they ask us
// to change how the master *receives* the partial results.
//   - Every process (including rank 0) throws its own share of darts.
//   - Every worker rank (rank != 0) MPI_Send's its local hit-count to
//     rank 0, tagged with TAG_COUNT.
//   - Rank 0 receives the (size-1) worker counts with an explicit
//     source loop (source = i, i = 1..size-1) and adds its own local
//     count to get the grand total.
//
// Build: mpicxx -O2 PiMonteCarlo.cc -o PiMonteCarlo
// Run:   mpirun --oversubscribe -np <P> ./PiMonteCarlo

#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <random>

static const int TAG_COUNT = 42;
static const long long TOTAL_POINTS = 10000000LL; // "Find the calculation for 10,000,000 times"

long long throw_darts(long long n_points, unsigned int seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    long long hits = 0;
    for (long long i = 0; i < n_points; ++i) {
        double x = dist(rng);
        double y = dist(rng);
        if (x * x + y * y <= 1.0) ++hits;
    }
    return hits;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double t_start = MPI_Wtime();

    long long base = TOTAL_POINTS / size;
    long long rem  = TOTAL_POINTS % size;
    long long my_points = base + (rank < rem ? 1 : 0);

    // Seed differs per rank so every process draws different random points.
    long long my_hits = throw_darts(my_points, 12345u + rank * 977u);

    long long total_hits = 0;
    long long total_points = 0;

    if (rank == 0) {
        total_hits = my_hits;
        total_points = my_points;
        for (int src = 1; src < size; ++src) {
            long long worker_hits;
            // Explicit source: we only accept the message from rank `src`.
            MPI_Recv(&worker_hits, 1, MPI_LONG_LONG, src, TAG_COUNT,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_hits += worker_hits;

            long long worker_points = base + (src < rem ? 1 : 0);
            total_points += worker_points;
        }
    } else {
        MPI_Send(&my_hits, 1, MPI_LONG_LONG, 0, TAG_COUNT, MPI_COMM_WORLD);
    }

    double t_end = MPI_Wtime();
    double local_elapsed = t_end - t_start;
    double max_elapsed;
    MPI_Reduce(&local_elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double pi_estimate = 4.0 * static_cast<double>(total_hits) / static_cast<double>(total_points);
        std::cout << "Processes: " << size
                  << "  Points: " << total_points
                  << "  Pi estimate: " << pi_estimate
                  << "  Time: " << max_elapsed << " s\n";
    }

    MPI_Finalize();
    return 0;
}
