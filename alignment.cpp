#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

/*
vector<char> x = {'A', 'E', 'D'};
vector<char> y = {'A', 'B', 'C', 'D'};
*/

vector<char> x = {'C', 'G'};
vector<char> y = {'C', 'A'};

/*
vector<char> y = {'A', 'G', 'G', 'G', 'C', 'T'};
vector<char> x = {'A', 'G', 'G', 'C', 'A'};
*/

vector<vector<int>> c (x.size()+1, vector<int>(y.size()+1, -1));


int align(vector<char> x, vector<char> y, int i, int j);
pair<string, string> reconstruct();
void initC();
void printC();


int main(){
    initC();
    int cost = align(x, y, x.size()-1, y.size()-1);
    printC();
    pair<string, string> res = reconstruct();
    cout << "Final X1: " << res.first << endl;
    cout << "Final Y1: " << res.second << endl;
    cout << "Cost: " << cost << endl;
}


// print matrix c
void printC(){
    for(int i=0; i<c.size(); i++){
        for(int j=0; j<c[0].size(); j++){
            if(c[i][j] >= 0){
                cout << " ";
            }
            cout << c[i][j] << " ";
        }
        cout << endl;
    }
}


// init matrix c
void initC(){
    for(int i=0; i<c.size(); i++){
        c[i][0] = i*2;
    }
    for(int j=0; j<c[0].size(); j++){
        c[0][j] = j*2;
    }
}


// compute minimum alignment cost
int align(vector<char> x, vector<char> y, int i, int j){
    if(c[i+1][j+1]==-1){
        if(x[i]==y[j]){
            c[i+1][j+1] = align(x, y, i-1, j-1);
        } else {
            c[i+1][j+1] = min(align(x, y, i-1, j), align(x, y, i, j-1)) + min(2, 5);
        }
    }
    return c[i+1][j+1];
}


// reconstruct solution
pair<string, string> reconstruct(){
    vector<char> str1;
    vector<char> str2;

    int i = x.size()-1;
    int j = y.size()-1;
    int iter = 0;
    while( (i>=0 || j>=0)){
        //cout << "i: " << i << " j: " << j << endl;
        if(x[i] == y[j]){
            str1.push_back(x[i]);
            str2.push_back(y[j]);
            //cout << "   X1:" << x[i] << " Y1:" << y[j] << endl;
            i--;
            j--;
        } else if(j>0 && c[i+1][j+1]==c[i+1][j]+2){
            str1.push_back('_');
            str2.push_back(y[j]);
            //cout << "   X1:_" << " Y1:" << y[j] << endl;
            j--;
        } else if(i>0 && c[i+1][j+1]==c[i][j+1]+2){
            str1.push_back(x[i]);
            str2.push_back('_');
            //cout << "   X1:" << x[i] << " Y1:_" << endl;
            i--;
        }
    }
    std::string x1(str1.begin(), str1.end());
    std::string y1(str2.begin(), str2.end());
    reverse(x1.begin(), x1.end());
    reverse(y1.begin(), y1.end());
    return make_pair(x1, y1);
}