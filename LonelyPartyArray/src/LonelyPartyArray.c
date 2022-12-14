/*
  Charlton Trezevant
  Dr.Sean Szumlanski
  COP 3502 - Sept 2018
  
  Program #2 - Lonely Party Array
  
  ---
  There once was a young boy named Alec.
  He wanted a string so he'd malloc.
  "Oh the bytes that I'll free!"
  He cried out with glee.
  Segmentation fault
  
  -Unknown Author
*/

#include<stdio.h>
#include<stdlib.h>
#include "LonelyPartyArray.h"

//#define DEBUG
#ifdef DEBUG
  #define debugf(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__); fflush(stderr)
#else
  #define debugf(fmt, ...) ((void)0)
#endif

int index_is_valid(int index, LonelyPartyArray *party);
int index_exists(int index, LonelyPartyArray *party);
int create_fragment(int fragment_index, LonelyPartyArray *party);
int fragment_exists(int index, LonelyPartyArray *party);
int get_lpa_fragment_index(int index, LonelyPartyArray *party);
int get_fragment_cell_index(int index, LonelyPartyArray *party);
int destroy_fragment(int fragment_index, LonelyPartyArray *party);

LonelyPartyArray *createLonelyPartyArray(int num_fragments, int fragment_length)
{
  int max_cap, i;
  LonelyPartyArray *LPA = malloc(sizeof(LonelyPartyArray));
  
  debugf("(createLonelyPartyArray) Allocated space for LPA: %d at address %p\n", (int)sizeof(LonelyPartyArray), LPA);
  
  if(num_fragments <= 0 || fragment_length <= 0){
    debugf("(createLonelyPartyArray) Terminated early: num_fragments[%d] or fragment_length[%d] were <= 0\n", num_fragments, fragment_length);
    return NULL;
  }
  
  max_cap = num_fragments * fragment_length;
  debugf("(createLonelyPartyArray) Computed maximum capacity for LPA: %d\n", max_cap);
  
  LPA->num_fragments = num_fragments;
  LPA->fragment_length = fragment_length;
  LPA->num_active_fragments = 0;
  LPA->size = 0;
    
  LPA->fragments = malloc(sizeof(int *) * num_fragments);
  debugf("(createLonelyPartyArray) Allocated %d to hold %d fragments in LPA->fragments at address %p\n", (int)(sizeof(int *) * num_fragments), num_fragments, LPA->fragments);
  
  LPA->fragment_sizes = malloc(sizeof(int) * num_fragments);
  debugf("(createLonelyPartyArray) Allocated %d to hold the sizes of %d fragments in LPA->fragment_sizes at address %p\n", (int)(sizeof(int) * num_fragments), num_fragments, LPA->fragment_sizes);

  if(LPA->fragments == NULL){
    debugf("(createLonelyPartyArray) Terminated early due to malloc failure! LPA->fragments is NULL\n");
    return NULL;
  }
  
  debugf("(createLonelyPartyArray) LPA fragments will now be initialized:\n");
  for(i = 0; i < num_fragments; i++){
    debugf("(createLonelyPartyArray) \tInitializing fragment %d (of %d members) to NULL\n", i, num_fragments);
    LPA->fragments[i] = NULL;
    
    debugf("(createLonelyPartyArray) \tUpdating fragment size table for ID %d\n", i);
    LPA->fragment_sizes[i] = 0;
  }
  
  printf("-> A new LonelyPartyArray has emerged from the void. (capacity: %d, fragments: %d)\n", max_cap, num_fragments);
  
  debugf("\n");
  return LPA;
}

LonelyPartyArray *destroyLonelyPartyArray(LonelyPartyArray *party)
{
  int i, j;
    
  if (party == NULL){
    debugf("(destroyLonelyPartyArray) Terminating early because party is NULL");
    debugf("\n");
    return NULL;
  }
  
  debugf("(destroyLonelyPartyArray) LPA struct members will now be free()'d:\n");
  for(i = 0; i < party->num_fragments; i++){
    destroy_fragment(i, party);
  }
  
  debugf("(destroyLonelyPartyArray) Freeing LPA fragment array at address %p\n", &(party->fragment_sizes[i]));
  free(party->fragments);
  
  debugf("(destroyLonelyPartyArray) Freeing fragment size counter at address %p\n", &(party->fragment_sizes[i]));
  free(party->fragment_sizes);
  
  debugf("(destroyLonelyPartyArray) Freeing LPA at address[%p]\n", party);
  free(party);
  
  printf("-> The LonelyPartyArray has returned to the void.\n");
  
  debugf("\n");
  return NULL;
}

