
//  Created by gordie2 on 10/2/20.
//  Copyright © 2020 Gordon Kolb. All rights reserved.
//

#ifndef Mine_hpp
#define Mine_hpp

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <getopt.h>
#include <iostream>
#include <queue>
#include <iomanip>
#include "P2random.h"

using namespace std;

#endif /* Mine_hpp */

struct location {
    int value;
    bool is_discovered = false;
    
    void print_location_value(){
        cout << setw(4) << value;
    }
};

struct pq_location {
    int value;
    size_t row;
    size_t col;
    
    void print_pq_location_value(){
        cout << setw(4) << value;
    }
};

struct pq_functor {
    bool operator() (const pq_location &a, const pq_location &b){
        if (a.value != b.value){
            return (a.value > b.value);
        }
        else if (a.col != b.col){
            return (a.col > b.col);
        }
        else {
            return (a.row > b.row);
        }
    }
};

struct pq_functor_reverse {
    bool operator() (const pq_location &a, const pq_location &b){
        if (a.value != b.value){
            return (a.value < b.value);
        }
        else if (a.col != b.col){
            return (a.col < b.col);
        }
        else {
            return (a.row < b.row);
        }
    }
};


class Mine {
private:
    bool verbose_flag = false;
    bool median_flag = false;
    bool stats_flag = false;
    size_t stats_flag_tiles = 0;
    size_t num_tnt = 0;
    bool map_format = false;
    bool random_format = false;
    size_t start_row = 0;
    size_t start_col = 0;
    size_t size = 0;
    size_t seed = 0;
    size_t max_rubble = 0;
    size_t tnt_chance = 0;
    string mode;
    bool escaped = false;
    int num_tiles = 0;
    int amount_rubble = 0;
    //creates a vector of size rows, size cols
    vector<vector<location>>map;
    //least is at the top()
    priority_queue <double> bottom_half_median;
    //greatest is at the top()
    priority_queue <double, vector<double>, greater<double>> top_half_median;
    
    priority_queue<pq_location, vector<pq_location>, pq_functor> stats_easiest;
    
    priority_queue<pq_location, vector<pq_location>, pq_functor_reverse> stats_hardest;
    
    vector<pq_location> stats_vector;
    
public:
    // The long_options variable contains a description of the options.
    void getOptions (int argc, char** argv){
        static option long_options[] = {
            // LONG        ARGUMENT USED?     (ignore) RETURN VALUE
            {"help",    no_argument,        nullptr, 'h'},
            {"stats",   required_argument,  nullptr, 's'},
            {"median",  no_argument,        nullptr, 'm'},
            {"verbose", no_argument,        nullptr, 'v'},
            // this one tells getopt_long that there are no more options:
            {nullptr,      0,                 nullptr,  0}
        };
        
        // 'choice' is the value returned by getopt_long.
        int choice = 0;
                                        //short hand args with colon after if required argument after
        while ((choice = getopt_long(argc, argv, "hmvs:", long_options, nullptr)) != -1) {
            
            if (choice == 's') { // stats
                stats_flag = true;
                
                string num(optarg);
                stats_flag_tiles = static_cast<size_t>(stoi(num));
            }
            if (choice == 'h') { // help
                cout << "description/n";
                exit(0);
            }
            if (choice == 'm') { // median
                median_flag = true;
            }
            if (choice == 'v') { // verbose
                verbose_flag = true;
            }
        }
        //error checking???
    }
    
