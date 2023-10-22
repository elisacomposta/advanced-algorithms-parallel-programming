//%%writefile lcs.cpp   // for notebook

#include <iostream>
#include <vector>
#include <string>

std::vector<char> x = {'B','D','C','A','B','A'};
std::vector<char> y = {'A','B','C','B','D','A','B'};
std::vector<std::vector<int>> c(x.size()+1, std::vector<int>(y.size()+1, -1));

/* Longest Common Subsequence function.
 * x, y: vectors of chars to compare
 * returns: longest common subsequence between x[0 to i] and y[0 to j]
*/
int lcs(std::vector<char> x, std::vector<char> y, int i, int j){
  if(c[i+1][j+1] == -1){
    if(x[i] == y[j]) {
      c[i+1][j+1] = lcs(x, y, i-1, j-1) + 1;
    } else {
      c[i+1][j+1] = std::max(lcs(x, y, i-1, j), lcs(x, y, i, j-1));
    }
  }
  return c[i+1][j+1];
}


std::string reconstruct(int i, int j, int len){
  std::vector<char> sub(len, 'z');
  while(len>0){
    while(j>0 && c[i][j]==c[i][j-1]){
      j--;
    }
    while(i>0 && c[i][j]==c[i-1][j]){
      i--;
    }
    i--;
    j--;
    len--;
    sub[len] = x[i];
  }
  std::string str(sub.begin(), sub.end());
  return str;
}

void printC(){
  for(int i=0; i<c.size(); i++){
    for(int j=0; j<c[0].size(); j++){
      if(c[i][j]>=0){
        std::cout << " ";
      }
      std::cout << c[i][j] << " ";
    }
    std::cout << std::endl;
  }
}


int main(){
  
  for(int i=0; i<x.size()+1; i++){
    for(int j=0; j<y.size()+1; j++){
      if(i==0 || j==0){
        c[i][j] = 0;
      }
    }
  }

  int len = lcs(x, y, x.size()-1, y.size()-1);
  std::string seq = reconstruct(x.size(), y.size(), len);

  std::cout << "Longest common subsequence: " << seq << "\nLength: " << seq.size() << std::endl;
  printC();
}