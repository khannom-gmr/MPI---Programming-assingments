#include <iostream>
#include <cstdlib>
#include <mpi.h>

using namespace std;

void generate_data(int data[], int size, int max_value) {
    for (int i = 0; i < size; i++) {
        data[i] = rand() % max_value; // números aleatorios
    }
}

int main(int argc, char** argv) {
    int num_elements = 1000;  // Tamaño total del conjunto de datos
    int num_bins = 10;        // Número de bins del histograma
    int max_value = 100;      // Máximo valor en los datos
    int rank, size;

    // Inicializa MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int* data = nullptr;
    int* local_data = nullptr;
    int* histogram = nullptr;
    int* local_histogram = nullptr;

    if (rank == 0) {
        // Proceso 0 genera datos de entrada
        data = new int[num_elements];
        generate_data(data, num_elements, max_value);

        // Inicializa el histograma global
        histogram = new int[num_bins]();
    }

    // Cantidad de datos que cada proceso manejará
    int local_size = num_elements / size;

    // Cada proceso tiene su propio conjunto de datos local
    local_data = new int[local_size];
    local_histogram = new int[num_bins]();

    // Distribuye los datos a los procesos
    MPI_Scatter(data, local_size, MPI_INT, local_data, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Cada proceso calcula su histograma local
    for (int i = 0; i < local_size; i++) {
        int bin = local_data[i] * num_bins / max_value;
        local_histogram[bin]++;
    }

    // Reduce todos los histogramas locales al proceso 0
    MPI_Reduce(local_histogram, histogram, num_bins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // El proceso 0 imprime el histograma final
    if (rank == 0) {
        cout << "Histograma final:" << endl;
        for (int i = 0; i < num_bins; i++) {
            cout << "Bin " << i << ": " << histogram[i] << endl;
        }
    }

    // Liberar memoria
    if (rank == 0) {
        delete[] data;
        delete[] histogram;
    }
    delete[] local_data;
    delete[] local_histogram;

    // Finaliza MPI
    MPI_Finalize();

    return 0;
}