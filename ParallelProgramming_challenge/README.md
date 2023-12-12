Project for _Advanced Algorithms and Parallel Programming_ (_Politecnico di Milano_, 2023-2024)

# SMILES coverage
In the chemistry domain, it is possible to represent a molecule in different ways. The SMILES notation is a compact notation used to store only the molecule's topology in a single line.
Molecule example: _C(=O)C@HNC(=O)OCCOC_<br>
The target application computes a dictionary with the ngrams that have the greatest coverage of a reference input.<br>
For example, this is a possible output for a dictionary with 6 elements and ngrams with maximum of 3 chraracters:<br>

| NGRAM	| COVERAGE |
|---|---|
(=O | 6
=O) | 6
C(= | 6
C | 6
O | 4
(= | 4

The goal of this challenge is to parallelize, using MPI, an application that generates a dictionary with the _ngrams_ that have the greatest coverage of a given input dataset. <br>

To compile and run the application: <br>

```bash
$ cmake -S . -B build
$ cmake --build build
$ ./scripts/launch.sh ./build/main ./data/molecules.smi output.csv 3
```

The output is stored in `output.csv`.
