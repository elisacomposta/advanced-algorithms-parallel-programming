#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <mpi.h>
#include "mpi_error_check.hpp"
using namespace std;

// set the maximum size of the ngram
static constexpr size_t max_pattern_len = 3;
static constexpr size_t max_dictionary_size = 100000000000;
static_assert(max_pattern_len > 1, "The pattern must contain at least one character");
static_assert(max_dictionary_size > 1, "The dictionary must contain at least one element");

struct word {
    char ngram[max_pattern_len + 1];  // the string data, statically allocated
    size_t size = 0;                  // the string size
    size_t coverage = 0;              // the score of the word
};

// utility struct to ease working with words
struct word_coverage_lt_comparator {
    bool operator()(const word &w1, const word &w2) const { return w1.coverage < w2.coverage; }
};
struct word_coverage_gt_comparator {
    bool operator()(const word &w1, const word &w2) const { return w1.coverage > w2.coverage; }
};

// dictionary of ngrams
struct dictionary {
    vector<word> data;
    vector<word>::iterator worst_element;

    void add_word(word &new_word) {
        const auto coverage = new_word.coverage;
        if (data.size() < max_dictionary_size) {
            data.emplace_back(move(new_word));
            worst_element = end(data);
        } else {
            if (worst_element == end(data)) {
                worst_element = min_element(begin(data), end(data), word_coverage_lt_comparator{});
            }
            if (coverage > worst_element->coverage) {
                *worst_element = move(new_word);
                worst_element = end(data);
            }
        }
    }

    void write(ostream &out) const {
        for (const auto &word : data) {
            out << word.ngram << ' ' << word.coverage << endl;
        }
        out << flush;
    }
};

size_t count_coverage(const string &dataset, const char *ngram) {
    size_t index = 0;
    size_t counter = 0;
    const size_t ngram_size = strlen(ngram);
    if(ngram_size==0) return -1;
    while (index < dataset.size()) {
        index = dataset.find(ngram, index);
        if (index != string::npos) {
        ++counter;
        index += ngram_size;
        }
    }
    return counter * ngram_size;
}


