#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

/* Define costs */
int cost_u = 2;
int cost_s = 5;

/* Examples of vectors x and y */
//vector<char> x = {'A', 'E', 'D'};
//vector<char> y = {'A', 'B', 'C', 'D'};

//vector<char> x = {'C', 'G'};
//vector<char> y = {'C', 'A'};

//vector<char> y = {'A', 'G', 'G', 'G', 'C', 'T'};
//vector<char> x = {'A', 'G', 'G', 'C', 'A'};

vector<char> y = {'G', 'G', 'G', 'C', 'T'};
vector<char> x = {'G', 'G', 'C', 'A'};

/* Matrix of costs */
vector<vector<int>> c (x.size()+1, vector<int>(y.size()+1, -1));

/* Functions */
void initC();
void printC();
int align(vector<char> x, vector<char> y, int i, int j);
pair<string, string> reconstruct();


int main(){
    initC();
    int cost = align(x, y, x.size()-1, y.size()-1);
    printC();
    pair<string, string> res = reconstruct();
    cout << "Final X: " << res.first << endl;
    cout << "Final Y: " << res.second << endl;
    cout << "Alignment cost: " << cost << endl;
}

/* Init matrix c */
void initC(){
    for(int i=0; i<c.size(); i++){
        c[i][0] = i*2;
    }
    for(int j=0; j<c[0].size(); j++){
        c[0][j] = j*2;
    }
}

/* Print matrix c */
void printC(){
    for(int i=0; i<c.size(); i++){
        for(int j=0; j<c[0].size(); j++){
            if(c[i][j] >= 0) cout << " "; 
            if(c[i][j]<10) cout << " "; 
            cout << c[i][j] << " ";
        }
        cout << endl;
    }
}

/* Recursively compute minimum alignment cost */
int align(vector<char> x, vector<char> y, int i, int j){
    if(c[i+1][j+1]==-1){
        if(x[i]==y[j]){
            c[i+1][j+1] = align(x, y, i-1, j-1);
        } else {
            int u1 = align(x, y, i-1, j) + cost_u;
            int u2 = align(x, y, i, j-1) + cost_u;
            int s1 = align(x, y, i-1, j-1) + cost_s*2;
            c[i+1][j+1] = min({u1, u2, s1});
        }
    }
    return c[i+1][j+1];
}


/* Reconstruct aligned strings */
pair<string, string> reconstruct(){
    vector<char> str1;
    vector<char> str2;

    int i = x.size()-1;
    int j = y.size()-1;
    while(i>=0 || j>=0){
        if(i>=0 && j>=0 && x[i] == y[j]){
            str1.push_back(x[i]);
            str2.push_back(y[j]);
            i--;
            j--;
        } else if(j>=0 && c[i+1][j+1]==c[i+1][j]+cost_u){
            str1.push_back('_');
            str2.push_back(y[j]);
            j--;
        } else if(i>=0 && c[i+1][j+1]==c[i][j+1]+cost_u){
            str1.push_back(x[i]);
            str2.push_back('_');
            i--;
        } else if(i>=0 && j>=0 && c[i+1][j+1]==c[i][j]+cost_s*2){
            str1.push_back('*');
            str2.push_back('*');
            i--;
            j--;
        }
    }
    std::string x1(str1.begin(), str1.end());
    std::string y1(str2.begin(), str2.end());
    reverse(x1.begin(), x1.end());
    reverse(y1.begin(), y1.end());
    return make_pair(x1, y1);
}