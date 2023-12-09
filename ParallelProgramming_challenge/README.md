Project for _Advanced Algorithms and Parallel Programming_ (_Politecnico di Milano_, 2023-2024)

# SMILES coverage
The goal of this challenge is to parallelize an application that generates a dictionary with the _ngrams_ that have the greatest coverage of a given input dataset. <br>

To compile and run the application: <br>

```bash
$ cmake -S . -B build
$ cmake --build build
$ ./scripts/launch.sh ./build/main ./data/molecules.smi output.csv 3
```

The output is stored in `output.csv`.