    void read_mine(){
        string read_in;
        
        
        // read in map type
        getline(cin, read_in);
        if (read_in == "M"){
            map_format = true;
            mode = "M";
            //cout << "map_format" << "\n";
        }
        else if (read_in == "R"){
            random_format = true;
            mode = "R";
            //cout << "random format" << "\n";
        }
        else {
            cerr << "Invalid input mode";
            exit(1);
        }
        
        
        
        //read in size
        cin >> read_in >> read_in;
        size = static_cast<size_t>(stoi(read_in));
        //cout << "size: " << read_in << "\n";
        
        //read in start position
        cin >> read_in >> read_in;
        start_row = static_cast<size_t>(stoi(read_in));
        cin >> read_in;
        start_col = static_cast<size_t>(stoi(read_in));
        //cout << "start row, col: " << start_row << ", " << start_col << "\n";
        
        if (start_row > size){
            cerr << "Invalid starting row";
            exit(1);
        }
        if (start_col > size){
            cerr << "Invalid starting column";
            exit(1);
        }
        
        map.resize(size, vector<location>(size));
        stringstream ss;
        
        if (random_format){
            //read in seed
            cin >> read_in >> read_in;
            seed = static_cast<size_t>(stoi(read_in));
            //cout << "seed: " << seed << "\n";
            
            //read in max rubble
            cin >> read_in >> read_in;
            max_rubble = static_cast<size_t>(stoi(read_in));
            //cout << "max rubble: " << max_rubble << "\n";
            
            //read in TNT
            cin >> read_in >> read_in;
            tnt_chance = static_cast<size_t>(stoi(read_in));
            //cout << "tnt: " << tnt_chance << "\n";
            
            
            P2random::PR_init(ss, static_cast<unsigned int>(size), static_cast<unsigned int>(seed), static_cast<unsigned int>(max_rubble), static_cast<unsigned int>(tnt_chance));
        }
        
            
        istream &inputStream = (mode == "M") ? cin : ss;
            
        for (size_t r = 0; r < size; r++){
            for (size_t c = 0; c < size; c++){
                location fill;
                inputStream >> read_in;
                fill.value = stoi(read_in);
                map[r][c] = fill;
            }
        }
            
        /*//testing to see if read in correctly
        for (size_t r = 0; r < size; r++){
            for (size_t c = 0; c < size; c++){
                
                map[r][c].print_location_value();
                cout << "   ";
            }
            cout << "\n";
        }*/
    }
    
