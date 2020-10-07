#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
using namespace std;

// global variables
int total_branches;

struct file_data {
    vector<unsigned long long> pc;
    vector<char> is_taken;
    vector<unsigned long long> target;
    };


// Prototypes
file_data read_file(ifstream &fin, file_data fd);
int always_taken(file_data fd);
void output_results(ofstream &fout, vector<int> correct_output);
void set_total_branches(int tb);

int main(void) {
    file_data fd;
    ifstream fin;
    ofstream fout;
    vector<int> correct_output;
    fin.open("../project1/traces/short_trace1.txt");
    fout.open("output.txt");
    fd = read_file(fin, fd);
    set_total_branches(fd.pc.size());
    
    correct_output.push_back(always_taken(fd));
    output_results(fout, correct_output);
    fin.close();
    fout.close();
}

file_data read_file(ifstream &fin, file_data fd){ 
    // Temporary variables
    unsigned long long addr;
    string behavior, line;
    unsigned long long target;

    // The following loop will read a line at a time
    while(getline(fin, line)) {
    // Now we have to parse the line into it's two pieces
    stringstream s(line);
    s >> std::hex >> addr >> behavior >> std::hex >> target;
    // Add data to struct
    fd.pc.push_back(addr);
    if(behavior == "T") {
        fd.is_taken.push_back('T');
    }else {
        fd.is_taken.push_back('N');
    }
    fd.target.push_back(target);
  }
  
    return fd;
}

int always_taken(file_data fd) {
    int correct = 0;
    for (int i=0; i<fd.pc.size(); i++) {
        if (fd.is_taken[i] == 'T') {
            correct++;
        }
    }
    return correct;
}

void output_results(ofstream &fout, vector<int> correct_output){
    int result_count = correct_output.size();
    int i;
    vector<int> end_line = {1, 2, 9, 16, 25, 26, 27};
    for(i=0; i<result_count; i++){
        fout << correct_output[i] << "," << total_branches << " ";
        i++;
        if(count(end_line.begin(), end_line.end(), i)){
            fout<< endl;
        }
    }
}

void set_total_branches(int tb){
    total_branches = tb;
}
