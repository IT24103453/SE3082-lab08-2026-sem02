// SourceMismatch.cc
// Exercise 5.1 - "Write a simple program where the source and the
// destination doesn't match. What happens in this situation?"
//
// This is message1.cc, modified so that the receiver listens for a
// message from the WRONG source rank.
//   rank 0  -> MPI_Send(..., dest = 1, tag = 0, ...)
//   rank 1  -> MPI_Recv(..., source = 2, tag = 0, ...)   <-- mismatch!
//
// Run with 3 processes: mpirun --oversubscribe -np 3 ./SourceMismatch
//
// WHAT HAPPENS:
// Rank 0 sends its message to rank 1 as intended. But rank 1's receive
// is only willing to match a message whose *source* is rank 2 - and
// rank 2 never sends anything to rank 1. Because MPI_Recv only matches
// messages against the exact (source, tag, comm) it was given (unless
// MPI_ANY_SOURCE / MPI_ANY_TAG is used), rank 1 blocks forever waiting
// for a message that will never arrive. Rank 0's send may or may not
// return (depending on whether MPI buffers it internally), but the
// program as a whole DEADLOCKS: it never reaches MPI_Finalize and has
// to be killed manually (Ctrl+C, or it will simply hang until a batch
// scheduler kills it).
//
// Build: mpicxx -O2 SourceMismatch.cc -o SourceMismatch
// Run (will hang - kill with Ctrl+C, or run under `timeout`):
//   timeout 5 mpirun --oversubscribe -np 3 ./SourceMismatch

#include <mpi.h>
#include <iostream>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 3) {
        if (rank == 0)
            std::cerr << "Run with at least 3 processes: -np 3\n";
        MPI_Finalize();
        return 0;
    }

    int number;
    if (rank == 0) {
        number = 42;
        std::cout << "Process 0 sending " << number << " to process 1...\n";
        MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        std::cout << "Process 0: send returned.\n";
    } else if (rank == 1) {
        std::cout << "Process 1 waiting for a message that claims to be "
                     "from source=2 (mismatch - process 0 is the real sender)...\n";
        // BUG: this will never be satisfied, because process 2 never sends
        // anything with this tag. Process 0's message (source=0) does not
        // match a receive posted for source=2, so it just sits unmatched.
        MPI_Recv(&number, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process 1 received " << number << " (unreachable line)\n";
    } else {
        std::cout << "Process " << rank << " has nothing to do and exits immediately.\n";
    }

    MPI_Finalize();
    return 0;
}
