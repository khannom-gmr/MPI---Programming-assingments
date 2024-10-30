#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>

void read_matrix(int* matrix, int n) {
    // Ejemplo de lectura de matriz en proceso 0 (esto se podría modificar para lectura real)
    for (int i = 0; i < n * n; i++) {
        matrix[i] = i + 1;  // Elementos de ejemplo, se reemplazarían con una lectura real
    }
}

void read_vector(int* vector, int n) {
    // Ejemplo de lectura de vector en proceso 0
    for (int i = 0; i < n; i++) {
        vector[i] = 1;  // Elementos de ejemplo, se reemplazarían con una lectura real
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int comm_sz, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Tamaño de la matriz y la distribución
    int n = 4; // Asumimos una matriz n x n para el ejemplo, divisible por raíz de comm_sz
    int q = std::sqrt(comm_sz); // Número de procesos por lado de la cuadrícula
    int block_size = n / q; // Tamaño de cada bloque

    // Buffer para almacenar la matriz y el vector en el proceso 0
    std::vector<int> matrix;
    std::vector<int> vector;
    if (my_rank == 0) {
        matrix.resize(n * n);
        vector.resize(n);
        read_matrix(matrix.data(), n);
        read_vector(vector.data(), n);
    }

    // Dividir el vector para procesos diagonales
    std::vector<int> local_vector(block_size);
    if (my_rank % (q + 1) == 0) { // Solo procesos en la diagonal reciben su parte del vector
        MPI_Scatter(vector.data(), block_size, MPI_INT, local_vector.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // Cada proceso recibe su bloque de la matriz
    std::vector<int> local_matrix(block_size * block_size);
    if (my_rank == 0) {
        for (int i = 0; i < comm_sz; i++) {
            int row = i / q;
            int col = i % q;
            if (i == 0) {
                for (int j = 0; j < block_size; j++) {
                    for (int k = 0; k < block_size; k++) {
                        local_matrix[j * block_size + k] = matrix[(row * block_size + j) * n + (col * block_size + k)];
                    }
                }
            } else {
                std::vector<int> temp_block(block_size * block_size);
                for (int j = 0; j < block_size; j++) {
                    for (int k = 0; k < block_size; k++) {
                        temp_block[j * block_size + k] = matrix[(row * block_size + j) * n + (col * block_size + k)];
                    }
                }
                MPI_Send(temp_block.data(), block_size * block_size, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
    } else {
        MPI_Recv(local_matrix.data(), block_size * block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Cada proceso multiplica su bloque local de la matriz por el vector parcial
    std::vector<int> local_result(block_size, 0);
    for (int i = 0; i < block_size; i++) {
        for (int j = 0; j < block_size; j++) {
            local_result[i] += local_matrix[i * block_size + j] * local_vector[j];
        }
    }

    // Recopilar los resultados de cada proceso en el proceso 0
    std::vector<int> result;
    if (my_rank == 0) result.resize(n);

    MPI_Gather(local_result.data(), block_size, MPI_INT, result.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Mostrar el resultado en el proceso 0
    if (my_rank == 0) {
        std::cout << "Resultado de la multiplicación matriz-vector:\n";
        for (int i = 0; i < n; i++) {
            std::cout << result[i] << " ";
        }
        std::cout << std::endl;
    }

    MPI_Finalize();
    return 0;
}
