//
//  20217231.cpp
//  AIM
//
//  Created by Zhuoran BI on 2022/3/30.
//
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <string.h>
using namespace std;


// Structures
struct item {
    int weight;
    int index;
    
};

struct bin{
    std::vector<item> packed_items; //the list of item in this bin
    int cap_left;
};

struct problem{
    int C; //capacity
    int num_of_items;
    struct item* items;
    std::string problemName;
    int exampleAnswer;
};


struct solution{
    struct problem* prob; //maintain a shallow copy of problem data
    float objective;
    bool feasibility; //indicate the feasibility of the solution
    std::vector<bin> bins;
};



int NUM_OF_RUNS = 1; // number of runs
// Initialization of variables
int MAX_TIME = 0;
char data_file[50]={}, out_file[50]={}, solution_file[50]={};
std::ofstream outfile;
int number_of_Problem_Instances = 0; //number of instances in the input file
vector<std::string> instances_Name; //store the name of every instances
vector<int> capacity;        //the bin capacity of each instances
vector<int> number_of_Items; //number of items in each items
vector<int> example_Bins;    //example answer of each instances
std::vector<std::vector<item>> itemsList;//item list
//VNS parameter
int K = 3; // number of neighbourhood is used
struct solution* best_sln;


// Function declaration
bool  comp(const item &a, const item &b);
void readInput(std::string path);
struct solution* firstFit(struct problem* prob);
struct solution* bestFit(struct problem* prob);
struct problem* init_prob(int instancesID);
struct solution* create_empty_sol(struct problem* my_prob);
struct solution* copy_from(struct solution* sln);
void print_sol(struct solution* sln);
void delateEmpty(struct solution* sln);
int fitnessFunction(struct solution* sln);
struct solution* shaking(struct solution* curSolution);
int fitnessFunction(struct solution* sln);
struct solution* H1(struct solution* sln);
struct solution* H2(struct solution* sln);
struct solution* H3(struct solution* sln);
struct solution* localSearch_vns(int nb_index,struct solution* sln);
int varaible_neighbourhood_search(struct problem* prob);


// --------------------------------------------------------------------
// --------------------------------------------------------------------


//This function is used to sort the the item
bool  comp(const item &a, const item &b) {
    return a.weight > b.weight;
}


//Read the input file
void readInput(std::string path){
    std::ifstream data;
    data.open(path);
    if (!data.is_open()){ //if can open
            perror("Error");
        }
    data >> number_of_Problem_Instances;
    
    //read all the instances at one time
    for (int i = 0; i<number_of_Problem_Instances; i++) {
        std::string temp;
        int temp2,temp3,temp4;
        
        data >> temp;
        instances_Name.push_back(temp);
        data >> temp2 >> temp3 >> temp4;
        capacity.push_back(temp2);
        number_of_Items.push_back(temp3);
        example_Bins.push_back(temp4);
        
        item temp5;
        int index = 0;
        std::vector<item> temp6;
        //read every item
        for (int j = 0; j < number_of_Items[i]; j++) {
            data >> temp5.weight;
            temp5.index = index;
            index++;
            temp6.push_back(temp5);
            }
        itemsList.push_back(temp6);

    }
    
    //sort the items in each question from large to small
    for (int i = 0; i < itemsList.size(); i++) {
        sort(itemsList[i].begin(),itemsList[i].end(),comp);
    }
    
    data.close();
}


// Read in problem(note: remember to use delete to free memory)
struct problem* init_prob(int instancesID){
    item* my_item = new item[number_of_Items[instancesID]];
    //put the item into the problem
    for(int i=0; i< number_of_Items[instancesID]; i++){
        my_item[i] = itemsList[instancesID][i];
    }
    
    problem* my_problem = new problem;
    my_problem->C  = capacity[instancesID];
    my_problem->items = my_item;
    my_problem->num_of_items = number_of_Items[instancesID];
    my_problem->problemName = instances_Name[instancesID];
    my_problem->exampleAnswer = example_Bins[instancesID];
    return my_problem;
    }


//Create solution with problem
struct solution* create_empty_sol(struct problem* my_prob){
    solution* my_sln = new solution;
    my_sln->prob = my_prob;
    my_sln->objective = 0;
    my_sln->feasibility = false;
    return my_sln;
}


