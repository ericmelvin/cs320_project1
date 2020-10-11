#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <map>
#include <math.h>
using namespace std;

// global variables
int total_branches;



struct input_data {
    unsigned long long pc;
    unsigned long long target;
    uint8_t is_taken;
};

// Prototypes
vector<input_data>  read_file(ifstream &fin, vector<input_data> fd);
int always_taken(const vector<input_data> &fd);
int always_not_taken(const vector<input_data> &fd);
void output_results(ofstream &fout, vector<int> correct_output);
void set_total_branches(int tb);
int adjust_counter_1_bit(uint8_t is_taken, int counter);
int bimodal_1_bit(const vector<input_data> &fd);
int main(void) {
    vector<input_data> fd;
    ifstream fin;
    ofstream fout;
    vector<int> correct_output;
    fin.open("../project1/traces/short_trace1.txt");
    fout.open("output.txt");
    fd = read_file(fin, fd);
    set_total_branches(fd.size());
    
    correct_output.push_back(always_taken(fd));
    correct_output.push_back(always_not_taken(fd));
    correct_output.push_back(bimodal_1_bit(fd));
    output_results(fout, correct_output);
    fin.close();
    fout.close();
}

vector<input_data> read_file(ifstream &fin, vector<input_data> fd){ 
    // Temporary variables
    unsigned long long addr;
    unsigned long long target;
    char behavior;
    uint8_t is_taken;
    string line;

    // The following loop will read a line at a time
    while(getline(fin, line)) {
        // Now we have to parse the line into it's two pieces
        stringstream s(line);
        s >> std::hex >> addr >> behavior >> std::hex >> target;
        // Add data to struct
        is_taken = (behavior == 'T') ? 1 : 0;
        input_data id = {addr, target, is_taken};
        // Add struct to vector
        fd.push_back(id);
    }
    return fd;
}

int always_taken(const vector<input_data> &fd) {
    int correct = 0;
    for (int i=0; i<fd.size(); i++) {
        if (fd[i].is_taken) {
            correct++;
        }
    }
    return correct;
}

int always_not_taken(const vector<input_data> &fd) {
    int correct = 0;
    for (int i=0; i<fd.size(); i++) {
        if (!fd[i].is_taken) {
            correct++;
        }
    }
    return correct;
}

int bimodal_1_bit(const vector<input_data> &fd) {
    map<unsigned long long, uint8_t> lookup_table;
    map<unsigned long long, uint8_t>::iterator iter;
    uint8_t counter = 1;  // taken
    int correct = 0;
    unsigned long long pc;
    unsigned long long mask;
    for (int i=0; i<fd.size(); i++) {
        mask = pow(2, 4) -1;
        pc = fd[i].pc & mask;
        iter = lookup_table.find(pc);
        if (iter == lookup_table.end()) { // pc doesn't exist in lookup table
            lookup_table.insert(pair<int, int>(pc, counter));
        }
        if (lookup_table[pc] == 1 && fd[i].is_taken) {
            correct++; 
        } else if (lookup_table[pc] == 0 && !fd[i].is_taken) {
            correct++;
        }
        counter = adjust_counter_1_bit(fd[i].is_taken, counter);
        lookup_table[pc] = counter;
    }
    for(auto elem : lookup_table) {
        cout << elem.first << " " << elem.second << endl;
    }
    return correct;
    
}
int adjust_counter_1_bit(uint8_t is_taken, int counter) {
    if (counter > 1) { counter = 1; };
    if (counter < 0) { counter = 0; };

    if (is_taken && counter < 1) { 
        return counter + 1; 
    }
    if (!is_taken && counter > 0) {
        return counter - 1; 
    }
    
    return counter;
}
void output_results(ofstream &fout, vector<int> correct_output){
    int result_count = correct_output.size();

    int i;
    vector<int> end_line = {0, 1, 8, 15, 24, 25, 26};
    for(i=0; i<result_count; i++){
        fout << correct_output[i] << "," << total_branches << " ";


        if(count(end_line.begin(), end_line.end(), i)){
            fout<< endl;
            
        }
    }
}

void set_total_branches(int tb){
    total_branches = tb;
}
