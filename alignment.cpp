#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
using namespace std;


/* Convert string to upper case */
void toUpperCase(string &s){
    for(char &c: s){
        c = toupper(c);
    }
}


/* Get valid sequence from input.
 * parameter: name of the input asked to the user
 * return: string of valid letters
 */
string inputString(string name){
    string s;
    bool correct = true;

    // read sequence from input
    cout << "Insert " << name << ": ";
    cin >> s;
    toUpperCase(s);

    // check if valid
    for(int i=0; i<s.length(); i++){
        if(s[i]!='A' && s[i]!='G' && s[i]!='C' && s[i]!='T'){
            correct = false;
        }
    }

    // ask for a new sequence until a valid one is given
    while(!correct){
        correct = true;
        cout << "Invalid sequence. Accepted letters: 'A', 'G', 'C', 'T'. "<< endl << "Insert " << name << ": ";
        cin >> s;
        toUpperCase(s);
        for(int i=0; i<s.length(); i++){
            if(s[i]!='A' && s[i]!='G' && s[i]!='C' && s[i]!='T'){
                correct = false;
            }
        }
    } 
    return s;
}


/* Initialize matrix of costs.
 * parameters: strings, cost of operations
 * return: initialized matrix of costs
 */
vector<vector<int>> initC(string x, string y, int cost_g, int cost_r){
    vector<vector<int>> c (x.length()+1, vector<int>(y.size()+1, -1));
    for(int i=0; i<c.size(); i++){
        c[i][0] = i*cost_g;
    }
    for(int j=0; j<c[0].size(); j++){
        c[0][j] = j*cost_g;
    }
    return c;
}


/* Print matrix c.
 * parameters: matrix c, strings x, y
 */
void printC(vector<vector<int>> c, string x, string y){
    // print first line string
    cout << endl <<  "        ";
    for(int j=0; j<y.size(); j++){
        cout << y[j] << "   ";
    }
    for(int i=0; i<c.size(); i++){
        // print letter of string at the beginning
        i>0? cout << x[i-1] << " " : cout << endl << "  ";
        // print values
        for(int j=0; j<c[0].size(); j++){
            if(c[i][j] >= 0) cout << " "; 
            if(c[i][j]<10) cout << " "; 
            cout << c[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;
}


/* Reconstruct aligned strings.
 * parameters: unaligned strings, matrix of costs, cost of operations
 * return: aligned strings x1, x2 closed in a pair
 */
pair<string, string> reconstruct(string x, string y, vector<vector<int>> c, int cost_g, int cost_r){
    string x1, y1;
    int i = x.size()-1;
    int j = y.size()-1;
    while(i>=0 || j>=0){
        if(i>=0 && j>=0 && x[i] == y[j]){
            x1 = x[i] + x1;
            y1 = y[j] + y1;
            i--;
            j--;
        } else if(j>=0 && c[i+1][j+1]==c[i+1][j]+cost_g){
            x1 = "_" + x1;
            y1 = y[j] + y1;
            j--;
        } else if(i>=0 && c[i+1][j+1]==c[i][j+1]+cost_g){
            x1 = x[i] + x1;
            y1 = "_" + y1;
            i--;
        } else if(i>=0 && j>=0 && c[i+1][j+1]==c[i][j]+cost_r*2){
            x1 = "*" + x1;
            y1 = "*" + y1;
            i--;
            j--;
        }
    }
    return make_pair(x1, y1);
}


/* Recursively compute minimum alignment cost.
 * parameters: strings to align, indices of the strings, matrix of costs, cost of operations
 * return: cost of the corresponding position
 */
int alignRecursive(string x, string y, int i, int j, vector<vector<int>> &c, int cost_g, int cost_r){
    if(c[i+1][j+1]==-1){
        if(x[i]==y[j]){
            c[i+1][j+1] = alignRecursive(x, y, i-1, j-1, c, cost_g, cost_r);
        } else {
            int u1 = alignRecursive(x, y, i-1, j, c, cost_g, cost_r) + cost_g;
            int u2 = alignRecursive(x, y, i, j-1, c, cost_g, cost_r) + cost_g;
            int s1 = alignRecursive(x, y, i-1, j-1, c, cost_g, cost_r) + cost_r*2;
            c[i+1][j+1] = min({u1, u2, s1});
        }
    }
    return c[i+1][j+1];
}


/* Align strings iteratively.
 * parameters: strings to align, cost of operations
 * return: alignment cost, aligned strings (closed in a nested pair)
 */
pair<vector<vector<int>>, pair<string, string>> alignIterative(string x, string y, int cost_g, int cost_r){
    // matrix of costs
    vector<vector<int>> c = initC(x, y, cost_g, cost_r);
    // iteratively fill matrix of costs
    for(int i=1; i<c.size(); i++){
        for(int j=1; j<c[0].size(); j++){
            if(x[i-1]==y[j-1]){
                c[i][j] = c[i-1][j-1];
            } else {
                int gap1 = c[i-1][j] + cost_g;
                int gap2 = c[i][j-1] + cost_g;
                int repl = c[i-1][j-1] + cost_r *2;
                c[i][j] = min({gap1, gap2, repl});
            }
        }
    }
    // aligned strings
    pair<string, string> res = reconstruct(x, y, c, cost_g, cost_r);
    return make_pair(c, res);
}


/* Get int from input */
int getInt(string name){
    int n;
    cout << "Insert " << name << ": ";
    if(!(cin>>n)){
        cout << "Failed. Not a number." << endl;
        exit(-1);
    };
    return n;
}

/* Generate two valid sequences with the maximum cost possible, given their length.
 * If the strings are longer than the number of available letters, they will contain all the letters:
 * Note that given this constraint, at least one letter will be aligned in the two strings.
 * parameters: length of the strings
 * return: strings of given length with maximum cost
 */
pair<string, string> generateSeq(int n, int m){
    string x, y;
    string letters = "ACGT";
    int nLetters = letters.size();
    
    // permutate letters
    srand(time(nullptr));
    random_shuffle(letters.begin(), letters.end());

    // generate x
    for(int i=0; i<n; i++){
        if(i<nLetters){
            x = x + letters[i];     // copy letters in the same order of the permutation
        } else {
            x = x + x[nLetters-1];  // copy last letter until the end
        }
    }

    // generate y
    for(int i=0; i<m; i++){
        if(i<nLetters){
            y = y + letters[nLetters-1-i];  // copy letters in the opposite order of the permutation
        } else {
            y = y + y[nLetters-1];          // copy last letter until the end
        }
    }

    return make_pair(x, y);
}


/* Generate two valid sequences with the maximum cost possible, given their length.
 * The generated strings don't have any letter in common, as this will surely produce
 * the greatest alignment cost.
 * parameters: length of the strings
 * return: strings of given length with maximum cost
 */
pair<string, string> generateSeqDisj(int n, int m){
    string x, y;
    string letters = "ACGT";
    int nLetters = letters.size();
    
    // permutate letters
    srand(time(nullptr));
    random_shuffle(letters.begin(), letters.end());

    // split valid letters into two subsets
    string letters1 = letters.substr(0, nLetters/2);
    string letters2 = letters.substr(nLetters/2, nLetters);

    // generate x with the fist subset of letters
    for(int i=0; i<n; i++){
        x = x + letters1[rand()%2];
    }

    // generate y with the second subset of letters
    for(int i=0; i<m; i++){
        y = y + letters2[rand()%2];
    }

    return make_pair(x, y);
}