//Replicate problem
struct solution* copy_from(struct solution* sln){
    solution* my_sln = new solution;
    my_sln->prob = sln->prob;  //shalow copy
    my_sln->objective=sln->objective;
    my_sln->bins = sln->bins;
    my_sln->feasibility = sln->feasibility;
    return my_sln;
}


//Print the solution and write it into file
void print_sol(struct solution* sln){
    
    //outfile.open(out_file,ios::out);
    //write them into the file
    outfile<<sln->prob->problemName<<std::endl;
    outfile<<" obj=   "<<  sln->bins.size() << "   " << (int)sln->bins.size() - (int)sln->prob->exampleAnswer <<std::endl;

    for(int i = 0; i < sln->bins.size();i++){
        for (int j = 0 ; j < sln->bins[i].packed_items.size(); j++) {
            outfile<<sln->bins[i].packed_items[j].index <<" ";
        }
        outfile<<std::endl;
    }
    
    //print to the Console
    std::cout<<sln->prob->problemName<<std::endl;
    std::cout <<" obj=   "<< sln->bins.size() << "   " <<(int)sln->bins.size() - (int)sln->prob->exampleAnswer <<std::endl;
    
    for(int i = 0; i < sln->bins.size();i++){
                for (int j = 0 ; j < sln->bins[i].packed_items.size(); j++) {
            std::cout<<sln->bins[i].packed_items[j].index <<" ";
        }
        
        std::cout<<std::endl;
    }

    
}


//A fitness function to evaluate a solution
int fitnessFunction(struct solution* sln){
    int c = sln->prob->C;
    int delta = 0;
    for(int i = 0; i < sln->bins.size(); i++){
        delta = delta + (c - sln->bins[i].cap_left)^2;
    }
    return delta;
}

//Delate a bin with no items inside
void delateEmpty(struct solution* sln){
    for(int i = 0; i < sln->bins.size(); i++){
        if (sln->bins[i].packed_items.empty() == true) {
            sln->bins.erase(sln->bins.begin()+i);
        }
    }
    
}

//Update the capcity left of each bins
struct solution* update_cap_left(struct solution* sln){
    for (int i = 0; i < sln->bins.size(); i++) {
        sln->bins[i].cap_left = sln->prob->C;
        for (int j = 0; j < sln->bins[i].packed_items.size(); j++) {
            sln->bins[i].cap_left = sln->bins[i].cap_left - sln->bins[i].packed_items[j].weight;
        }
    }
    return sln;
}

//The initial solution: FirstFit
struct solution* firstFit(struct problem* prob)
{
    //creat new empty solution
    struct solution* firstFit_sln = create_empty_sol(prob);

    // Initialize result (Count of bins)
    int res = 0;


    // Place items one by one
    for (int i = 0; i < prob->num_of_items; i++) {
        // Find the first bin that can accommodate weight[i]
        int j;
        for (j = 0; j < res; j++) {
            if (firstFit_sln->bins[j].cap_left >= prob->items[i].weight) {
                firstFit_sln->bins[j].cap_left = firstFit_sln->bins[j].cap_left - prob->items[i].weight;
                //put it into the
                firstFit_sln->bins[j].packed_items.push_back(prob->items[i]);
                break;
            }
        }

        // If no bin could accommodate weight[i]
        if (j == res) {
            bin* b = new bin;
            b->cap_left =  prob->C - prob->items[i].weight;
            b->packed_items.push_back(prob->items[i]);
            firstFit_sln->bins.push_back(*b);
            delete b;
            res++;
        }

    }
    return firstFit_sln;
}


//The initial solution: BestFit
struct solution* bestFit(struct problem* prob)
{
    // Initialize result (Count of bins)
    struct solution* bestFit_sln = create_empty_sol(prob);
    int res = 0;
    
