#include <mpi.h>
#include <stdio.h>

#define SIZE 100000

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("Rank %d is reported\n", rank);

    if (rank == 0) {
        printf("Rank size: %d\n", size);
    }

    int v1[SIZE];
    int v2[SIZE];

    // Initialize data for demonstration
    for (int i = 0; i < SIZE; i++) {
        v1[i] = rank;  // Each rank initializes v1 with its rank ID
    }

    // Define the partner rank
    int partner = (rank == 0) ? 1 : 0;

    // Use MPI_Sendrecv to combine send and receive in one call
    MPI_Sendrecv(
        v1, SIZE, MPI_INT, partner, 100,       // Send parameters
        v2, SIZE, MPI_INT, partner, 100,       // Receive parameters
        MPI_COMM_WORLD, MPI_STATUS_IGNORE     // Communicator and status
    );

    // Print the result of the communication
    printf("Rank %d successfully sent data to and received data from Rank %d. First element of received data: %d\n", rank, partner, v2[0]);

    MPI_Finalize();
    return 0;
}
