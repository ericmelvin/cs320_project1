#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <map>
#include <math.h>
#include <string>
#include <bitset>
using namespace std;

// global variables
int total_branches;

struct input_data {
    unsigned long long pc;
    unsigned long long target;
    int is_taken;
};

// Prototypes

// IO
void output_results(ofstream &fout, vector<int> correct_output);
vector<input_data>  read_file(ifstream &fin, vector<input_data> fd);

// Predictors
int always_taken(const vector<input_data> &fd);
int always_not_taken(const vector<input_data> &fd);

int bimodal(int counter_limit, unsigned long long table_len, vector<input_data> fd);
int correct_branch(int counter, int is_taken, int counter_len);
int adjust_counter(int is_taken, int counter_limit, int counter_val);

int gshare(unsigned long long ghr_bit_len, vector<input_data> fd);
unsigned long long new_ghr(unsigned long long ghr, int is_taken, unsigned long long ghr_mask);

int tournament(const vector<input_data> &fd);
int adjust_selector_counter(int p1c, int p2c, int counter);
int correct_selector(int counter, int gshare_correct, int bimodal_correct);

void set_total_branches(int tb);

int main(int argc, char** argv) {
    // Vector of struct for input file data
    vector<input_data> fd;
    // vector of int to hold all correct results
    vector<int> correct_output;
    ifstream fin;
    ofstream fout;

    // Open input file and output file
    fin.open(argv[1]);
    fout.open("output.txt");

    // Input file data to fd vector
    fd = read_file(fin, fd);

    // Set global total_branches
    set_total_branches(fd.size());
    
    // Run prediction algorithms
    // correct_output.push_back(always_taken(fd));

    // correct_output.push_back(always_not_taken(fd));

    // correct_output.push_back(bimodal(1, 16, fd));
    // correct_output.push_back(bimodal(1, 32, fd));
    // correct_output.push_back(bimodal(1, 128, fd));
    // correct_output.push_back(bimodal(1, 256, fd));
    // correct_output.push_back(bimodal(1, 512, fd));
    // correct_output.push_back(bimodal(1, 1024, fd));
    // correct_output.push_back(bimodal(1, 2048, fd));

    // correct_output.push_back(bimodal(3, 16, fd));
    // correct_output.push_back(bimodal(3, 32, fd));
    // correct_output.push_back(bimodal(3, 128, fd));
    // correct_output.push_back(bimodal(3, 256, fd));
    // correct_output.push_back(bimodal(3, 512, fd));
    // correct_output.push_back(bimodal(3, 1024, fd));
    // correct_output.push_back(bimodal(3, 2048, fd));


    // correct_output.push_back(gshare(3, fd));
    // correct_output.push_back(gshare(4, fd));
    // correct_output.push_back(gshare(5, fd));
    // correct_output.push_back(gshare(6, fd));
    // correct_output.push_back(gshare(7, fd));
    // correct_output.push_back(gshare(8, fd));
    // correct_output.push_back(gshare(9, fd));
    // correct_output.push_back(gshare(10, fd));
    // correct_output.push_back(gshare(11, fd));

    correct_output.push_back(tournament(fd));

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
int bimodal(int counter_limit, unsigned long long table_len, vector<input_data> fd) {
    map<unsigned long long, int> lookup_table;
    map<unsigned long long, int>::iterator iter;
    unsigned long long pc;
    unsigned long long mask = table_len -1;

    int correct = 0;
    
    // If counter len = 3, bits = 2, else 1
    int counter_len = (counter_limit == 3) ? 2 : 1;
    for (int i=0; i<fd.size(); i++) {
        pc = fd[i].pc & mask;           // Obtain pc's lsb's
        iter = lookup_table.find(pc);  // Does pc exist in table?
        
        // If pc does not exist
        if (iter == lookup_table.end()) { 
            // Add pc to table with counter value
            lookup_table.insert(pair<int, int>(pc, counter_limit));
        }

        // Count correct predictions
        correct += correct_branch(lookup_table[pc], fd[i].is_taken, counter_len);
      
        // Increment counter or decrement counter
        lookup_table[pc] = adjust_counter(fd[i].is_taken, counter_limit, lookup_table[pc]);
    }
    return correct;
}

int gshare(unsigned long long ghr_bit_len, vector<input_data> fd) {

    unsigned long long ghr = 0;
    unsigned long long index = 0;
    unsigned long long pc_mask = 0;
    unsigned long long ghr_mask = pow(2, ghr_bit_len)-1;
    unsigned long long pc = 0;
    int counter_limit = 3; // and initial counter state 
    int table_size = 2048;
    int correct = 0;
    map<unsigned long long, int> lookup_table;

    pc_mask = table_size-1; // Create mask for pc based on table length

    // Build prediction table
    for(int i=0; i<table_size; i++) {
        lookup_table[i] = counter_limit;
    }

    // Iterate over pc and (T/N)
    for(int i=0; i<fd.size(); i++) {
        pc = fd[i].pc & pc_mask; // Obtain pc's lsb's
        ghr = ghr & ghr_mask; // Create ghr mask
        index = pc^ghr; // Create index

        // Increment correct predictions
        correct += correct_branch(lookup_table[index], fd[i].is_taken, 2);

        // Increment counter or decrement counter
        lookup_table[index] = adjust_counter(fd[i].is_taken, counter_limit, lookup_table[index]);

        // Create new ghr
        ghr = new_ghr(ghr, fd[i].is_taken, ghr_mask);
        //cout << "new ghr: " << ghr << endl;
    }

    return correct;
}

unsigned long long new_ghr(unsigned long long ghr, int is_taken, unsigned long long ghr_mask) {
    ghr = ghr << 1;
    ghr = ghr | is_taken;
    ghr = ghr & ghr_mask;
    return ghr;
}

int adjust_counter(int is_taken, int counter_limit, int counter_val) {
    if (is_taken == 1) {
        counter_val = min(counter_limit, ++counter_val);
    } else if (is_taken == 0) {
        counter_val = max(0, --counter_val);
    } else {
        cout << "Error: no known 'is_taken' value" << endl;
    }
    return counter_val;
}

int correct_branch(int counter, int is_taken, int counter_len) {
    int correct = 0;
    if (counter_len == 1) {
        if(counter > 0 && is_taken) {
        correct++;
        } else if (counter < 1 && !is_taken) {
            correct++;
        }
    } else if (counter_len == 2) {
        if(counter > 1 && is_taken) {
        correct++;
        } else if (counter < 2 && !is_taken) {
            correct++;
        }
    } else {
        cout << "Error: counter length must be 1 or 2" << endl;
    }
    
    return correct;
}
int tournament(const vector<input_data> &fd) {
    // Selector table
    map<unsigned long long, int> selector_table;
    // bimodal counter table
    map<unsigned long long, int> bimodal_table;
    // gshare counter table
    map<unsigned long long, int> gshare_table;

    // Initial values
    int gshare_correct = 0;
    int bimodal_correct = 0;
    int tbl_len = 2048;
    int correct = 0;
    int counter_limit = 3;
    int selector_counter = 0;
    int counter_len = (counter_limit == 3) ? 2 : 1;

    unsigned long long pc_mask = tbl_len-1;
    unsigned long long ghr_mask = tbl_len-1;
    unsigned long long gshare_index = 0;
    unsigned long long ghr = 0;
    unsigned long long pc = 0;

    // Fill counter tables with default values
    for(int i=0; i<tbl_len; i++) {
        selector_table[i] = selector_counter;
        bimodal_table[i] = counter_limit;
        gshare_table[i] = counter_limit;
    }
    // Iterate over all instructions
    for(int i=0; i<fd.size(); i++) {
        // Reset correct predictors
        bimodal_correct = 0;
        gshare_correct = 0;

        pc = fd[i].pc & pc_mask;
        ghr = ghr & ghr_mask;
        gshare_index = pc^ghr;
        gshare_index = gshare_index & ghr_mask;
        // gshare_index mask

        // Bimodal predictor---------------------------------------------------------------
        
        // Check if predictor is correct
        bimodal_correct = correct_branch(bimodal_table[pc], fd[i].is_taken, counter_len);
        // Adjust bimodal counter
        bimodal_table[pc] = adjust_counter(fd[i].is_taken, counter_limit, bimodal_table[pc]);
        
        // Gshare predictor----------------------------------------------------------------
        
        // Check if predictor is correct
        gshare_correct = correct_branch(gshare_table[gshare_index], fd[i].is_taken, counter_len);

        // Adjust gshare counter
        gshare_table[gshare_index] = adjust_counter(fd[i].is_taken, counter_limit, gshare_table[gshare_index]);

        // Update ghr
        ghr = ghr << 1;
        ghr = ghr | fd[i].is_taken;
        int is_taken = fd[i].is_taken;
        ghr = ghr & ghr_mask;
        // Selector-----------------------------------------------------------------------

        // Adjust correct predictions
        correct += correct_selector(selector_table[pc], gshare_correct, bimodal_correct);

        // Adjust selector counter
        selector_table[pc] = adjust_selector_counter(bimodal_correct, gshare_correct, selector_table[pc]);
        // if(bimodal_correct && !gshare_correct) {
        //     int selector_counter = selector_table[pc];
        //     cout << "pc at" << pc << "+1";
        // }
        // For testing only
        if (i % 10000 == 0) {
            cout << i << "/" << fd.size() << endl;
        }
    }
    return correct;;
}

int adjust_selector_counter(int p1c, int p2c, int counter) {
    int dif = p1c - p2c;
    if(dif == 1) {
        return min(3, counter + dif);
    } else if (dif == -1) {
        return max(0, counter - dif);
    } else {
        return counter;
    }
}

int correct_selector(int counter, int gshare_correct, int bimodal_correct) {
    // Selector counter logic----------------------------------------------------------
    // If counter 00 or 01, then bimodal
    // If counter 10 or 11, then gshare
    // Increment correct predictions
    int correct = 0;
    if (counter < 2 && gshare_correct) {
        correct++;
    } else if (counter > 1 && bimodal_correct) {
        correct++;
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
