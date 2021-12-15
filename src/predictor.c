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
const char *bpName[6] = { "Static", "Gshare",
                          "Tournament", "Custom","TE","TG" };

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

//Selective
uint8_t *correct_ht;
uint8_t correct_mask;
uint8_t correct_history_length;
uint8_t count_1_threshold;





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
  // return (uint32_t) ((1<<(length+1)) -1);
}

uint32_t
calculate_table_size(int bits_number){
  uint32_t size =1;
  size = 1<<bits_number;
  return size;
  // return (uint32_t) 1<<bits_number;
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

uint8_t
count_1(uint8_t correct_his,uint8_t correct_history_length){
  uint8_t count = 0;
  for(int i=0;i<correct_history_length;i++){
    if(correct_his&1){
      count += 1;
    }
    correct_his = correct_his >>1;
  }
  return count;
}

uint8_t
update_correct_his(uint8_t correct_his,uint8_t mask,uint8_t is_correct){
  return (uint8_t) (((correct_his<<1) & mask) + (is_correct&1));
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

    case TG:
      ghistoryBits = 14; 
      lhistoryBits = 10; 
      pcIndexBits = 10; 
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
    case CUSTOM:
      ghistoryBits = 13; 
      lhistoryBits = 10; 
      pcIndexBits = 10; 
      correct_history_length = 4;
      count_1_threshold = 2;
      global_mask = mask(ghistoryBits);
      table_size = calculate_table_size(ghistoryBits);
      //Estimator
      correct_ht = (uint8_t*) malloc(sizeof(uint8_t)*table_size);
      for(int i=0;i<table_size;i++){correct_ht[i]=1;}
      correct_mask = mask(correct_history_length);
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
    case TE:
      ghistoryBits = 12; 
      lhistoryBits = 10; 
      pcIndexBits = 10; 
      correct_history_length = 8;
      count_1_threshold = 1;
      global_mask = mask(ghistoryBits);
      table_size = calculate_table_size(ghistoryBits);
      //Estimator
      correct_ht = (uint8_t*) malloc(sizeof(uint8_t)*table_size);
      for(int i=0;i<table_size;i++){correct_ht[i]=1;}
      correct_mask = mask(correct_history_length);
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

  uint8_t correct_his;
  uint8_t correct_his_count_1;
  int gshare_index;


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
    case TG:
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
    case CUSTOM:
      global_index = generate_gshare_index(pc,global_history,global_mask);
      correct_his = correct_ht[global_index];
      correct_his_count_1 = count_1(correct_his,correct_history_length);
      counter_value = choicer_table[global_index];
      if((if_significant_bit_is_1(counter_value) && correct_his_count_1>=count_1_threshold) || (!(if_significant_bit_is_1(counter_value) || correct_his_count_1>=count_1_threshold))){
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
    case TE:
      global_index = generate_masked_index(global_history,global_mask);
      correct_his = correct_ht[global_index];
      correct_his_count_1 = count_1(correct_his,correct_history_length);
      counter_value = choicer_table[global_index];
      if((if_significant_bit_is_1(counter_value) && correct_his_count_1>=count_1_threshold) || (!(if_significant_bit_is_1(counter_value) || correct_his_count_1>=count_1_threshold))){
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

  uint8_t correct_his;
  uint8_t correct_his_count_1;
  uint8_t updated_correct_his;
  int gshare_index;
  uint8_t final_prediction;
  
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
    case TG:
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
        correct_his = correct_ht[global_index];
        correct_his_count_1 = count_1(correct_his,correct_history_length);
        if(if_significant_bit_is_1(choice)){
          final_prediction = global_prediction;
        }else{
            final_prediction = local_prediction;
        }
          updated_correct_his = update_correct_his(correct_his,correct_mask,final_prediction==outcome);
          // printf("pc:%d,glo:%d,loc:%d,choice:%d,outcome:%d,estimator:%d,correct:%d\n",pc,global_prediction,local_prediction,if_significant_bit_is_1(choice),outcome,correct_his_count_1>=count_1_threshold,updated_correct_his&1);
          correct_ht[global_index] = updated_correct_his;
      }  
      break;   
    case TE:

      //Update Global
      global_index = generate_masked_index(global_history,global_mask);
      counter_value = global_pht[global_index];
      updated_counter = update_counter(counter_value,outcome);
      global_pht[global_index] = updated_counter;
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
        correct_his = correct_ht[global_index];
        correct_his_count_1 = count_1(correct_his,correct_history_length);
        if(if_significant_bit_is_1(choice)){
          final_prediction = global_prediction;
        }else{
            final_prediction = local_prediction;
        }
          updated_correct_his = update_correct_his(correct_his,correct_mask,final_prediction==outcome);
          correct_ht[global_index] = updated_correct_his;
      }  
      break;   


    default:
      break;
  }
}
