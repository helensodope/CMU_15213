#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "contracts.h"
#include "cachelab.h"

typedef struct cache_line
{
    int valid;
    int tag;
    int ucount;
}line;

//extracts set number from address
int set_index(int s, int b, unsigned address)
{
    unsigned x = address >> b;
    int mask = (1 << s) - 1;
    return (x & mask);
}

//extracts tag
int tag_info(int s, int b, unsigned address)
{
    unsigned x = address >> (s + b);
    return x;
}

//sets up cache
line ** cache_set(int s, int e)
{
    int num_set = (1 << s);

    //initialize 2d array
    line **cache = calloc(num_set, sizeof(line*));

    //initialize each index
    for(int i = 0; i < num_set; i++)
    {
        cache[i] = calloc(e, sizeof(line));
        for(int j = 0; j < e; j++)
        {
            cache[i][j].tag = 0;
            cache[i][j].valid = 0;
            cache[i][j].ucount = 0;
        }
    }
    return cache;
}

//is there an empty space in the set? if there is,
//modify input index as the empty index
bool space_query(line **cache, int set_num, int e, int *index)
{
    for(int i = 0; i < e; i++)
    {
        if(cache[set_num][i].valid == 0)
        {
            *index = i;
            return true;
        }
    }
    return false;
}

//is it a hit or miss? if it is, modify input index as the index of hit
bool hit_or_miss(line **cache, int set_num, int e, int target_tag, int *index)
{
    for(int i = 0; i < e; i++)
    {
        if((cache[set_num][i].tag == target_tag) &&
                cache[set_num][i].valid == 1) 
        {
            *index = i;
            return true;
        }
    }
    return false;
}

//use this function for both load and store
void load_or_store(line **cache, int s, int e, int b, unsigned address, 
                   int *hit, int *miss, int *evict)
{
    //declare local variables
    int empty_index = 0;
    int hit_index = 0;
    int lru_counter = 0;
    int lru_index = 0;

    //extract set index and tag from the given address
    int set_num = set_index(s, b, address);
    int tag = tag_info(s, b, address);

    //increasse ucount for each line in the set
    //its purpose is to caclulate lru_index later in the function
    for(int i = 0; i < e; i++)
    {
        cache[set_num][i].ucount++;
    }

    //is it a hit or miss? if it is a hit, store the hit index in hit_index
    bool hit_miss = hit_or_miss(cache, set_num, e, tag, &hit_index);

    //if it is a hit, set ucount as zero (since now it's not the least recently used)
    //increase hit
    if(hit_miss == true) 
    {
        cache[set_num][hit_index].ucount = 0;
        (*hit)++;
    }


    //if it is a miss, do the following operations
    else
    {
        //increase miss
        (*miss)++;

        //is there an empty line in the set? if there is, save the
        //line index of the empty line in empty_index
        bool is_space = space_query(cache, set_num, e, &empty_index);

        //the set is not full, so input the new tag
        if(is_space)
        {
            cache[set_num][empty_index].valid = 1;
            cache[set_num][empty_index].tag = tag;
            cache[set_num][empty_index].ucount = 0;
        }

        //the set is full, evict!
        else
        {
            //for loop to determine lru_index for eviction
            for(int j = 0; j < e; j++)
            {
                //compare the ucount of each line to lru_counter, a comparison variable
                if(cache[set_num][j].ucount > lru_counter)
                {
                    lru_counter = cache[set_num][j].ucount;
                    lru_index = j;
                }
            }
            //change the tag of the least recently used line
            cache[set_num][lru_index].tag = tag;
            cache[set_num][lru_index].ucount = 0;
            //increase evict
            (*evict)++;
        }
    }
    return;
}

int main(int argc, char **argv)
{
    //declare variables
    int s, e, b;
    int temp;
    FILE *tracefile;
    line **cache;
    char operation;
    unsigned address;
    int hit = 0;
    int miss = 0;
    int evict = 0;

    //read in variables from the command line
    while(-1 != (temp = getopt(argc, argv, "vs:E:b:t:")))
    {
        switch(temp){
            case 'v': printf("lulz i don't have a verbose mode. what do you expect?\n");
                      break;
            case 's': s = atoi(optarg);
                      break;
            case 'E': e = atoi(optarg);
                      break;
            case 'b': b = atoi(optarg);
                      break;
            case 't': tracefile = fopen(optarg, "r");
                      break;
            default: printf("the fat unicorn sneezed. exiting..\n");
                     break;
        }
    }

    //initialize new cache
    cache = cache_set(s, e);

    //depending on the operation, do one or two load_or_store
    while(fscanf(tracefile, "%c %x", &operation, &address) > 0)
    {
        if((operation == 'S') || (operation == 'L'))
        {
            load_or_store(cache, s, e, b, address, &hit, &miss, &evict);
        }
        if(operation == 'M')
        {
            load_or_store(cache, s, e, b, address, &hit, &miss, &evict);
            load_or_store(cache, s, e, b, address, &hit, &miss, &evict);
        }
    }

    //close file
    fclose(tracefile);
    
    //free what was allocated
    for(int i = 0; i < (1 << s); i++)
    {
        free(cache[i]);
    }
    free(cache);

    //print out results
    printSummary(hit, miss, evict);
    return 0;
}
