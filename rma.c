#include <stdio.h>
#include "mpi.h"

#define SIZE 10
#define USERID 28

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int v1[SIZE];

    // Initialize v1
    for (int i = 0; i < SIZE; i++) {
        if (rank == 0)
            v1[i] = USERID + i; // Initial values for rank 0
        else if (rank == 1)
            v1[i] = (USERID - i) * (-1); // Initial values for rank 1
    }

    // Print initial vector values in both processes
    for (int i = 0; i < SIZE; i++) 
        printf("[%d/%d] index %d value %d\n", rank, size, i, v1[i]);

    // Create a memory window for RMA
    MPI_Win win;
    MPI_Win_create(v1, SIZE * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

    // Data exchange using RMA
    if (rank == 0) {
        MPI_Win_fence(0, win); // Synchronize to read data from rank 1

        // Get odd-index values from rank 1
        for (int i = 1; i < SIZE; i += 2) {
            int temp;
            MPI_Get(&temp, 1, MPI_INT, 1, i, 1, MPI_INT, win);
            v1[i] = temp; // Update odd-index values in rank 0
        }

        MPI_Win_fence(0, win); // Final synchronization
    } else if (rank == 1) {
        MPI_Win_fence(0, win); // Synchronize for rank 0 to read data
        MPI_Win_fence(0, win); // Final synchronization
    }

    // Print final vector in the specified format in rank 0
    if (rank == 0) {
        printf("Vector in rank 0: {");
        for (int i = 0; i < SIZE; i++) {
            if (i % 2 == 0) {
                printf("value_%d_rank0", v1[i]);
            } else {
                printf("value_%d_rank1", v1[i]);
            }
            if (i < SIZE - 1) printf(", ");
        }
        printf("}\n");
    }

    // Free the memory window
    MPI_Win_free(&win);
    MPI_Finalize();
    return 0;
}