    void median_helper(double value){
        //add value to priority queue
        if (bottom_half_median.empty()){
            bottom_half_median.push(value);
        }
        else if (top_half_median.empty()){
            top_half_median.push(value);
            if (!bottom_half_median.empty() && !top_half_median.empty() && bottom_half_median.top() > top_half_median.top()){
                double top = bottom_half_median.top();
                double bottom = top_half_median.top();
                bottom_half_median.pop();
                top_half_median.pop();
                bottom_half_median.push(bottom);
                top_half_median.push(top);
            }
        }
        else if (value < top_half_median.top()){
            bottom_half_median.push(value);
        }
        else {
            top_half_median.push(value);
        }
        //adjust median if needed
        if ((top_half_median.size() - bottom_half_median.size()) == 2){
            bottom_half_median.push(top_half_median.top());
            top_half_median.pop();
        }
        else if ((bottom_half_median.size() - top_half_median.size()) == 2){
            top_half_median.push(bottom_half_median.top());
            bottom_half_median.pop();
        }
        //print median
        if ((bottom_half_median.size() - top_half_median.size()) == 1){
            cout << "Median difficulty of clearing rubble is: " << bottom_half_median.top() << "\n";
        }
        else if ((top_half_median.size() - bottom_half_median.size()) == 1){
            cout << "Median difficulty of clearing rubble is: " << top_half_median.top() << "\n";
        }
        else {
            cout << "Median difficulty of clearing rubble is: "
            << ((top_half_median.top() + bottom_half_median.top()) / 2) << "\n";
        }

    }
        
        
    void escape(){
        
        /*cout << "test \n";
        cout << "\n";
        pq_location a;
        a.value = 9;
        a.row = 0;
        a.col = 2;
        
        pq_location b;
        b.value = 9;
        b.row = 1;
        b.col = 1;
        
        pq_location c = (a, b);
        cout << c.value << c.row << c.col;*/
        
        priority_queue<pq_location, vector<pq_location>, pq_functor> primary_pq;
        
        pq_location start_location;
        start_location.row = start_row;
        start_location.col = start_col;
        start_location.value = map[start_row][start_col].value;
        map[start_row][start_col].is_discovered = true;
        primary_pq.push(start_location);
        size_t current_row = start_row;
        size_t current_col = start_col;
        
        pq_location current_location = primary_pq.top();
        primary_pq.pop();
        
        
        while (escaped == false){
            
            if (current_location.row == size - 1 || current_location.col == size - 1 ||
                current_location.row == 0 || current_location.col == 0){
                //cout << "end row: " << current_location.row << "end col: " << current_location.col << "\n";
                
                escaped = true;
            }
            
            //if current location is a tnt
            if (current_location.value == -1){
                size_t tnt_row = current_row;
                size_t tnt_col = current_col;
                pq_location current_tnt = current_location;
                priority_queue<pq_location, vector<pq_location>, pq_functor> tnt_pq;
                //tnt_pq.push(current_tnt);
                bool first_neg_one = true;
                /*if (verbose_flag){
                    cout << "TNT explosion at " << "[" << tnt_row << "," << tnt_col << "]!\n";
                }*/
                if (stats_flag){
                    stats_easiest.push(current_tnt);
                    stats_hardest.push(current_tnt);
                    stats_vector.push_back(current_tnt);
                }
                num_tnt++;
            
                while (current_tnt.value == -1){
                    /*cout << "testing map \n";
                    print_map_helper(map);
                    cout << "\n";*/
                    //above
                    
                    
                    if (current_tnt.row != 0){
                        escape_add_helper(tnt_pq, current_tnt.row - 1, current_tnt.col, false);
                        map[current_tnt.row - 1][current_tnt.col].value = 0;
                    }
                    
                    
                        
                    //below
                    if (current_tnt.row != size - 1){
                        escape_add_helper(tnt_pq, current_tnt.row + 1, current_tnt.col, false);
                        map[current_tnt.row + 1][current_tnt.col].value = 0;
                    }
                    
                    
                    //left
                    if (current_tnt.col != 0){
                        escape_add_helper(tnt_pq, current_tnt.row, current_tnt.col - 1, false);
                        map[current_tnt.row][current_tnt.col - 1].value = 0;
                    }
                    
                    
                      
                    
                    //right
                    if (current_tnt.col != size - 1){
                        escape_add_helper(tnt_pq, current_tnt.row, current_tnt.col + 1, false);
                        map[current_tnt.row][current_tnt.col + 1].value = 0;
                    }
                    
                    
                        
                    
                    
                    
                    if (!first_neg_one){
                        if (stats_flag){
                            stats_easiest.push(current_tnt);
                            stats_hardest.push(current_tnt);
                            stats_vector.push_back(current_tnt);
                            num_tnt++;
                        }
                    }
                    map[tnt_row][tnt_col].value = 0;
                    current_tnt.value = 0;
                    if (!first_neg_one){
                        primary_pq.push(current_tnt);
                    }
                    if (verbose_flag){
                        if (tnt_row == 502 && tnt_col == 503){
                            tnt_pq.top();
                        }
                        //if (tnt_row != prev_row || tnt_col != prev_col){
                            cout << "TNT explosion at " << "[" << tnt_row << "," << tnt_col << "]!\n";
                        //}
                    }
                    first_neg_one = false;
                    //if (!tnt_pq.empty()){
                        current_tnt = tnt_pq.top();
                        tnt_pq.pop();
                        tnt_row = current_tnt.row;
                        tnt_col = current_tnt.col;
                    //}

                }
                while (!tnt_pq.empty()){
                    /*cout << "testing map \n";
                    print_map_helper(map);
                    cout << "\n";*/
                    if (current_tnt.value != 0){
                        num_tiles += 1;
                        amount_rubble += current_tnt.value;
                        if (verbose_flag){
                            cout << "Cleared by TNT: " << current_tnt.value
                                 << " at [" << tnt_row << "," << tnt_col << "]\n";
                        }
                        if (median_flag){
                            median_helper(current_tnt.value);
                        }
                        if (stats_flag){
                            stats_easiest.push(current_tnt);
                            stats_hardest.push(current_tnt);
                            stats_vector.push_back(current_tnt);
                        }
                    }
                    map[tnt_row][tnt_col].value = 0;
                    current_tnt.value = 0;
                    primary_pq.push(current_tnt);
                    current_tnt = tnt_pq.top();
                    tnt_pq.pop();
                    tnt_row = current_tnt.row;
                    tnt_col = current_tnt.col;
                    if (tnt_pq.empty()){
                        if (current_tnt.value != 0){
                            num_tiles += 1;
                            amount_rubble += current_tnt.value;
        
                            if (verbose_flag){
                                cout << "Cleared by TNT: " << current_tnt.value
                                     << " at [" << tnt_row << "," << tnt_col << "]\n";
                            }
                            if (median_flag){
                                median_helper(current_tnt.value);
                            }
                            if (stats_flag){
                                stats_easiest.push(current_tnt);
                                stats_hardest.push(current_tnt);
                                stats_vector.push_back(current_tnt);
                            }
                        }
                        map[tnt_row][tnt_col].value = 0;
                        current_tnt.value = 0;
                        primary_pq.push(current_tnt);
                    }
                }
                /*cout << "testing map after -1s \n";
                print_map_helper(map);
                cout << "\n";*/
            }
            
            ///////////////fdsdfdsds
            
            //above
            if (escaped == false){
                escape_add_helper(primary_pq, current_location.row - 1, current_location.col, true);
                //below
                escape_add_helper(primary_pq, current_location.row + 1, current_location.col, true);
                //left
                escape_add_helper(primary_pq, current_location.row, current_location.col - 1, true);
                //right
                escape_add_helper(primary_pq, current_location.row, current_location.col + 1, true);
            }
            
            
            if (map[current_location.row][current_location.col].value != 0){
                num_tiles += 1;
                amount_rubble += map[current_location.row][current_location.col].value;
            
                if (verbose_flag){
                    cout << "Cleared: " << map[current_location.row][current_location.col].value
                         << " at [" << current_location.row << "," << current_location.col << "]\n";
                }
                if (median_flag){
                    median_helper(current_location.value);
                }
                if (stats_flag){
                    stats_easiest.push(current_location);
                    stats_hardest.push(current_location);
                    if (current_location.row == 566 && current_location.col == 483){
                        stats_vector.size();
                    }
                    stats_vector.push_back(current_location);
                }
            }
            map[current_location.row][current_location.col].value = 0;
            if (!primary_pq.empty()){
                current_location = primary_pq.top();
                primary_pq.pop();
            
                while (map[current_location.row][current_location.col].value != current_location.value){
                    current_location = primary_pq.top();
                    primary_pq.pop();
                }
            }
            current_row = current_location.row;
            current_col = current_location.col;
            
            
        }
        cout << "Cleared " << num_tiles << " tiles containing " << amount_rubble
        << " rubble and escaped.\n";
    }
    