    // Place items one by one
    for (int i = 0; i < prob->num_of_items; i++) {
    
        // Find the best bin that can accommodate weight[i]
        int j;

        // Initialize minimum space left and index of best bin
        int min = prob->C + 1, bi = 0;

        for (j = 0; j < res; j++) {
            if (bestFit_sln->bins[j].cap_left >= prob->items[i].weight && bestFit_sln->bins[j].cap_left - prob->items[i].weight < min) {
                bi = j;
                min = bestFit_sln->bins[j].cap_left - prob->items[i].weight;
            }
        }

        // If no bin could accommodate weight[i], create a new bin
        if (min == prob->C + 1) {
            bin* b = new bin;
            b->cap_left =  prob->C - prob->items[i].weight;
            b->packed_items.push_back(prob->items[i]);
            bestFit_sln->bins.push_back(*b);
            delete b;
            res++;
        }
        else {
        // Assign the item to best bin
        bestFit_sln->bins[bi].cap_left -= prob->items[i].weight;
        bestFit_sln->bins[bi].packed_items.push_back(prob->items[i]);
        }
    }
    return bestFit_sln;
}

//Shaking: random swap a pair in different bin and with different size
struct solution* shaking(struct solution* curSolution){
    solution* lackSolution = create_empty_sol(curSolution->prob); // Inside are all the non-full bins
    solution* fullSolution = create_empty_sol(curSolution->prob); // Inside are all the full bins
    
        srand((unsigned int)time(NULL));
        int bestMove[] = {-1,-1,-1,-1}; // this is used to store the index of the pair of items that will be swaped
        //contain the index of the items
        vector<int> bin1;
        vector<int> item1;
        vector<int> bin2;
        vector<int> item2;

        // Separate the empty and full bins of the current solution
    for (int i = 0; i < curSolution->bins.size(); i++) {
        //get all the non-full bins
            if(curSolution->bins[i].cap_left != 0){
                bin* b = new bin;
                b->cap_left = curSolution->prob->C;
                for (int j = 0; j < curSolution->bins[i].packed_items.size() ; j++) {
                    b->cap_left =  b->cap_left  - curSolution->bins[i].packed_items[j].weight;
                    b->packed_items.push_back(curSolution->bins[i].packed_items[j]);
                }
                lackSolution->bins.push_back(*b);
                delete b;

            }else{
                //get all the full binz
                bin* b = new bin;
                b->cap_left = curSolution->prob->C;
                for (int j = 0; j < curSolution->bins[i].packed_items.size() ; j++) {
                    b->cap_left =  b->cap_left - curSolution->bins[i].packed_items[j].weight;
                    b->packed_items.push_back(curSolution->bins[i].packed_items[j]);
                }
                fullSolution->bins.push_back(*b);
                delete b;
            }
        }
        
        
        // search for the feasiable pair of items
        for (int i = 0; i < lackSolution->bins.size(); i++) {
            for (int j = 0 ; j < lackSolution->bins[i].packed_items.size(); j++) {
                for(int x = i+1 ;x< lackSolution->bins.size();x++){
                    for (int y = 0; y < lackSolution->bins[x].packed_items.size(); y++) {
                        //Swap i,j with x,y
                        // Judge whether it exceeds capacity and if this two item has the same weight, that is, whether it can be changed.
                        if (lackSolution->bins[i].cap_left + lackSolution->bins[i].packed_items[j].weight - lackSolution->bins[x].packed_items[y].weight >= 0 && lackSolution->bins[x].cap_left + lackSolution->bins[x].packed_items[y].weight - lackSolution->bins[i].packed_items[j].weight >= 0 &&
                            lackSolution->bins[x].packed_items[y].weight != lackSolution->bins[i].packed_items[j].weight) {
                            // put all the possible swaping pair of items into vectors
                            bin1.push_back(i);
                            item1.push_back(j);
                            bin2.push_back(x);
                            item2.push_back(y);
                        }
                    }
                }
            }
        }
        
    // random pick a pair of item to swap
        int random_num = rand() % bin1.size();
        bestMove[0] = bin1[random_num];
        bestMove[1] = item1[random_num];
        bestMove[2] = bin2[random_num];;
        bestMove[3] = item2[random_num];
        
        //this is the action to do the swap
        lackSolution->bins[bestMove[0]].packed_items.push_back(lackSolution->bins[bestMove[2]].packed_items[bestMove[3]]);
        
        lackSolution->bins[bestMove[2]].packed_items.push_back(lackSolution->bins[bestMove[0]].packed_items[bestMove[1]]);
        
        lackSolution->bins[bestMove[0]].packed_items.erase(lackSolution->bins[bestMove[0]].packed_items.begin() + bestMove[1]);
        
        lackSolution->bins[bestMove[2]].packed_items.erase(lackSolution->bins[bestMove[2]].packed_items.begin() + bestMove[3]);
        