int set(LonelyPartyArray *party, int index, int key)
{
  int fragment_index, index_localized, was_empty = 0;
  
  if(party == NULL){
    debugf("(set) Terminating early because LPA is NULL\n");
    debugf("\n");
    return LPA_FAILURE;
  }
  
  fragment_index = get_lpa_fragment_index(index, party);

  debugf("(set) Computed index of target fragment %d from provided index of %d\n", fragment_index, index);
  
  if(index_is_valid(index, party) < 0){
    debugf("(set) Terminating early because index %d is invalid\n", index);
    debugf("\n");
    printf("-> Bloop! Invalid access in set(). (index: %d, fragment: %d, offset: %d)\n", index, fragment_index, get_fragment_cell_index(index, party));
    return LPA_FAILURE;
  }
  
  if(fragment_exists(fragment_index, party) < 0)
    if(create_fragment(fragment_index, party) < 0){
      debugf("(set) Terminating early because we FAILED to create a new fragment at LPA->fragments[%d] for the provided index %d\n", fragment_index, index);
      debugf("\n");
      return LPA_FAILURE;
    }
  
  index_localized = index % party->fragment_length;
  
  if(party->fragments[fragment_index][index_localized] == UNUSED)
    was_empty = 1;
  else
    debugf("(set) INFO: Cell %d in fragment %d was not empty when we were called\n", index_localized, fragment_index);
    
  debugf("(set) Storing value %d at index %d of fragment %d (index %d overall)...\n", key, index_localized, fragment_index, index);
  party->fragments[fragment_index][index_localized] = key;
  
  if(was_empty == 1){
    party->fragment_sizes[fragment_index]++;
    debugf("(set) Size of fragment %d updated to %d (from %d)\n", fragment_index, party->fragment_sizes[fragment_index], party->fragment_sizes[fragment_index]-1);
    party->size++;
    debugf("(set) Overall LPA size updated to %d (from %d)\n", party->size, party->size-1);
  }
  
  debugf("\n");
  return LPA_SUCCESS;
}

int get(LonelyPartyArray *party, int index)
{
  int fragment_index, index_localized;
  
  if(party == NULL){
    printf("-> Bloop! NULL pointer detected in get().\n");
    debugf("(set) Terminating early because LPA is NULL\n");
    debugf("\n");
    return LPA_FAILURE;
  }
  
  fragment_index = get_lpa_fragment_index(index, party);
    
  debugf("(get) Computed index of target fragment %d from provided index of %d\n", fragment_index, index);
  
  if(index_is_valid(index, party) < 0){
    debugf("(get) Terminating early because index %d is invalid\n", index);
    debugf("\n");
    printf("-> Bloop! Invalid access in get(). (index: %d, fragment: %d, offset: %d)\n", index, fragment_index, get_fragment_cell_index(index, party));
    return LPA_FAILURE;
  }

  if(fragment_exists(fragment_index, party) < 0){
    debugf("(get) Nothing exists at index %d\n", index);
    debugf("\n");
    return UNUSED;
  }
  
  index_localized = index % party->fragment_length;
  debugf("\n");
  return party->fragments[fragment_index][index_localized];
}

int index_is_valid(int index, LonelyPartyArray *party)
{
  if(index >= 0 && index <= (party->num_fragments * party->fragment_length - 1)){
    return 0;
  } else {
    debugf("(index_is_valid) WARNING: index %d does not fall within the bounds of [0, %d] for LPA\n", index, (party->num_fragments * party->fragment_length - 1));
    debugf("\n");
    return -1;
  }
}

