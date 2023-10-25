#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

/* Define costs */
int cost_g = 2;     // gap cost
int cost_r = 5;     // replace cost

/* Functions */
string inputString(string name);
string toUpperCase(string &s);
vector<vector<int>> initC(string x, string y);
void printC(vector<vector<int>> c, string x, string y);
int alignRecursive(string x, string y, int i, int j, vector<vector<int>> &c);
pair<string, string> reconstruct(string x, string y, vector<vector<int>> c);
pair<vector<vector<int>>, pair<string, string>> alignIterative(string x, string y);


int main(){
    // Get x, y from input
    string x = inputString("X");
    string y = inputString("Y");
    string x_aligned, y_aligned;

    /* Recursive approach */
    vector<vector<int>> c_rec = initC(x, y);
    int cost_rec = alignRecursive(x, y, x.size()-1, y.size()-1, c_rec);
    pair<string, string> x_y = reconstruct(x, y, c_rec);
    x_aligned = x_y.first;
    y_aligned = x_y.first;

    // Print results
    printC(c_rec, x, y);
    cout << "Aligned X: " << x_aligned << endl;
    cout << "Aligned Y: " << y_aligned << endl;
    cout << "Alignment cost: " << cost_rec << endl;

    /* Iterative approach */
    pair<vector<vector<int>>, pair<string, string>> res = alignIterative(x, y);
    vector<vector<int>> c = res.first;
    int cost = c[c.size()-1][c[0].size()-1];
    x_aligned = res.second.first;
    y_aligned = res.second.second;

    // Print results
    printC(c, x, y);
    cout << "Aligned X: " << x_aligned << endl;
    cout << "Aligned Y: " << y_aligned << endl;
    cout << "Alignment cost: " << cost << endl;
}


/* get string from input */
string inputString(string name){
    string s;
    bool correct = true;
    cout << "Insert " << name << ": ";
    cin >> s;
    toUpperCase(s);
    for(int i=0; i<s.length(); i++){
        if(s[i]!='A' && s[i]!='G' && s[i]!='C' && s[i]!='T'){
            correct = false;
        }
    }
    while(!correct){
        correct = true;
        cout << "Invalid sequence, accepted letters: 'A', 'G', 'C', 'T'. "<< endl << "Insert " << name << ": ";
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

/* Converts a string to upper case */
string toUpperCase(string &s){
    for(char &c: s){
        c = toupper(c);
    }
    return s;
}

/* Init matrix c */
vector<vector<int>> initC(string x, string y){
    vector<vector<int>> c (x.length()+1, vector<int>(y.size()+1, -1));
    for(int i=0; i<c.size(); i++){
        c[i][0] = i*2;
    }
    for(int j=0; j<c[0].size(); j++){
        c[0][j] = j*2;
    }
    return c;
}

/* Print matrix c */
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

/* Recursively compute minimum alignment cost */
int alignRecursive(string x, string y, int i, int j, vector<vector<int>> &c){
    if(c[i+1][j+1]==-1){
        if(x[i]==y[j]){
            c[i+1][j+1] = alignRecursive(x, y, i-1, j-1, c);
        } else {
            int u1 = alignRecursive(x, y, i-1, j, c) + cost_g;
            int u2 = alignRecursive(x, y, i, j-1, c) + cost_g;
            int s1 = alignRecursive(x, y, i-1, j-1, c) + cost_r*2;
            c[i+1][j+1] = min({u1, u2, s1});
        }
    }
    return c[i+1][j+1];
}

/* Reconstruct aligned strings */
pair<string, string> reconstruct(string x, string y, vector<vector<int>> c){
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

/* Align strings iteratively */
pair<vector<vector<int>>, pair<string, string>> alignIterative(string x, string y){
    vector<vector<int>> c = initC(x, y);

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
    pair<string, string> res = reconstruct(x, y, c);
    return make_pair(c, res);
}