#include <iostream>
#include <vector>
#include "alignment.hpp"
using namespace std;

void runRecursive();
void runIterative();
void runSequenceGeneration();
void runSequenceGenerationDisjoint();

int main(){
    cout << "Please select the algorithm you want to run." << endl;
    cout << "1: recurstive alignment" << endl << "2: iterative alignment" << endl;
    cout << "3: sequence generation with all possible letters" << endl;
    cout << "4: sequence generation with disjoint set of letters" << endl;
    cout << "Selection: ";
    int option;
    cin >> option;
    switch(option){
        case 1: runRecursive(); break;
        case 2: runIterative(); break;
        case 3: runSequenceGeneration(); break;
        case 4: runSequenceGenerationDisjoint(); break;
        default: cout << "Invalid option" << endl;
    }
}

/* Run the recursive algorithm to solve the alignment problem */
void runRecursive(){
    // Define costs 
    int cost_g = 2;     // gap cost
    int cost_r = 5;     // replace cost

    // Get x, y from input
    string x = inputString("X");
    string y = inputString("Y");
    string x_aligned, y_aligned;

    // Align
    vector<vector<int>> c_rec = initC(x, y, cost_g, cost_r);
    int cost_rec = alignRecursive(x, y, x.size()-1, y.size()-1, c_rec, cost_g, cost_r);
    pair<string, string> x_y = reconstruct(x, y, c_rec, cost_g, cost_r);
    x_aligned = x_y.first;
    y_aligned = x_y.first;

    // Print results
    cout << endl << "Gap cost: " << cost_g << ". Replacement cost: " << cost_r << endl;
    cout << "Aligned X: " << x_aligned << endl;
    cout << "Aligned Y: " << y_aligned << endl;
    cout << "Alignment cost: " << cost_rec << endl;
}

/* Run the iterative algorithm to solve the alignment problem */
void runIterative(){
    // Define costs 
    int cost_g = 2;     // gap cost
    int cost_r = 5;     // replace cost

    // Get x, y from input
    string x = inputString("X");
    string y = inputString("Y");
    string x_aligned, y_aligned;

    // Align
    pair<vector<vector<int>>, pair<string, string>> res = alignIterative(x, y, cost_g, cost_r);
    vector<vector<int>> c = res.first;
    int cost = c[c.size()-1][c[0].size()-1];
    x_aligned = res.second.first;
    y_aligned = res.second.second;

    // Print results
    cout << endl <<"Gap cost: " << cost_g << ". Replacement cost: " << cost_r << endl;
    cout << "Aligned X: " << x_aligned << endl;
    cout << "Aligned Y: " << y_aligned << endl;
    cout << "Alignment cost: " << cost << endl;
    
}

/* Generate two strings of maximum cost, with each letter when possible */
void runSequenceGeneration(){
    // Define costs 
    int cost_g = getInt("gap cost");            // gap cost
    int cost_r = getInt("replacement cost");    // replace cost
    int n = getInt("n");    // legth of first sequence
    int m = getInt("m");    // length of second sequence

    // Generate sequences
    pair<string, string> p = generateSeq(n, m);
    string x = p.first;
    string y = p.second;
    cout << endl << "Sequence X: " << x << endl;
    cout << "Sequence Y: " << y << endl;

    // Align
    pair<vector<vector<int>>, pair<string, string>> res = alignIterative(x, y, cost_g, cost_r);
    vector<vector<int>> c = res.first;
    int cost = c[c.size()-1][c[0].size()-1];
    string x_aligned = res.second.first;
    string y_aligned = res.second.second;

    // Print results of alignment
    cout << endl << "Aligned X: " << x_aligned << endl;
    cout << "Aligned Y: " << y_aligned << endl;
    cout << "Alignment cost: " << cost << endl;
}

/* Generate two strings of maximum cost, using different letters in each string */
void runSequenceGenerationDisjoint(){
    // Define costs 
    int cost_g = getInt("gap cost");            // gap cost
    int cost_r = getInt("replacement cost");    // replace cost
    int n = getInt("n");    // legth of first sequence
    int m = getInt("m");    // length of second sequence

    // Generate sequences
    pair<string, string> p = generateSeqDisj(n, m);
    string x = p.first;
    string y = p.second;
    cout << endl << "Sequence X: " << x << endl;
    cout << "Sequence Y: " << y << endl;
    
    // Align
    pair<vector<vector<int>>, pair<string, string>> res = alignIterative(x, y, cost_g, cost_r);
    vector<vector<int>> c = res.first;
    int cost = c[c.size()-1][c[0].size()-1];
    string x_aligned = res.second.first;
    string y_aligned = res.second.second;

    // Print results of alignment
    cout << endl << "Aligned X: " << x_aligned << endl;
    cout << "Aligned Y: " << y_aligned << endl;
    cout << "Alignment cost: " << cost << endl;
}