        // after the swap the capcity left must be updated
        lackSolution = update_cap_left(lackSolution);
        

        //Splice the replaced ones and the ones that are full and do not need to be replaced.
        for (int n = 0; n < fullSolution->bins.size(); n++) {
            lackSolution->bins.push_back(fullSolution->bins[n]);
        }
    
    delete fullSolution;
    return lackSolution;

}


//This is the neigbourhood action 1(transfer),this heuristic selects each item from the bin with the largest residual capacity and tries to move the items to the rest of the bins using the best fit descent heuristic.
struct solution* H1(struct solution* sln){
    int index = 0;
    int max_cap_left = 0;
    

    struct solution* tempSln = create_empty_sol(sln->prob);
    tempSln = copy_from(sln);

    // Select the largest bin, and keep the index
    for (int i = 0; i < tempSln->bins.size(); i++) {
        if (tempSln->bins[i].cap_left> max_cap_left ){
            max_cap_left = tempSln->bins[i].cap_left;
            index = i;
        }
    }

    int min_left_bin = 0;
    int min = 100000000;
    int bestmin = 100000000;
    bool flag = false;

    // Start the operation on the bin with the largest remaining space
    for (int j = (int)tempSln->bins[index].packed_items.size() - 1; j >= 0 ; j--) {
        //flag is used to mark if the transfer action can be done
        flag = false;
        min_left_bin = -1;
        min = 100000000;
        bestmin = 100000000;
        // Test each bin to see if it can be exchanged.
        for (int i = 0; i <  tempSln->bins.size(); i++) {
            //jump the current bin
            if (i == index) {
                continue;
            }
            //If there's enough space left in this box, take a look.
            if (tempSln->bins[i].cap_left >= tempSln->bins[index].packed_items[j].weight) {
                // when the capacity left greater than this item size
                min = tempSln->bins[i].cap_left - tempSln->bins[index].packed_items[j].weight;
                if (min < bestmin) {
                    bestmin = min;
                    min_left_bin = i;
                    flag = true;
                }
            }
        }
        // if the transfer can happen, the do it
        if (flag) {
            tempSln->bins[min_left_bin].packed_items.push_back(tempSln->bins[index].packed_items[j]);

            tempSln->bins[index].packed_items.erase(tempSln->bins[index].packed_items.begin()+j);
            
            //update thecapacity left
            tempSln = update_cap_left(tempSln);

            //delate the empty bin
            delateEmpty(tempSln);
            tempSln->feasibility = true;
        }
    }
        
    return tempSln;
}

//This is the neigbourhood action 2(swap), selects the largest item from the bin with the largest residual capacity and exchanges this item with another smaller item
struct solution* H2(struct solution* sln){
    int index = 0;
    int max_cap_left = 0;
    int index_item = 0;
    int max_item = 0;

    struct solution* tempSln = create_empty_sol(sln->prob);
    //update the capacity
    sln = update_cap_left(sln);
    tempSln = copy_from(sln);
    //make the feasibility to be false at first.
    tempSln->feasibility = false;
    
    //Select the largest bin, and store the index.
    for (int i = 0; i < tempSln->bins.size(); i++) {
        if (tempSln->bins[i].cap_left> max_cap_left ){
            max_cap_left = tempSln->bins[i].cap_left;
            index = i;
        }
    }
    //Select the largest item in the largest bin.
    for (int j = 0 ; j < tempSln->bins[index].packed_items.size() ; j++){
        if (tempSln->bins[index].cap_left>max_item ){
            max_item = tempSln->bins[index].cap_left;
            index_item = j;
        }
    }

    int maxGap = -1;
    int gap = -1;
    int bestMove[] = {-1,-1};
    bool flag = false;
    
    // begin to find all the pair that can be swaped
    for (int i = 0 ; i < tempSln->bins.size(); i++) {
        for (int j = 0; j < tempSln->bins[i].packed_items.size(); j++) {
            if (i == index) {
                continue;
            }//first some candition should need
            if (tempSln->bins[index].packed_items[index_item].weight > tempSln->bins[i].packed_items[j].weight ) {
                
                if (tempSln->bins[index].packed_items[index_item].weight -  tempSln->bins[i].packed_items[j].weight <= tempSln->bins[i].cap_left) {
                    gap = tempSln->bins[index].packed_items[index_item].weight -  tempSln->bins[i].packed_items[j].weight;
                    // this step is used to remember the best solution
                    if (gap > maxGap) {
                        maxGap = gap;
                        bestMove[0] = i;
                        bestMove[1] = j;
                        flag = true;
                        
                    }
                }
                
            }
        }
    }

