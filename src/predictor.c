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
uint8_t *gshare_history_table;

//Tournament
uint32_t local_mask;
uint32_t pc_mask;
uint8_t *global_pht;
uint8_t *local_pht;
uint32_t *local_bht;
uint8_t *choicer_table;




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
  // uint32_t size =1;
  // size = 1<<bits_number;
  // return size;
  return (uint32_t) 1<<bits_number;
}

int 
generate_gshare_index(uint32_t address, int historyBits,uint32_t mask){
  int index = (address&mask) ^ (historyBits&mask);
  return index;
}

uint8_t
if_significant_bit_is_1(uint8_t counter){
  return (((counter>>1)&1) == 1); //2-bit counter
}

uint8_t
update_counter(uint8_t counter_value,uint8_t outcome){
  if((outcome==TAKEN) && (counter_value<3)){
    return counter_value+1;
  }else if((outcome==NOTTAKEN) && (counter_value>0)){
    return counter_value-1;
  }else{
    return counter_value;
  }
}

int
update_history(int global_history,uint8_t outcome){
  return (global_history<<1)+outcome;
}

int 
generate_masked_index(int historyBits,uint32_t mask){
  return historyBits & mask;
}


void
init_predictor()
{
  global_history = 0;
  global_mask = mask(ghistoryBits);
  uint32_t table_size = calculate_table_size(ghistoryBits);
  switch (bpType) {
    case GSHARE:
      gshare_history_table = (uint8_t*) malloc(sizeof(uint8_t)*table_size);
      for(int i=0;i<table_size;i++){gshare_history_table[i]=1;}
      break;
    case TOURNAMENT:
      //Choicer
      choicer_table = (uint8_t*) malloc(sizeof(uint8_t)*table_size);
      for(int i=0;i<table_size;i++){choicer_table[i]=2;}
      //Global
      global_pht = (uint8_t*) malloc(sizeof(uint8_t)*table_size);
      for(int i=0;i<table_size;i++){global_pht[i]=1;}
      //Local
      local_mask = mask(lhistoryBits);
      pc_mask = mask(pcIndexBits);
      table_size = calculate_table_size(lhistoryBits);
      local_pht = (uint8_t*) malloc(sizeof(uint8_t)*table_size);
      for(int i=0;i<table_size;i++){local_pht[i]=1;}
      table_size = calculate_table_size(pcIndexBits);
      local_bht = (uint32_t*) malloc(sizeof(uint32_t)*table_size);
      for(int i=0;i<table_size;i++){local_bht[i]=0;}
      break;

    case CUSTOM:
      ghistoryBits = 13; 
      lhistoryBits = 11; 
      pcIndexBits = 11; 
      global_mask = mask(ghistoryBits);
      table_size = calculate_table_size(ghistoryBits);
      //Choicer
      choicer_table = (uint8_t*) malloc(sizeof(uint8_t)*table_size);
      for(int i=0;i<table_size;i++){choicer_table[i]=2;}
      //Global
      gshare_history_table = (uint8_t*) malloc(sizeof(uint8_t)*table_size);
      for(int i=0;i<table_size;i++){gshare_history_table[i]=1;}
      //Local
      local_mask = mask(lhistoryBits);
      pc_mask = mask(pcIndexBits);
      table_size = calculate_table_size(lhistoryBits);
      local_pht = (uint8_t*) malloc(sizeof(uint8_t)*table_size);
      for(int i=0;i<table_size;i++){local_pht[i]=1;}
      table_size = calculate_table_size(pcIndexBits);
      local_bht = (uint32_t*) malloc(sizeof(uint32_t)*table_size);
      for(int i=0;i<table_size;i++){local_bht[i]=0;}
      break;

    // break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  uint8_t counter_value;
  int global_index;
  uint8_t prediction;
  int local_pc_index;
  int local_pattern_index;


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

    case TOURNAMENT:
      global_index = generate_masked_index(global_history,global_mask);
      counter_value = choicer_table[global_index];
      if(if_significant_bit_is_1(counter_value)){
        prediction = global_pht[global_index];
      }else{
        local_pc_index = generate_masked_index(pc,pc_mask);
        local_pattern_index = generate_masked_index(local_bht[local_pc_index],local_mask);
        prediction = local_pht[local_pattern_index]; 
      }
      if(if_significant_bit_is_1(prediction)){
        return TAKEN;
      }else{
        return NOTTAKEN;
      }
    case CUSTOM:
      global_index = generate_gshare_index(pc,global_history,global_mask);
      counter_value = choicer_table[global_index];
      if(if_significant_bit_is_1(counter_value)){
        prediction = gshare_history_table[global_index];
      }else{
        local_pc_index = generate_masked_index(pc,pc_mask);
        local_pattern_index = generate_masked_index(local_bht[local_pc_index],local_mask);
        prediction = local_pht[local_pattern_index]; 
      }
      if(if_significant_bit_is_1(prediction)){
        return TAKEN;
      }else{
        return NOTTAKEN;
      }


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
  uint8_t counter_value;
  uint8_t updated_counter;

  uint8_t local_prediction;
  uint8_t global_prediction;
  uint8_t local_counter;
  uint8_t global_counter;
  uint8_t updated_global_counter;
  uint8_t updated_local_counter;
  uint8_t choice;
  uint8_t updated_choice;
  int local_pc_index;
  int local_pattern_index;
  
  switch (bpType) {
    case GSHARE:
      
      global_index = generate_gshare_index(pc,global_history,global_mask);
      counter_value = gshare_history_table[global_index];
      updated_counter = update_counter(counter_value,outcome);
      gshare_history_table[global_index] = updated_counter;
      global_history = update_history(global_history,outcome);
      break;
    case TOURNAMENT:
      //Update Global
      global_index = generate_masked_index(global_history,global_mask);
      global_counter = global_pht[global_index];
      updated_global_counter = update_counter(global_counter,outcome);
      global_pht[global_index] = updated_global_counter;
      global_history = update_history(global_history,outcome);
      //Update Local
      local_pc_index = generate_masked_index(pc,pc_mask);
      local_pattern_index = generate_masked_index(local_bht[local_pc_index],local_mask);
      local_counter = local_pht[local_pattern_index];
      updated_local_counter = update_counter(local_counter,outcome);
      local_pht[local_pattern_index] = updated_local_counter;
      local_bht[local_pc_index] = update_history(local_bht[local_pc_index],outcome);
      //Update Choice
      choice = choicer_table[global_index];
      if(if_significant_bit_is_1(global_counter)){
        global_prediction = TAKEN;
      }else{
        global_prediction = NOTTAKEN;
      }
      if(if_significant_bit_is_1(local_counter)){
        local_prediction = TAKEN;
      }else{
        local_prediction = NOTTAKEN;
      }
      if(global_prediction != local_prediction){
        updated_choice = update_counter(choice,global_prediction==outcome);
        choicer_table[global_index] = updated_choice;
      }  
      break;   
    case CUSTOM:
      //Update Global
      global_index = generate_gshare_index(pc,global_history,global_mask);
      counter_value = gshare_history_table[global_index];
      updated_counter = update_counter(counter_value,outcome);
      gshare_history_table[global_index] = updated_counter;
      global_history = update_history(global_history,outcome);
      //Update Local
      local_pc_index = generate_masked_index(pc,pc_mask);
      local_pattern_index = generate_masked_index(local_bht[local_pc_index],local_mask);
      local_counter = local_pht[local_pattern_index];
      updated_local_counter = update_counter(local_counter,outcome);
      local_pht[local_pattern_index] = updated_local_counter;
      local_bht[local_pc_index] = update_history(local_bht[local_pc_index],outcome);
      //Update Choice
      choice = choicer_table[global_index];
      if(if_significant_bit_is_1(counter_value)){
        global_prediction = TAKEN;
      }else{
        global_prediction = NOTTAKEN;
      }
      if(if_significant_bit_is_1(local_counter)){
        local_prediction = TAKEN;
      }else{
        local_prediction = NOTTAKEN;
      }
      if(global_prediction != local_prediction){
        updated_choice = update_counter(choice,global_prediction==outcome);
        choicer_table[global_index] = updated_choice;
      }  
      break;   

    default:
      break;
  }
}
