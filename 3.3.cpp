#include <mpi.h>
#include <iostream>

int main(int argc, char* argv[]) {
    int comm_sz, my_rank;

    // Inicializar el entorno MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int local_sum = my_rank;

    // Si comm_sz no es potencia de dos, los rangos más altos envían sus datos a rangos más bajos
    int next_power_of_two = 1;
    while (next_power_of_two < comm_sz) {
        next_power_of_two *= 2;
    }

    if (my_rank >= next_power_of_two / 2) {
        int partner = my_rank - (next_power_of_two / 2);
        MPI_Send(&local_sum, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
    } else if (my_rank < comm_sz - (next_power_of_two / 2)) {
        int partner = my_rank + (next_power_of_two / 2);
        int received_value;
        MPI_Recv(&received_value, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        local_sum += received_value;
    }

    // Suma en estructura de árbol (como en el caso de potencia de dos)
    int partner;
    for (int step = 1; step < next_power_of_two / 2; step *= 2) {
        if (my_rank % (2 * step) == 0) {
            partner = my_rank + step;
            int received_value;
            MPI_Recv(&received_value, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            local_sum += received_value;
        } else if (my_rank % step == 0) {
            partner = my_rank - step;
            MPI_Send(&local_sum, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
            break;
        }
    }

    if (my_rank == 0) {
        std::cout << "Suma global: " << local_sum << std::endl;
    }

    MPI_Finalize();
    return 0;
}