    //if the condition is reached, the swap will begin.
    if (flag) {
        //first push the items into the the end of the bin.
        tempSln->bins[bestMove[0]].packed_items.push_back(tempSln->bins[index].packed_items[index_item]);
        
        tempSln->bins[index].packed_items.push_back(tempSln->bins[bestMove[0]].packed_items[bestMove[1]]);
        
        // get rid of the items
        tempSln->bins[bestMove[0]].packed_items.erase(tempSln->bins[bestMove[0]].packed_items.begin() + bestMove[1]);
        
        tempSln->bins[index].packed_items.erase(tempSln->bins[index].packed_items.begin() + index_item);
       
        //update the capcity left
        tempSln = update_cap_left(tempSln);
        //make the feasibility be ture.
        tempSln->feasibility = true;
        
    }
        
    return tempSln;
    
}

// This heuristic is very similar with the heuristic 2, the only different is that the target bin of this heuristic is the second largest bin, which is the largest bin in heuristic 2.
struct solution* H3(struct solution* sln){
    int index = 0;
    int max_cap_left = 0;
    int index_item = 0;
    int max_item = 0;
    
    int index2 = 0;//index of the second largest pair
    int copy_index = 0;
    struct solution* tempSln = create_empty_sol(sln->prob);
    
    tempSln = copy_from(sln);
    
    tempSln->feasibility =false;
    //Select the largest bin, and store the index.
    for (int i = 0; i < tempSln->bins.size(); i++) {
        if (tempSln->bins[i].cap_left> max_cap_left ){
            max_cap_left = tempSln->bins[i].cap_left;
            index = i;
        }
    }
    
    max_cap_left = 0;
    //find the bin with the second largest capcity left.
    for (int i = 0; i < tempSln->bins.size(); i++) {
        if (i == index) {
            continue;
        }
        if (tempSln->bins[i].cap_left> max_cap_left ){
            max_cap_left = tempSln->bins[i].cap_left;
            index2 = i;
        }
    }
    // store the index of the largest bin
    copy_index = index;
    index = index2;
    
    //find the largset item in the second largest bin
    for (int j = 0 ; j < tempSln->bins[index].packed_items.size() ; j++){
        if (tempSln->bins[index].cap_left>max_item ){
            max_item = tempSln->bins[index].cap_left;
            index_item = j;
        }
    }
        
    int maxGap = -1;
    int gap = -1;
    int bestMove[] = {-1,-1};
    bool flag = false;
    
    // begin to find all the pair that can be swaped
    for (int i = 0 ; i < tempSln->bins.size(); i++) {
        for (int j = 0; j < tempSln->bins[i].packed_items.size(); j++) {
            //this swap process will not involve the bin that are in both largest bin and the scond largest bin.
            if (i == index) {
                continue;
            }
            else if (i == copy_index) {
                continue;
            }
            // check if all the swap condition is fit
            if (tempSln->bins[index].packed_items[index_item].weight > tempSln->bins[i].packed_items[j].weight && tempSln->bins[index].packed_items[index_item].weight -  tempSln->bins[i].packed_items[j].weight <= tempSln->bins[i].cap_left) {
                
                gap = tempSln->bins[index].packed_items[index_item].weight -  tempSln->bins[i].packed_items[j].weight;
                // store the best target item that can be swaped.
                if (gap > maxGap) {
                    maxGap = gap;
                    bestMove[0] = i;
                    bestMove[1] = j;
                    flag = true;
                    
                }
            }
        }
    }
    //if the condition is reached, the swap will begin.
    if (flag) {
        
        //first push the items into the the end of the bin.
        tempSln->bins[bestMove[0]].packed_items.push_back(tempSln->bins[index].packed_items[index_item]);

        tempSln->bins[index].packed_items.push_back(tempSln->bins[bestMove[0]].packed_items[bestMove[1]]);
        
        tempSln->bins[bestMove[0]].packed_items.erase(tempSln->bins[bestMove[0]].packed_items.begin() + bestMove[1]);
        
        tempSln->bins[index].packed_items.erase(tempSln->bins[index].packed_items.begin() + index_item);
        
        //update the capcity left
        tempSln = update_cap_left(tempSln);
        //make the feasibility be ture.
        tempSln->feasibility = true;
    }
    return tempSln;
}