int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {

    // initialize the MPI environment
    int provided_thread_level;
    const int rc_init = MPI_Init_thread(&argc, &argv, MPI_THREAD_SINGLE, &provided_thread_level);
    exit_on_fail(rc_init);
    if (provided_thread_level < MPI_THREAD_SINGLE) {
        cerr << "Minimum thread level not available!\n";
        return EXIT_FAILURE;
    }

    // get size and rank
    int world_size, world_rank;
    const int rc_size = MPI_Comm_size(MPI_COMM_WORLD , &world_size);
    exit_on_fail(rc_size);
    const int rc_rank = MPI_Comm_rank(MPI_COMM_WORLD , &world_rank);
    exit_on_fail(rc_rank);
    
    // read database of SMILES and put them in a single string
    unordered_set<char> alphabet_builder;
    string database;
    database.reserve(209715200);  // 200MB
    for (string line; getline(cin, line); ) {
        for (const auto character : line) {
            alphabet_builder.emplace(character);
            database.push_back(character);
        }
    }
    
    int database_size = database.size();
    int rc_br = MPI_Bcast(&database_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    exit_on_fail(rc_br);

    database.resize(database_size);
    rc_br = MPI_Bcast(database.data(), database_size, MPI_CHAR, 0, MPI_COMM_WORLD);
    exit_on_fail(rc_br);   

    // put the alphabet in a container with random access capabilities
    vector<char> alphabet;
    alphabet_builder.reserve(alphabet_builder.size());
    for_each(begin(alphabet_builder), end(alphabet_builder), [&alphabet](const auto character) {alphabet.push_back(character); });

    // precompute the number of permutations according to the number of characters
    auto permutations = vector(max_pattern_len, alphabet.size());
    int n_permutations = alphabet.size();

    for (size_t i{1}; i < permutations.size(); ++i) {
        permutations[i] = alphabet.size() * permutations[i - size_t{1}];
        n_permutations += permutations[i];
    }

    // map permutation string to word
    unordered_map<string, word> string_to_word;
    vector<string> stringData(n_permutations);
    int i_str = 0;

    // this outer loop goes through the n-gram with different sizes
    for (size_t ngram_size{1}; ngram_size <= max_pattern_len; ++ngram_size) {
        // this loop goes through all the permutation of the current ngram-size
        const auto num_words = permutations[ngram_size - size_t{1}];
        for (size_t word_index{0}; word_index < num_words; ++word_index) {  // only P0 in here
            // compose the ngram
            word current_word;
            memset(current_word.ngram, '\0', max_pattern_len + 1);
            for (size_t character_index{0}, remaining_size = word_index; character_index < ngram_size;
                ++character_index, remaining_size /= alphabet.size()) {
                current_word.ngram[character_index] = alphabet[remaining_size % alphabet.size()];
            }
            current_word.size = ngram_size;

            string_to_word[current_word.ngram] = current_word;
            stringData[i_str++] = current_word.ngram;
        }
    }
    if(world_rank==0) cerr << "P0 built " << stringData.size() << " permutations" << endl;

    // here, all permutations have been computed (by P0 only)
    rc_br = MPI_Bcast(&n_permutations, 1, MPI_INT, 0, MPI_COMM_WORLD);
    exit_on_fail(rc_br);

    const int n_data = n_permutations;

    int dataPerProcess = n_data / world_size;
    int totalData = n_data;
    if(n_data % world_size != 0){
        totalData = (dataPerProcess+1) * world_size; 
        dataPerProcess = totalData / world_size;  
        for(int i=n_data; i<totalData; i++){
            stringData.push_back("");
        }
    }

    random_shuffle(stringData.begin(), stringData.end());

    // put all permutations on a single vector of char
    const int charPerProcess = dataPerProcess * (max_pattern_len+1);
    vector<char> flatData;
    flatData.reserve(charPerProcess * world_size);
    if(world_rank==0){
        for(string str: stringData){
            for(char c: str){   // copy str to char vector
                flatData.push_back(c);
            }
            for(int i=0; i<int(max_pattern_len+1 - str.size()); i++){ 
                flatData.push_back('\0');   // fill with '\0'
            }
        }
    }

    // buffer to store data to process
    vector<char> localData(charPerProcess);

    // SCATTER DATA
    const int rc_scatter = MPI_Scatter(flatData.data(), charPerProcess, MPI_CHAR, localData.data(), charPerProcess, MPI_CHAR, 0, MPI_COMM_WORLD);
    exit_on_fail(rc_scatter);

    // extract strings to process
    vector<string> dataToProcess;
    dataToProcess.reserve(dataPerProcess);
    for(int i=0; i<dataPerProcess; i++){
        string currentString = "";
        for(int j=0; j<int(max_pattern_len); j++){
            char c = localData[i*(max_pattern_len+1)+j];
            currentString.push_back(c);
        }
        dataToProcess.push_back(currentString);
    }

    // COMPUTE COVERAGE
    vector<int> computedCoverage;
    computedCoverage.reserve(dataPerProcess);
    for(string s: dataToProcess){
        int coverage = count_coverage(database, s.c_str());
        computedCoverage.push_back(coverage);
    }    
    cerr << "P" << world_rank << " computed the coverage of " << computedCoverage.size() << " words" << endl;

    // GATHER DATA
    vector<int> gatheredData(totalData, -1);
    int rc_gather = MPI_Gather(computedCoverage.data(), dataPerProcess, MPI_INT,
            gatheredData.data(), dataPerProcess, MPI_INT, 0, MPI_COMM_WORLD);
    exit_on_fail(rc_gather);
    if(world_rank==0) cerr << "P0 gathered all the results" << endl;

    // OUTPUT RESULTS
    //    string_to_word: map permutation to word
    //    gatheredData: vector of (shuffled) coverages
    //    stringData: vector of (shuffled) strings
    if(world_rank==0){
        dictionary resultDict;
        for(int i=0; i<totalData; i++){
            string str = stringData[i];
            int cov = gatheredData[i];
            if(str.size() > 0){     // valid ngram
                word current_word = string_to_word[str];
                current_word.coverage = cov;
                resultDict.add_word(current_word);
            }
        }

        cout << "NGRAM COVERAGE" << endl;
        sort(begin(resultDict.data), end(resultDict.data), word_coverage_gt_comparator{});
        resultDict.write(cout);
        cerr << "Results stored in output.csv" << endl;
    }

    cerr << "P" << world_rank << " finalizing.." << endl;

    // clear the MPI environment
    const int rc_finalize = MPI_Finalize();
    exit_on_fail(rc_finalize);
    return EXIT_SUCCESS;
}
