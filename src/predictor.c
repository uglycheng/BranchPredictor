//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Zhinan Cheng";
const char *studentID   = "A59002461";
const char *email       = "zcheng@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

// Gshare
int global_history;
uint32_t global_mask;
uint32_t *gshare_history_table;



//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
uint32_t
mask(uint8_t length){
  uint32_t mask = 1;
  for(uint8_t i=1;i<length;i++){
    mask = (mask<<1)+1;
  }
  return mask;
}

uint32_t
calculate_table_size(int bits_number){
  uint32_t size =1;
  size = 1<<bits_number;
  return size;
}

int 
generate_gshare_index(uint32_t address, int historyBits,uint32_t mask){
  int index = (address&mask) ^ (historyBits&mask);
  return index;
}

uint8_t
if_significant_bit_is_1(uint32_t counter){
  return (((counter>>1)&1) == 1); //2-bit counter
}

uint32_t
update_counter(uint32_t counter_value,uint8_t outcome){
  if((outcome==TAKEN) && (counter_value<3)){
    return counter_value+1;
  }else if((outcome==NOTTAKEN) && (counter_value>0)){
    return counter_value-1;
  }else{
    return counter_value;
  }
}

int
update_global_history(int global_history,uint8_t outcome){
  return (global_history<<1)+outcome;
}



void
init_predictor()
{
  switch (bpType) {
    case GSHARE:
      global_history = 0;
      global_mask = mask(ghistoryBits);
      uint32_t table_size = calculate_table_size(ghistoryBits);
      gshare_history_table = (uint32_t*) malloc(sizeof(uint32_t)*table_size);
      for(int i=0;i<table_size;i++){gshare_history_table[i]=1;}
    // case TOURNAMENT:
    // case CUSTOM:
    break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  uint32_t counter_value;
  int global_index;

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      global_index = generate_gshare_index(pc,global_history,global_mask);
      counter_value = gshare_history_table[global_index];
      if(if_significant_bit_is_1(counter_value)){
        return TAKEN;
      }else{
        return NOTTAKEN;
      }
    // case TOURNAMENT:
    // case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  int global_index;
  uint32_t counter_value;
  uint32_t updated_counter;
  
  switch (bpType) {
    case GSHARE:
      
      global_index = generate_gshare_index(pc,global_history,global_mask);
      counter_value = gshare_history_table[global_index];
      updated_counter = update_counter(counter_value,outcome);
      gshare_history_table[global_index] = updated_counter;
      global_history = update_global_history(global_history,outcome);
    // case TOURNAMENT:
    // case CUSTOM:
    default:
      break;
  }
}
