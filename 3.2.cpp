#include <iostream>
#include <cstdlib>
#include <cmath>
#include <mpi.h>
#include <ctime>  // time()

int main(int argc, char** argv) {
    long long int number_of_tosses, local_number_of_tosses;
    long long int number_in_circle = 0, local_number_in_circle = 0;
    double x, y, distance_squared;
    int rank, size, toss;

    // Inicializa MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // El proceso 0 lee el número de lanzamientos
    if (rank == 0) {
        std::cout << "número total de lanzamientos: ";
        std::cin >> number_of_tosses;
    }

    // Difundir el número total de lanzamientos a todos los procesos
    MPI_Bcast(&number_of_tosses, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);

    // Cada proceso realiza una cantidad local de lanzamientos
    local_number_of_tosses = number_of_tosses / size;

    // Semilla diferente para cada proceso
    srand(time(nullptr) + rank); // Inicializa la semilla de rand()

    // Generar lanzamientos aleatorios y contar los que caen dentro del círculo
    for (toss = 0; toss < local_number_of_tosses; toss++) {
        // Generar coordenadas aleatorias
        x = static_cast<double>(rand()) / RAND_MAX * 2.0 - 1.0;
        y = static_cast<double>(rand()) / RAND_MAX * 2.0 - 1.0;
        distance_squared = x * x + y * y;

        if (distance_squared <= 1.0) {
            local_number_in_circle++;
        }
    }

    // Reducir los resultados locales en el proceso 0 para obtener el número total en el círculo
    MPI_Reduce(&local_number_in_circle, &number_in_circle, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // El proceso 0 calcula y muestra el valor estimado de pi
    if (rank == 0) {
        double pi_estimate = 4.0 * static_cast<double>(number_in_circle) / static_cast<double>(number_of_tosses);
        std::cout << "pi = " << pi_estimate << std::endl;
    }

    // Finaliza MPI
    MPI_Finalize();
    return 0;
}