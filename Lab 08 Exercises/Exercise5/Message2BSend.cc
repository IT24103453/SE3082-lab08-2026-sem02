// Message2BSend.cc
// Exercise 5.2 - "Rewrite message2.cc using Buffered Send (BSend) ensure
// that you do not replace the variables that you are using to send the
// data."
//
// Original message2.cc reuses ONE variable `number` across 3 iterations:
// it sets number = i*10, sends it, then immediately overwrites it on the
// next loop iteration. With plain MPI_Send this happens to be safe
// because MPI_Send is blocking - it does not return until it is safe to
// reuse the send buffer.
//
// MPI_Bsend behaves differently: it copies the data into a user-supplied
// buffer (attached with MPI_Buffer_attach) and returns immediately,
// *before* the message is necessarily delivered. Once MPI_Bsend returns,
// though, the copy has already happened, so - like MPI_Send - it is
// already safe to overwrite `number` right away. The important part of
// this exercise is making sure the ATTACHED BUFFER is large enough to
// hold all in-flight messages (each needs its payload size plus
// MPI_BSEND_OVERHEAD bytes); if it isn't, MPI_Bsend aborts the program.
//
// Build: mpicxx -O2 Message2BSend.cc -o Message2BSend
// Run:   mpirun --oversubscribe -np 2 ./Message2BSend

#include <mpi.h>
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    const int NUM_MESSAGES = 3;

    if (rank == 0) {
        // Buffer big enough for NUM_MESSAGES in-flight int messages.
        int buffer_size = NUM_MESSAGES * (sizeof(int) + MPI_BSEND_OVERHEAD);
        char* buffer = new char[buffer_size];
        MPI_Buffer_attach(buffer, buffer_size);
    }

    int number;
    for (int i = 0; i < NUM_MESSAGES; i++) {
        if (rank == 0) {
            number = i * 10;
            MPI_Bsend(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            std::cout << "Process 0 sent " << number << " (buffered)\n";
            // Safe to overwrite `number` on the next loop iteration:
            // MPI_Bsend already copied it into our attached buffer.
        } else if (rank == 1) {
            MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << "Process 1 received " << number << "\n";
        }
    }

    if (rank == 0) {
        void* buf_ptr;
        int buf_size;
        // Blocks until all buffered sends have actually been delivered,
        // then hands the buffer back to us to free.
        MPI_Buffer_detach(&buf_ptr, &buf_size);
        delete[] static_cast<char*>(buf_ptr);
    }

    MPI_Finalize();
    return 0;
}
