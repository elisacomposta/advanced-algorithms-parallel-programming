#ifndef ALIGNMENT_H
#define ALIGNMENT_H
using namespace std;

void toUpperCase(string &s);
string inputString(string name);

vector<vector<int>> initC(string x, string y, int cost_g, int cost_r);
void printC(vector<vector<int>> c, string x, string y);

pair<string, string> reconstruct(string x, string y, vector<vector<int>> c, int cost_g, int cost_r);

int alignRecursive(string x, string y, int i, int j, vector<vector<int>> &c, int cost_g, int cost_r);
pair<vector<vector<int>>, pair<string, string>> alignIterative(string x, string y, int cost_g, int cost_r);

int getInt(string name);
pair<string, string> generateSeq(int n, int m);
pair<string, string> generateSeqDisj(int n, int m);

#endif