//This is function to control the process of the vns
struct solution* localSearch_vns(int nb_index,struct solution* sln){
    struct solution* answer = create_empty_sol(sln->prob);
    //according to the neighbouthood index, choose the neighbouthood method
    switch (nb_index){
        case 1:
            answer = copy_from(H1(sln));
            break;
        case 2:
            answer = copy_from(H2(sln));
            break;
        case 3:
            answer = copy_from(H3(sln));
            break;
        default:
            cout<<"Neighbourhood index is out of the bounds, nothing is done!"<<endl;
    }
    return answer;
}


int varaible_neighbourhood_search(struct problem* prob){
    best_sln = create_empty_sol(prob); // initailize global optimal solution
    int nb_index = 1; //neighbourhood index
    struct solution* cur_solution = bestFit(prob);//initial solution that are used in this VNS

    struct solution* neighb_s = create_empty_sol(prob);// initailize neighbourhood solution.

    best_sln = copy_from(cur_solution);//Let the initial solution be the optimal solution temporarily.
    
    //this is used to limit the time spanding
    time_t c = time(NULL);
    while(difftime(time (NULL),c)< MAX_TIME){
        nb_index = 1;
        while(nb_index <= K){
            neighb_s = localSearch_vns(nb_index,cur_solution);
            //if the size is of the neighbourhood solution is reduce, the solution should be accepted.
            if (neighb_s->bins.size() < cur_solution->bins.size()) {
                cur_solution = copy_from(neighb_s);
                cur_solution->feasibility = false;
                nb_index = 1;
                continue;
            }
            //if the number of bins is not changed, the feasibility will be put into used to decide of the solution will be accepted.
            else if (neighb_s->feasibility == true) {
                cur_solution = copy_from(neighb_s);
                cur_solution->feasibility = false;
                nb_index = 1;
                continue;
            }
            //if the solution is not accpected, it will go to the next neighbourhood to find a acceptable solution.
            else{
                nb_index++;
            }
        }
        //Only if the bin size is reduced, the solution will be better, so the best solution will be updated only the number of used bins are reduced for this Bin Packing Problem.
        if (cur_solution->bins.size()< best_sln->bins.size() ) {
            best_sln = cur_solution;
        }
        cur_solution = shaking(cur_solution);
    }
    delete neighb_s;
    delete cur_solution;
    return  0;
}


//Main fucntion
int main(int argc, const char * argv[]) {
        // This is used to get some parameters from the terminal, like max time, file name.
        if(argc<3)
        {
            printf("Insufficient arguments. Please use the following options:\n   -s data_file (compulsory)\n   -o out_file (default my_solutions.txt)\n   -c solution_file_to_check\n   -t max_time (in sec)\n");
            return 1;
        }
        else if(argc>9)
        {
            printf("Too many arguments.\n");
            return 2;
        }
        else
        {
            for(int i=1; i<argc; i=i+2)
            {
                if(strcmp(argv[i],"-s")==0)
                    strcpy(data_file, argv[i+1]);
                else if(strcmp(argv[i],"-o")==0)
                    strcpy(out_file, argv[i+1]);
                else if(strcmp(argv[i],"-c")==0)
                    strcpy(solution_file, argv[i+1]);
                else if(strcmp(argv[i],"-t")==0)
                    MAX_TIME = atoi(argv[i+1]);
            }
        }
    
    readInput(data_file);//read the input file
    outfile.open(out_file,ios::out);
    std::cout<<number_of_Problem_Instances<<std::endl;//print out all the number of instances
    outfile<<number_of_Problem_Instances<<std::endl;//write the number of instances into output file
    
    //Run VNS for each instance
    for (int i = 0; i < number_of_Problem_Instances; i++) {
        struct problem* prob = init_prob(i);
        varaible_neighbourhood_search(prob);
        print_sol(best_sln);
        delete prob;
    }
    delete best_sln;
    outfile.close();
    return 0;
}