    void print_stats(){
        if (stats_flag){
            size_t min = 0;
            if (stats_flag_tiles <= static_cast<size_t>(num_tiles) + num_tnt){
                min = stats_flag_tiles;
            }
            else {
                min = (static_cast<size_t>(num_tiles) + num_tnt);
            }
            //first n tiles cleared
            cout << "First tiles cleared:\n";
            for (size_t i = 0; i < min; i++){
                if (stats_vector[i].value == -1){
                    cout << "TNT";
                }
                else {
                    cout << stats_vector[i].value;
                }
                cout << " at [" << stats_vector[i].row << "," << stats_vector[i].col << "]\n";
            }
            //last n tiles cleared
            cout << "Last tiles cleared:\n";
            size_t s = stats_vector.size() - 1;
            for (size_t i = 0; i < min; i++){
                if (stats_vector[s-i].value == -1){
                    cout << "TNT";
                }
                else {
                    cout << stats_vector[s-i].value;
                }
                cout << " at [" << stats_vector[s-i].row << "," << stats_vector[s-i].col << "]\n";
            }
            //easiest n tiles cleared
            cout << "Easiest tiles cleared:\n";
            for (size_t i = 0; i < min; i++){
                if (stats_easiest.top().value == -1){
                    cout << "TNT";
                }
                else {
                    cout << stats_easiest.top().value;
                }
                cout << " at [" << stats_easiest.top().row << "," << stats_easiest.top().col << "]\n";
                stats_easiest.pop();
            }
            //hardest n tiles cleared
            cout << "Hardest tiles cleared:\n";
            for (size_t i = 0; i < min; i++){
                if (stats_hardest.top().value == -1){
                    cout << "TNT";
                }
                else {
                    cout << stats_hardest.top().value;
                }
                cout << " at [" << stats_hardest.top().row << "," << stats_hardest.top().col << "]\n";
                stats_hardest.pop();
            }
        }
    }
    
    void print_map_helper(const vector<vector<location>> &map){
        //testing to see if read in correctly
        for (size_t r = 0; r < size; r++){
            for (size_t c = 0; c < size; c++){
                
                cout << setw(4) << map[r][c].value;
                cout << "   ";
            }
            cout << "\n";
        }
    }
    
    void escape_add_helper(priority_queue<pq_location,
                           vector<pq_location>, pq_functor> &a,
                           size_t r, size_t c, bool is_discovered_required){
        if (r > size - 1 || c > size - 1){
            return;
        }
        /*if (map[r][c].value == 0){
            return;
        }*/
        if (map[r][c].is_discovered == false && is_discovered_required){
            escape_add_helper2(a, r, c);
            map[r][c].is_discovered = true;
        }
        else if (is_discovered_required == false){
            escape_add_helper2(a, r, c);
            map[r][c].is_discovered = true;
        }
    }
    
    void escape_add_helper2(priority_queue<pq_location,
                           vector<pq_location>, pq_functor> &a,
                           size_t r, size_t c){
            pq_location temp;
            temp.value = map[r][c].value;
            temp.row = r;
            temp.col = c;
            a.push(temp);
    }
        
    
    
    
    
    
};
