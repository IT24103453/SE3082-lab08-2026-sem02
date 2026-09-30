// PiAnySourceBSend.cc
// Exercise 7 - "Rewrite Exercise 6 by using Buffered Send (BSend)."
//
// Same as ex6_pi_any_source.cc, except every worker now sends its hit
// count with MPI_Bsend instead of MPI_Send. Each worker only ever sends
// once, so a small buffer sized for a single long long is enough; every
// rank attaches/detaches its own buffer around its one Bsend call.
// The master's receive loop is unchanged (still MPI_ANY_SOURCE from
// Exercise 6), and MPI_Recv works identically regardless of whether the
// sender used Send or Bsend - buffering only changes the SENDER's
// behaviour, not the receiver's.
//
// Build: mpicxx -O2 PiAnySourceBSend.cc -o PiAnySourceBSend
// Run:   mpirun --oversubscribe -np <P> ./PiAnySourceBSend

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
            MPI_Recv(&worker_hits, 1, MPI_LONG_LONG, MPI_ANY_SOURCE, TAG_COUNT,
                     MPI_COMM_WORLD, &status);
            int src = status.MPI_SOURCE;
            long long worker_points = base + (src < rem ? 1 : 0);
            total_hits += worker_hits;
            total_points += worker_points;
        }
    } else {
        // CHANGED from Exercise 6: use a buffered send for the one
        // message this worker ever sends.
        int buffer_size = sizeof(long long) + MPI_BSEND_OVERHEAD;
        char* buffer = new char[buffer_size];
        MPI_Buffer_attach(buffer, buffer_size);

        MPI_Bsend(&my_hits, 1, MPI_LONG_LONG, 0, TAG_COUNT, MPI_COMM_WORLD);

        void* buf_ptr;
        int buf_size;
        MPI_Buffer_detach(&buf_ptr, &buf_size); // waits until the send completes
        delete[] static_cast<char*>(buf_ptr);
    }

    double t_end = MPI_Wtime();
    double local_elapsed = t_end - t_start;
    double max_elapsed;
    MPI_Reduce(&local_elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double pi_estimate = 4.0 * static_cast<double>(total_hits) / static_cast<double>(total_points);
        std::cout << "[ANY_SOURCE + BSend] Processes: " << size
                  << "  Points: " << total_points
                  << "  Pi estimate: " << pi_estimate
                  << "  Time: " << max_elapsed << " s\n";
    }

    MPI_Finalize();
    return 0;
}
