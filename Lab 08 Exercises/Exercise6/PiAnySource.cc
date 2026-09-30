// PiAnySource.cc
// Exercise 6 - "Rewrite Exercise 3 by changing the receiving Tag to
// MPI_ANY_SOURCE, redo Exercise 3 and compare with the original results."
//
// Note on wording: MPI_ANY_SOURCE is a wildcard for the *source* rank
// argument of MPI_Recv, not the tag argument (that wildcard is
// MPI_ANY_TAG). This file makes the change the exercise is actually
// after: instead of rank 0 looping "for src = 1..size-1" and demanding
// the message from that exact rank, it just says "give me the next
// worker result from anyone" and reads the true sender out of the
// MPI_Status.
//
// This is the same computation as ex3_pi_montecarlo.cc, so the Pi
// estimate should match (within Monte-Carlo noise) - only the order in
// which the master happens to service workers can change, since a
// slower worker no longer blocks the master from processing a faster
// worker's result first.
//
// Build: mpicxx -O2 PiAnySource.cc -o PiAnySource
// Run:   mpirun --oversubscribe -np <P> ./PiAnySource

#include <mpi.h>
#include <iostream>
#include <random>

static const int TAG_COUNT = 42;
static const long long TOTAL_POINTS = 10000000LL;

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

    long long my_hits = throw_darts(my_points, 12345u + rank * 977u);

    long long total_hits = 0;
    long long total_points = 0;

    if (rank == 0) {
        total_hits = my_hits;
        total_points = my_points;
        for (int i = 1; i < size; ++i) {
            long long worker_hits;
            MPI_Status status;
            // CHANGED from Exercise 3: source = MPI_ANY_SOURCE instead of
            // an explicit rank. Whichever worker finishes first (and has
            // a message waiting) gets serviced first.
            MPI_Recv(&worker_hits, 1, MPI_LONG_LONG, MPI_ANY_SOURCE, TAG_COUNT,
                     MPI_COMM_WORLD, &status);
            int src = status.MPI_SOURCE; // find out who it actually was
            long long worker_points = base + (src < rem ? 1 : 0);
            total_hits += worker_hits;
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
        std::cout << "[ANY_SOURCE] Processes: " << size
                  << "  Points: " << total_points
                  << "  Pi estimate: " << pi_estimate
                  << "  Time: " << max_elapsed << " s\n";
    }

    MPI_Finalize();
    return 0;
}