LonelyPartyArray *resetLonelyPartyArray(LonelyPartyArray *party)
{
  int i;
  
  if(party == NULL){
    printf("-> Bloop! NULL pointer detected in resetLonelyPartyArray().\n");
    return NULL;
  }
  
  for(i = 0; i < party->num_active_fragments; i++){
    if(party->fragment_sizes[i] > 0){
      destroy_fragment(i, party);
      party->fragment_sizes[i] = 0;
    }
  }
  party->num_active_fragments = 0;
  
  printf("-> The LonelyPartyArray has returned to its nascent state. (capacity: %d, fragments: %d)\n", party->num_fragments * party->fragment_length, party->num_fragments);
  return party;
}

int isSet(LonelyPartyArray *party, int index)
{
  int val = get(party, index);
  
  if(party != NULL && val != LPA_FAILURE && val != UNUSED)
    return 1;
  else
    return 0;
}

int printIfValid(LonelyPartyArray *party, int index)
{
  
  if(isSet(party, index) == 0)
    return LPA_FAILURE;
    
  printf("%d\n", get(party, index));
  return LPA_SUCCESS;
}

int getSize(LonelyPartyArray *party)
{
  return (party != NULL) ? party->size : -1;
}

int getCapacity(LonelyPartyArray *party)
{
  if(party != NULL)
    return party->num_fragments * party->fragment_length;
  else
    return -1;
}

int getAllocatedCellCount(LonelyPartyArray *party)
{
  if(party != NULL)
    return party->num_active_fragments * party->fragment_length;
  else
    return -1;
}

long long unsigned int getArraySizeInBytes(LonelyPartyArray *party)
{
  if(party != NULL)
    return getCapacity(party) * sizeof(int);
  else
    return 0;
}

long long unsigned int getCurrentSizeInBytes(LonelyPartyArray *party){
  if(party != NULL)
    return sizeof(LonelyPartyArray*)
    + sizeof(party)
    + sizeof(party->fragments)
    + sizeof(party->fragment_sizes)
    + (sizeof(int) * getAllocatedCellCount(party));
  else
    return 0;
}

int delete(LonelyPartyArray *party, int index)
{
  int fragment_index, index_localized;
    
  if(party == NULL){
    debugf("(delete) Terminating early because party is NULL\n");
    printf("-> Bloop! NULL pointer detected in delete().");
  }
  
  if(index_is_valid(index, party) < 0){
    debugf("(delete) Terminating early because index is invalid\n");
    debugf("\n");
    return LPA_FAILURE;
  }
  
  fragment_index = get_lpa_fragment_index(index, party);
  debugf("(delete) Checking viability of requested fragment index %d (index %d overall)\n", fragment_index, index);
  
  for(int z = 0; z < party->num_fragments; z++)
    printf("]]]] Size of fragment %d: %d\n", z, party->fragment_sizes[z]);
  
  if(fragment_exists(fragment_index, party) < 0){
    debugf("(delete) Nothing exists at index %d\n", index);
    debugf("\n");
    printf("-> Bloop! Invalid access in delete(). (index: %d, fragment: %d, offset: %d)\n", fragment_index, party->fragment_sizes[fragment_index], get_fragment_cell_index(index, party));
    return LPA_FAILURE;
  }
  
  index_localized = get_fragment_cell_index(index, party);
  
  if(party->fragments[fragment_index][index_localized] == UNUSED){
    debugf("(delete) Nothing to delete at index %d (space marked as UNUSED)\n", index);
    debugf("\n");
    return LPA_FAILURE;
  }
  
  debugf("(delete) Overwrote value at LPA index %d in fragment %d (cell %d in fragment): old: %d, new: %d\n", index, fragment_index, index_localized, party->fragments[fragment_index][index_localized], UNUSED);
  party->fragments[fragment_index][index_localized] = UNUSED;
  
  party->size--;
  debugf("(delete) Overall size counter for LPA updated to %d (from %d)\n", party->size, party->size+1);
  
  debugf("(delete) New size of fragment %d is: %d\n", fragment_index, party->fragment_sizes[fragment_index]);
  
  party->fragment_sizes[fragment_index]--;
  debugf("(delete) Size counter for fragment %d updated to %d (from %d). %d total fragments active in LPA overall\n", fragment_index, party->fragment_sizes[fragment_index], party->fragment_sizes[fragment_index]+1, party->num_active_fragments);
  
  if(party->fragment_sizes[fragment_index] <= 0){
    debugf("(delete) Fragment %d emptied and marked for destruction\n", fragment_index);
    destroy_fragment(fragment_index, party);
    printf("-> Deallocated fragment %d. (capacity: %d, indices: %d..%d)\n", fragment_index, party->fragment_sizes[fragment_index], (party->fragment_length * (fragment_index+1)), (party->fragment_length * (fragment_index+1)-1));
  }
  
  debugf("\n");
  return LPA_SUCCESS;
}

