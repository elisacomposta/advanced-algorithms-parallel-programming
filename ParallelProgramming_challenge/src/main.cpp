#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <queue>
#include <mpi.h>
#include "mpi_error_check.hpp"
using namespace std;

// define tags
const int tag_result = 0;
const int tag_data = 1;
const int tag_end_job = 2;

// set the maximum size of the ngram
static constexpr size_t max_pattern_len = 3;
static constexpr size_t max_dictionary_size = 100000000000;
static_assert(max_pattern_len > 1, "The pattern must contain at least one character");
static_assert(max_dictionary_size > 1, "The dictionary must contain at least one element");

// simple class to represent a word of our dictionary
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

// this is our dictionary of ngram
struct dictionary {
    std::vector<word> data;
    std::vector<word>::iterator worst_element;

    void add_word(word &new_word) {
        const auto coverage = new_word.coverage;
        if (data.size() < max_dictionary_size) {
            data.emplace_back(std::move(new_word));
            worst_element = std::end(data);
        } else {
            if (worst_element == std::end(data)) {
                worst_element = std::min_element(std::begin(data), std::end(data), word_coverage_lt_comparator{});
            }
            if (coverage > worst_element->coverage) {
                *worst_element = std::move(new_word);
                worst_element = std::end(data);
            }
        }
    }

    void write(std::ostream &out) const {
        for (const auto &word : data) {
            out << word.ngram << ' ' << word.coverage << std::endl;
        }
        out << std::flush;
    }
};

size_t count_coverage(const string &dataset, const char *ngram) {
    size_t index = 0;
    size_t counter = 0;
    const size_t ngram_size = ::strlen(ngram);
    if(int(ngram_size) == 0 ) return -1;
    while (index < dataset.size()) {
        index = dataset.find(ngram, index);
        if (index != std::string::npos) {
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

    // get rank, size info
    int world_size, world_rank;
    const int rc_size = MPI_Comm_size(MPI_COMM_WORLD , &world_size);
    exit_on_fail(rc_size);
    const int rc_rank = MPI_Comm_rank(MPI_COMM_WORLD , &world_rank);
    exit_on_fail(rc_rank);

    /*if(world_size < 2){
        printf("Not enough processes.\n");
        return EXIT_FAILURE;
    }*/
    
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

    // map permutation to word
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
    cerr << "P" << world_rank << " built " << stringData.size() << " permutations" << endl;

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
    
    /*if(world_rank==0){
        cerr << "SHUFFLED DATA: "; 
        for(string s: stringData){
            cerr << s << " ";
        }
        cerr << endl;
    }*/

    // char data for scatter
    vector<char> flatData;
    if(world_rank==0){
        for (int i = 0; i < totalData; i++) {
            for(char c: stringData[i]){
                flatData.push_back(c);
            }
            flatData.push_back('\0');
        }
    }

    
    // BROADCAST DATA TO ELABORATE
    int flat_size = flatData.size();
    rc_br = MPI_Bcast(&flat_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    exit_on_fail(rc_br);

    flatData.resize(flat_size);
    rc_br = MPI_Bcast(flatData.data(), flat_size, MPI_CHAR, 0, MPI_COMM_WORLD);
    exit_on_fail(rc_br);

    // RECEIVE DATA and compute results
    vector<string> dataToProcess;
    int scanned_strings = 0;
    string currentString = "";
    for(int i=0; i<flat_size; i++){
        char c = flatData[i];
        if(c!='\0'){
            currentString.push_back(c);
        } else {
            //stringa completata
            scanned_strings++;
            if(scanned_strings > world_rank * dataPerProcess){
                dataToProcess.push_back(currentString);
                if(int(dataToProcess.size())==dataPerProcess){
                    break;
                }
            }
            currentString = "";
        }
    }

    vector<int> computed_coverage;
    for(string s: dataToProcess){
        int coverage = count_coverage(database, s.c_str());
        computed_coverage.push_back(coverage);
    }    
    cerr << "P" << world_rank << " computed the coverage of " << computed_coverage.size() << " words" << endl;

    // GATHER DATA
    vector<int> gatheredData(totalData);
    int rc_gather = MPI_Gather(computed_coverage.data(), dataPerProcess, MPI_INT,
            gatheredData.data(), dataPerProcess, MPI_INT, 0, MPI_COMM_WORLD);
    exit_on_fail(rc_gather);
    int rc_barrier = MPI_Barrier(MPI_COMM_WORLD);
    exit_on_fail(rc_barrier); 

    if(world_rank==0) cerr << "P0 gathered " << gatheredData.size() << " results" << endl;

    /*if (world_rank == 0) {
        cerr << "Gathered data: ";
        for (int i = 0; i < totalData; i++) {
            cerr << gatheredData[i] << " ";
        }
        cerr << endl;
    }*/

    // string_to_word: map permutation to word
    // gatheredData: vector of (shuffled) coverages
    // stringData: vector of (shuffled) strings

    if(world_rank==0){
        // declare the dictionary that holds all the ngrams with the greatest coverage
        dictionary resultDict;

        for(int i=0; i<totalData; i++){
            string str = stringData[i];
            int cov = gatheredData[i];
            if(str.size() > 0){
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
