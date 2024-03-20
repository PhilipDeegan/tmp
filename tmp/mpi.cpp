#include <mpi.h>

#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int NoOfProcess, ProcessNo, tagUB, err;

    MPI_Comm_rank(MPI_COMM_WORLD, &ProcessNo);

    std::cout << ProcessNo << std::endl;

    MPI_Comm_size(MPI_COMM_WORLD, &NoOfProcess);

    std::cout << ProcessNo << std::endl;

    MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_TAG_UB, &tagUB, &err);

    std::cout << ProcessNo << " among " << NoOfProcess
              << " and tagUb = " << tagUB << " err = " << err << "\n";

    MPI_Finalize();
}