int destroy_fragment(int fragment_index, LonelyPartyArray *party)
{
  int j;
  
  if(party->fragments[fragment_index] == NULL){
    debugf("(destroy_fragment) \tFragment at index %d is NULL (won't attempt to free() it)\n", fragment_index);
    debugf("\n");
    return -1;
  }
  
  debugf("(destroy_fragment) \tFreeing %d members of LPA->fragments[%d]\n", party->fragment_sizes[fragment_index], fragment_index);
  for(j = 0; j < party->fragment_sizes[fragment_index] - 1; j++){
      debugf("(destroy_fragment) \t\tFreeing member %d of LPA->fragments[%d] at address %p\n", j, fragment_index, &(party->fragments[fragment_index][j]));
      free(&(party->fragments[fragment_index][j]));
  }
  
  debugf("(destroy_fragment) Destroyed fragment at index %d\n", fragment_index);
  
  party->num_active_fragments--;
  debugf("(destroy_fragment) Active fragment counter updated to %d (from %d)\n", party->num_active_fragments, party->num_active_fragments+1);
  
  debugf("\n");
  return 0;
}

int get_fragment_cell_index(int index, LonelyPartyArray *party){
  return index % party->fragment_length;
}

int get_lpa_fragment_index(int index, LonelyPartyArray *party){
  int target_fragment = index / (party->fragment_length);
  
  debugf("(get_lpa_fragment_index) Matched provided index %d to container fragment %d\n", index, target_fragment);
  
  return target_fragment;
}

int fragment_exists(int fragment_index, LonelyPartyArray *party)
{
  if((fragment_index+1) > party->num_fragments || party->fragments[fragment_index] == NULL){
    debugf("(fragment_exists) INFO: fragment at index %d does not exist for LPA (but it may be created soon)\n", fragment_index);
    debugf("\n");
    return -1;
  }
  
  debugf("\n");
  return 0;
}

int create_fragment(int fragment_index, LonelyPartyArray *party)
{
  int i;
  party->fragments[fragment_index] = malloc(sizeof(int) * party->fragment_length);
  debugf("(create_fragment) Allocated %d to LPA->fragments[%d] to store %d ints at address %p\n", (int)(sizeof(int) * party->fragment_length), fragment_index, party->fragment_length, party->fragments[fragment_index]);
  
  if(party->fragments[fragment_index] == NULL){
    debugf("(create_fragment) Terminating early because malloc() FAILED for fragment_index %d\n", fragment_index);
    debugf("\n");
    return -1;
  }
  
  debugf("(create_fragment) Fragment at index %d will now be initialized\n", fragment_index);
  for(i = 0; i < party->fragment_length; i++)
    party->fragments[fragment_index][i] = UNUSED;
  
  party->num_active_fragments++;
  printf("-> Spawned fragment %d. (capacity: %d, indices: %d..%d)\n", fragment_index, party->fragment_length, (party->fragment_length * (fragment_index+1)), (party->fragment_length * (fragment_index+1)-1));
  debugf("(create_fragment) Active fragment counter updated to %d (from %d)\n", party->num_active_fragments, party->num_active_fragments-1);
  
  debugf("\n");
  return 0;
}

double difficultyRating(void)
{
  return 5.0;
}

double hoursSpent(void)
{
  return 5.0;
}
