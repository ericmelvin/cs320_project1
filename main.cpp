#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <map>
#include <math.h>
#include <string>
using namespace std;

// global variables
int total_branches;



struct input_data {
    unsigned long long pc;
    unsigned long long target;
    int is_taken;
};

// Prototypes
vector<input_data>  read_file(ifstream &fin, vector<input_data> fd);
int always_taken(const vector<input_data> &fd);
int always_not_taken(const vector<input_data> &fd);
void output_results(ofstream &fout, vector<int> correct_output);
void set_total_branches(int tb);
int adjust_counter(int bit_len, int is_taken, int counter);
int bimodal(int counter_limit, unsigned long long table_len, const vector<input_data> &fd);
int main(void) {
    // Vector of struct for input file data
    vector<input_data> fd;
    // vector of int to hold all correct results
    vector<int> correct_output;
    ifstream fin;
    ofstream fout;

    // Open input file and output file
    fin.open("../project1/traces/short_trace1.txt");
    fout.open("output.txt");

    // Input file data to fd vector
    fd = read_file(fin, fd);

    // Set global total_branches
    set_total_branches(fd.size());
    
    // Run prediction algorithms
    correct_output.push_back(always_taken(fd));

    correct_output.push_back(always_not_taken(fd));
    
    correct_output.push_back(bimodal(1, 16, fd));
    correct_output.push_back(bimodal(1, 32, fd));
    correct_output.push_back(bimodal(1, 128, fd));
    correct_output.push_back(bimodal(1, 256, fd));
    correct_output.push_back(bimodal(1, 512, fd));
    correct_output.push_back(bimodal(1, 1024, fd));
    correct_output.push_back(bimodal(1, 2048, fd));

    correct_output.push_back(bimodal(3, 16, fd));
    correct_output.push_back(bimodal(3, 32, fd));
    correct_output.push_back(bimodal(3, 128, fd));
    correct_output.push_back(bimodal(3, 256, fd));
    correct_output.push_back(bimodal(3, 512, fd));
    correct_output.push_back(bimodal(3, 1024, fd));
    correct_output.push_back(bimodal(3, 2048, fd));
    correct_output.push_back(bimodal_2bit(16, fd));
    correct_output.push_back(bimodal_2bit(32, fd));



    // Output results to file
    output_results(fout, correct_output);

    // Close input and output files
    fin.close();
    fout.close();
}

/*Function: read_file:
    input: fin: input file
    input: fd: struct to hold file data
    output: struct of file data */
vector<input_data> read_file(ifstream &fin, vector<input_data> fd){ 
    unsigned long long addr;
    unsigned long long target;
    string behavior;
    int is_taken;
    string line;

    // The following loop will read a line at a time
    while(getline(fin, line)) {
        // Now we have to parse the line into it's two pieces
        stringstream s(line);
        s >> std::hex >> addr >> behavior >> std::hex >> target;
        
        // Convert branch to 0 or 1
        is_taken = (behavior == "T") ? 1 : 0;

        // Add file data to struct
        input_data id = {addr, target, is_taken};

        // Add struct to vector
        fd.push_back(id);
    }
    return fd;
}

/*Function: always taken:
    input: fd: struct of file data
    output: number of correct branches predicted */
int always_taken(const vector<input_data> &fd) {
    int correct = 0;
    for (int i=0; i<fd.size(); i++) {
        if (fd[i].is_taken) {
            correct++;
        }
    }
    return correct;
}

/*Function: always not taken:
    input: fd: struct of file data
    output: number of correct branches predicted */
int always_not_taken(const vector<input_data> &fd) {
    int correct = 0;
    for (int i=0; i<fd.size(); i++) {
        if (!fd[i].is_taken) {
            correct++;
        }
    }
    return correct;
}

/*Function: bimodal:
    input: counter_limit: the length of bits for the counter
    input: table_len: the number of entries allowed in the lookup table
    input: fd: struct of file data
    output: number of correct branches predicted 
    NOTE: initial counter value = counter_limit */
int bimodal(int counter_limit, unsigned long long table_len, const vector<input_data> &fd) {
    map<unsigned long long, int> lookup_table;
    map<unsigned long long, int>::iterator iter;

    

    int correct = 0;

    unsigned long long pc;
    unsigned long long mask;
    for (int i=0; i<fd.size(); i++) {
        mask = table_len-1;             // Create mask for pc based on table length
        pc = fd[i].pc & mask;           // Obtain pc's lsb's
        iter = lookup_table.find(pc);  // Does pc exist in table?
        if (iter == lookup_table.end()) { // If pc does not exist
            // Add pc to table with counter value
            lookup_table.insert(pair<int, int>(pc, counter_limit));
        }

        // Count correct predictions
        if(counter_limit == 1) {
                // If prediction is taken and actual outcome is taken
            if (lookup_table[pc] > 0 && fd[i].is_taken) {
                // Increment correct
                correct++; 
            } else if (lookup_table[pc] < 1 && !fd[i].is_taken) {  // not taken and actual outcome is not taken
                // Increment correct
                correct++;
            }
        } else if(counter_limit == 3) {
            if (lookup_table[pc] > 1 && fd[i].is_taken) {
                // Increment correct
                correct++; 
            } else if (lookup_table[pc] < 2 && !fd[i].is_taken) {  // not taken and actual outcome is not taken
                // Increment correct
                correct++;
            }
        } else {
            cout << "Error: counter_limit must be 1 or 3" << endl;
        }  

        // Increment counter or decrement counter
        if (fd[i].is_taken) { 
            lookup_table[pc] = lookup_table[pc] + 1;
            lookup_table[pc] = min(counter_limit, lookup_table[pc]); // Actual outcome adjusts counter
            
        } else if (!(fd[i].is_taken)) { 
            lookup_table[pc] = lookup_table[pc] -1;
            lookup_table[pc] = max(0, lookup_table[pc]);
        } else {
            cout << "ERROR: no known is_taken value" << endl;
        }
    }
    return correct;
}

void output_results(ofstream &fout, vector<int> correct_output){
    int result_count = correct_output.size();

    int i;
    vector<int> end_line = {0, 1, 8, 15, 24, 25, 26};
    for(i=0; i<result_count; i++){
        fout << correct_output[i] << "," << total_branches;


        if(count(end_line.begin(), end_line.end(), i)){
            fout << ";" << endl;
        } else {
            fout << "; ";
        }
    }
}

void set_total_branches(int tb){
    total_branches = tb;
}
