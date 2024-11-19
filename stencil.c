#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ROWS 8
#define COLS 8

void print_grid(int rank, double grid[ROWS][COLS]) {
    printf("Rank %d grid:\n", rank);
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%5.1f ", grid[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 4) {
        if (rank == 0) {
            fprintf(stderr, "This program requires exactly 4 processes.\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // Initialize the grid for each rank
    double grid[ROWS][COLS];
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            grid[i][j] = rank; // Fill grid with rank ID
        }
    }

    // Print the initialized grid
    MPI_Barrier(MPI_COMM_WORLD);
    print_grid(rank, grid);

    // Halo exchange
    double top_halo[COLS], bottom_halo[COLS];
    MPI_Request requests[4];
    if (rank > 0) { // Send top row to the rank above and receive its bottom row
        MPI_Isend(&grid[0][0], COLS, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &requests[0]);
        MPI_Irecv(top_halo, COLS, MPI_DOUBLE, rank - 1, 1, MPI_COMM_WORLD, &requests[1]);
    }
    if (rank < size - 1) { // Send bottom row to the rank below and receive its top row
        MPI_Isend(&grid[ROWS - 1][0], COLS, MPI_DOUBLE, rank + 1, 1, MPI_COMM_WORLD, &requests[2]);
        MPI_Irecv(bottom_halo, COLS, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &requests[3]);
    }

    // Wait for all halo exchanges to complete
    if (rank > 0) MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
    if (rank < size - 1) MPI_Waitall(2, &requests[2], MPI_STATUSES_IGNORE);

    // Apply halos to the grid
    if (rank > 0) {
        for (int j = 0; j < COLS; j++) {
            grid[0][j] = top_halo[j];
        }
    }
    if (rank < size - 1) {
        for (int j = 0; j < COLS; j++) {
            grid[ROWS - 1][j] = bottom_halo[j];
        }
    }

    // Print the grid after halo exchange
    MPI_Barrier(MPI_COMM_WORLD);
    print_grid(rank, grid);

    // Perform stencil computation
    double new_grid[ROWS][COLS];
    for (int i = 1; i < ROWS - 1; i++) {
        for (int j = 1; j < COLS - 1; j++) {
            new_grid[i][j] = (grid[i - 1][j] + grid[i + 1][j] + grid[i][j - 1] + grid[i][j + 1]) / 4.0;
        }
    }

    // Update the grid with new values
    for (int i = 1; i < ROWS - 1; i++) {
        for (int j = 1; j < COLS - 1; j++) {
            grid[i][j] = new_grid[i][j];
        }
    }

    // Print the grid after stencil computation
    MPI_Barrier(MPI_COMM_WORLD);
    print_grid(rank, grid);

    MPI_Finalize();
    return 0;
}
