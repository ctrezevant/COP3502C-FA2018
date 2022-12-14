// Charlton Trezevant
// Sean Szumlanski
// COP 3502C
// Assignment #3 - ListyString.c

/*
	A few notes for the Grader,
	
	- If you like, I can try to write a proper command to strip out Atom's space
	indentation and replace it with actual tabs. I didn't have enough time to figure
	that out this time around, because by the time I discovered how helpfully Atom had
  indented my lines for me, I had already written the majority of this code.
	
	- Secondly, if there's any interest in my nifty debugf macro, I'd be more than happy to
	share it with others. I've built in full support for variadic arguments, so it'll
	work just like printf (*and* flush your output after each call). Additionally, it
	doesn't affect Szum's test cases (since all of its output is sent to stderr).
	
	- Thirdly, ever since I picked up Go I've gotten somewhat tired of manually enforcing
	style restrictions, so if you could ask whoever's in charge of Eustis to please install
	clang-format utility, I can help get Szum's styleguide distilled into a format file so
	that all of the styling and refactoring can be automatically performed. Incidentally,
	this would also take care of any reformatting required for Atom or other text editors.
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include "ListyString.h"

//#define DEBUG
#ifdef DEBUG
  #define debugf(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__); fflush(stderr)
#else
  #define debugf(fmt, ...) ((void)0)
#endif

// Helper functions
void print_usage(char *name);
char *listy_to_string(ListyString *list);
ListyNode *get_listy_tail(ListyString *list);

int main(int argc, char **argv)
{
  // Handle cases where too few arguments are provided, so we don't segfault.
	if (argc < 2 || argv[1] == NULL)
  {
    debugf("(main) Error: Invalid arguments\n");
		print_usage(argv[0]);
		return 1;
	}
  
  return processInputFile(argv[1]);
}

int processInputFile(char *filename)
{
  FILE *ifp;
  char *raw_string, cmd, key;
  ListyString *listy;
  
  ifp = fopen(filename, "r");
  
  if (ifp == NULL)
  {
    debugf("(processInputFile) [1] Invalid filename provided.\n");
    return 1;
  }
	
	raw_string = malloc((sizeof(char) * 1024) + 1);
	fscanf(ifp, "%s", raw_string);
	
	listy = createListyString(raw_string);
	
	// Handle the odd, unlikely failure case here and there
	if (listy == NULL)
	{
		debugf("(processInputFile) [1] ListyString creation failed.\n");
		return 1;
	}
	
	// Our main file processing loop, which breaks at EOF
	while (!feof(ifp))
	{
		fscanf(ifp, "%c", &cmd);
		
		switch (cmd)
		{
			case '@':
				fscanf(ifp, "%c", &key);
				fscanf(ifp, "%c", &key);
				fscanf(ifp, "%s", raw_string);
				debugf("(processInputFile) Read command: %c,%c,%s [replace]\n", cmd, key, raw_string);
				replaceChar(listy, key, raw_string);
				break;
			
			case '+':
				fscanf(ifp, "%s", raw_string);
				debugf("(processInputFile) Read command: %c,%s [concatenate]\n", cmd, raw_string);
				listyCat(listy, raw_string);
				break;
			
			case '-':
				fscanf(ifp, "%c", &key);
				fscanf(ifp, "%c", &key);
				debugf("(processInputFile) Read command: %c,%c [delete]\n", cmd, key);
				replaceChar(listy, key, "");
				break;
				
			case '~':
				debugf("(processInputFile) Read command: %c [reverse]\n", cmd);
				reverseListyString(listy);
				break;
				
			case '?':
				debugf("(processInputFile) Read command: %c [print length]\n", cmd);
				printf("%d\n", listyLength(listy));
				break;
				
			case '!':
				debugf("(processInputFile) Read command: %c [print listystring]\n", cmd);
				printListyString(listy);
				break;
		}
	}
  
	free(raw_string);
  return 0;
}

ListyString *createListyString(char *str)
{
  ListyString *list;
  ListyNode *prev_node, *tmp;
  int i;

  debugf("(createListyString) --- enter\n");
  
  list = malloc(sizeof(ListyString));
  list->length = 0;
  list->head = NULL;
  debugf("(createListyString) malloc'd %d for a ListyString of length %d\n", (int)(sizeof(ListyString)), list->length);
  
  if (str == NULL)
	{
    debugf("(createListyString) [empty list] Recieved a NULL input string\n");
    debugf("(createListyString) --- exit\n");
    return list;
  }
  
  list->length = strlen(str);
  
	// Return a NULL-initialized ListyString for an empty input string
  if (list->length < 1)
	{
    debugf("(createListyString) [empty list] Recieved an input string with a length of 0\n");
    debugf("(createListyString) --- exit\n");
    return list;
  }

  debugf("(createListyString) LENGTH UPDATE: Populating ListyString with %d nodes\n", list->length);
	
	// If we have a sizeable enough input string, we can populate the ListyString with some nodes.
  prev_node = malloc(sizeof(ListyNode));
  list->head = prev_node;
  list->head->data = str[0];
  debugf("(createListyString) malloc'd %d for the ListyString HEAD node at index %d to hold %c\n", (int)(sizeof(ListyNode)), 0, str[0]);
  
  for (i = 1; i < list->length; i++)
  {
    tmp = malloc(sizeof(ListyNode));
    debugf("(createListyString) malloc'd %d for a ListyString node at index %d to hold %c\n", (int)(sizeof(ListyNode)), i, str[i]);
    
    tmp->data = str[i];
    tmp->next = NULL;
    
    prev_node->next = tmp;
    prev_node = tmp;
  }

  debugf("(createListyString) [new list] Triumphantly returning new ListyString of length %d\n", list->length);
  debugf("(createListyString) --- exit\n");
  return list;
}

ListyString *destroyListyString(ListyString *listy)
{
  ListyNode *next_node, *tmp;
  int i;
  
  debugf("(destroyListyString) --- enter\n");
  
	// Return early if we're passed a NULL ListyString pointer
  if (listy == NULL)
  {
    debugf("(destroyListyString) [NULL] Terminating early due to NULL arguments\n");
    debugf("(destroyListyString) --- exit\n");
    return NULL;
  }

 // Or, if we're passed a ListyString with a NULL head, we don't need to bother freeing any nodes.
 if (listy->head == NULL)
 {
   free(listy);
   debugf("(destroyListyString) [NULL] Triumphantly returning NULL after successful destruction of an EMPTY listystring\n");
   debugf("(destroyListyString) --- exit\n");
   return NULL;
 }

  tmp = listy->head;
    
  for (i = 0; i < listy->length - 1; i++)
  {
    next_node = tmp->next;
    
    free(tmp);
    
    tmp = next_node;
  }
  
  free(listy);
  
  debugf("(destroyListyString) [NULL] Triumphantly returning NULL after successful destruction of a listystring\n");
  debugf("(destroyListyString) --- exit\n");
  return NULL;
}

ListyString *cloneListyString(ListyString *listy)
{
  char *listy_contents;
  
  debugf("(cloneListyString) --- enter\n");
  if (listy == NULL)
  {
    debugf("(cloneListyString) [NULL] ERROR: Terminating early due to NULL listystring pointer!\n");
    debugf("(cloneListyString) --- exit\n");
    return NULL;
  }

  listy_contents = listy_to_string(listy);
  debugf("(cloneListyString) Converted ListyString of length %d to a string of length %d\n", (int)listy->length, (int)strlen(listy_contents));

  debugf("(cloneListyString) [new listy string] Triumphantly returning a new listy string.\n");
  debugf("(cloneListyString) --- exit\n");
  return createListyString(listy_contents);
}

void replaceChar(ListyString *listy, char key, char *str)
{
  int str_len, i, list_length;
  ListyString *tmp_string;
  ListyNode *tmp_head, *tmp_string_tail, *tmp_prev;
  
  debugf("(replaceChar) --- enter\n");
  if(listy == NULL || listy->head == NULL)
  {
    debugf("(replaceChar) [void] Called with NULL arguments\n");
    debugf("(replaceChar) --- exit\n");
    return;
  }
  
  str_len = strlen(str);
  tmp_head = listy->head;
  tmp_prev = NULL;
  list_length = listy->length;

  for (i = 0; i < list_length; i++)
  {
		
		if (tmp_head == NULL)
			continue;
			
		debugf("(replaceChar) INFO: String state is \"%s\"\n", listy_to_string(listy));
		
    if (tmp_head->data == key)
		{
      debugf("(replaceChar) Found key %c in string!\n", key);
      
			// All cases in here are for DELETING characters from our listystring
      if (str_len == 0 || str == NULL)
      {
        debugf("(replaceChar) Provided string is NULL, so we'll delete this node\n");
        debugf("(replaceChar) (tmp_prev == NULL): %d (tmp_head->next == NULL): %d \n", (tmp_prev == NULL), (tmp_head->next == NULL));
        
        if (tmp_prev != NULL && tmp_head->next != NULL)
        {
          debugf("(replaceChar) Connecting node %c@%p to %c@%p (DELETE, case A)\n", tmp_prev->data, tmp_prev, tmp_head->next->data, tmp_head->next);
          tmp_prev->next = tmp_head->next;
          free(tmp_head);
          tmp_head = tmp_prev;
        }
        
        if (tmp_prev == NULL && tmp_head->next != NULL)
        {
          debugf("(replaceChar) Connecting original HEAD node %c@%p to new HEAD %c@%p (DELETE, case B)\n", listy->head->data, listy->head, tmp_head->data, tmp_head);
					listy->head = listy->head->next;
					tmp_head = listy->head;
					tmp_prev = NULL;
					continue;
        }
        
        if (tmp_prev != NULL && tmp_head->next == NULL)
        {
          debugf("(replaceChar) Connecting node %c@%p to NULL to create a new end of our list (DELETE, case C)\n", tmp_prev->data, tmp_prev);
          tmp_prev->next = NULL;
          free(tmp_head);
					tmp_head = tmp_prev;
        }
        
        listy->length -= 1;
			
      }
      else
      {
				// Everything in here does a proper replacement
        debugf("(replaceChar) Will now replace '%c' with '%s'\n", key, str);
        tmp_string = createListyString(str);
        tmp_string_tail = get_listy_tail(tmp_string);
        debugf("(replaceChar) Now have a new ListyString of length %d, and its tail pointer: %p\n", tmp_string->length, tmp_string_tail);
        debugf("(replaceChar) (tmp_prev == NULL): %d (tmp_head->next == NULL): %d \n", (tmp_prev == NULL), (tmp_head->next == NULL));
        
				// If the node in question is between two other nodes
        if (tmp_prev != NULL && tmp_head->next != NULL)
        {
          debugf("(replaceChar) Connecting node %c@%p to %c@%p (case A)\n", tmp_prev->data, tmp_prev,  tmp_string->head->data, tmp_string->head);
          tmp_prev->next = tmp_string->head;
          tmp_string_tail->next = tmp_head->next;
          free(tmp_head);
          tmp_head = tmp_string_tail;
        }
        
				// If the node is the first node in the linked list
        if (tmp_prev == NULL && tmp_head->next != NULL)
        {
          debugf("(replaceChar) Connecting original HEAD node %c@%p to new HEAD %c@%p (case B)\n", listy->head->data, listy->head, tmp_string->head->data, tmp_string->head);
        	free(listy->head);
					tmp_string_tail->next = tmp_head->next;
          listy->head = tmp_string->head;
          tmp_head = tmp_string_tail;
        }
        
				// If the node is the last node in the linked list
        if (tmp_prev != NULL && tmp_head->next == NULL)
        {
          debugf("(replaceChar) Connecting node %c@%p to %c@%p (case C)\n", tmp_prev->data, tmp_prev, tmp_string_tail->data, tmp_string_tail);
          tmp_prev->next = tmp_string->head;
          free(tmp_head);
          tmp_head->next = tmp_string_tail;
					tmp_string_tail->next = NULL;
        }

				// If the node is the *only* node in the linked list
				if (tmp_prev == NULL && tmp_head->next == NULL)
        {
          debugf("(replaceChar) Replacing original HEAD node %c@%p with new HEAD %c@%p (case D)\n", listy->head->data, listy->head, tmp_string->head->data, tmp_string->head);
        	free(listy->head);
					listy->head = tmp_string->head;
					tmp_head = listy->head;
					tmp_string_tail->next = NULL;
        }
        
				// Whenever we append to the list, increment the length property accordingly
        listy->length += (str_len - 1);
      }
    }

      tmp_prev = tmp_head;
      tmp_head = tmp_head->next;
  }

  debugf("(replaceChar) --- exit\n");
}

void reverseListyString(ListyString *listy)
{
  ListyNode *next, *head, *prev;
  
  debugf("(reverseListyString) --- enter\n");
  if(listy == NULL || listy->head == NULL)
  {
    debugf("(reverseListyString) [void] Called with NULL arguments\n");
    debugf("(reverseListyString) --- exit\n");
    return;
  }

	head = listy->head;
	prev = NULL;
	while (head != NULL)
	{
     next = head->next;
     head->next = prev;
		 prev = head;
     head = next;
  }
	
	// The new root of our linked list will be the last node that was
	// processed, since head will now be NULL
	listy->head = prev;

  debugf("(reverseListyString) --- exit\n");
}

ListyString *listyCat(ListyString *listy, char *str)
{
  int str_len = 0;
  ListyNode *tail;
	ListyString *tmp_string;
  
  debugf("(listyCat) --- enter\n");
	debugf("(listyCat) (listy == NULL): %d (str == NULL): %d\n", (listy == NULL), (str == NULL));
	
	if(listy == NULL && str == NULL)
	{
		debugf("(listyCat) [NULL] Called with NULL arguments\n");
		debugf("(listyCat) --- exit\n");
		return NULL;
	}

	debugf("(listyCat) Str length: %d\n", str_len);
	
	if(listy == NULL && str_len > 0)
	{
		debugf("(listyCat) [createListyString()] Called with NULL ListyString and non-NULL string\n");
		debugf("(listyCat) --- exit\n");
		return createListyString(str);
	}
  
  if(listy == NULL && str_len == 0)
  {
    debugf("(listyCat) [ListyString] Called with NULL ListyString and non-NULL, empty string\n");
    debugf("(listyCat) --- exit\n");
    return createListyString(str);
  }
  
	if(str != NULL)
		str_len = strlen(str);
	
	// Only try to append to the linked list if we have some data in the string
	if (str_len > 0)
	{
		str_len = strlen(str);
  	tail = get_listy_tail(listy);
		tmp_string = createListyString(str);
		
		if (tail == NULL)
			listy->head = tmp_string->head;
		else
			tail->next = tmp_string->head;
			
			free(tmp_string);
			listy->length += str_len;
  }
	
  debugf("(listyCat) [ListyString] Triumphantly returning our concatenated ListyString\n");
  debugf("(listyCat) --- exit\n");
  return listy;
}

int listyCmp(ListyString *listy1, ListyString *listy2)
{
  int i;
  ListyNode *l1_tmp, *l2_tmp;
  
  debugf("(listyCmp) --- enter\n");
  
  // Two NULL pointers are considered equivalent
  if (listy1 == NULL && listy2 == NULL)
  {
    debugf("(listyCmp) [0] INFO: Encountered two NULL ListyStrings\n");
    debugf("(listyCmp) --- exit\n");
    return 0;
  }
  
  // Perform checks specific to non-NULL listystrings
  if (listy1 != NULL && listy2 != NULL)
  {
    // Two non-NULL listystrings with NULL head pointers are equivalent
    if (listy1->head == NULL && listy2->head == NULL)
    {
      debugf("(listyCmp) [0] INFO: Encountered two non-NULL ListyStrings with NULL heads\n");
      debugf("(listyCmp) --- exit\n");
      return 0;
    }
    
    // Two non-NULL listystrings with unequal lengths must be non-equivalent
    if(listy1->length != listy2->length)
    {
      debugf("(listyCmp) [-1] INFO: Encountered two non-NULL ListyStrings with unequal lengths\n");
      debugf("(listyCmp) --- exit\n");
      return -1;
    }
  }
  else
  {
    // A NULL listystring pointer is not equivalent to a non-NULL one
    debugf("(listyCmp) [-1] INFO: Returning -1 \n");
    debugf("(listyCmp) --- exit\n");
    return -1;
  }

  l1_tmp = listy1->head;
  l2_tmp = listy2->head;

  for (i = 0; i < listy1->length; i++)
  {
    if (l1_tmp->data != l2_tmp->data)
      return -1;
      
    l1_tmp = l1_tmp->next;
    l2_tmp = l2_tmp->next;
  }
  
  debugf("(listyCmp) [0] Triumphantly returning 0 for two equivalent ListyStrings\n");
  debugf("(listyCmp) --- exit\n");
  return 0;
}

int listyLength(ListyString *listy)
{
  debugf("(listyLength) --- enter\n");
  if (listy == NULL)
  {
    debugf("(listyLength) [-1] INFO: Y'all passed me an empty string\n");
    return -1;
  }
  
  debugf("(listyLength) [%d] Triumphantly returning ListyString length\n", listy->length);
  debugf("(listyLength) --- exit\n");
  return listy->length;
}

void printListyString(ListyString *listy)
{
  debugf("(printListyString) --- enter\n");
  if (listy == NULL || listy->head == NULL || listy->length == 0)
  {
    printf("(empty string)\n");
    debugf("(printListyString) INFO: Y'all passed me an empty string\n");
  }
  else
  {
    printf("%s\n", listy_to_string(listy));
  }
  
  debugf("(printListyString) --- exit\n");
}

char *listy_to_string(ListyString *list)
{
  ListyNode *tmp;
  char *str;
  int i;
  
  debugf("(listy_to_string) --- enter\n");
  if (list == NULL || list->head == NULL)
  {
    debugf("(listy_to_string) [NULL] ERROR: Terminating early due to NULL pointer!\n");
    debugf("(listy_to_string) --- exit\n");
    return NULL;
  }
  
  tmp = list->head;
  str = malloc(sizeof(char) * (list->length-1));
  
  // May need to implement proper bounds checking for the very last node,
  // which will not have a next pointer.
  for (i = 0; i < list->length; i++)
  {
    str[i] = tmp->data;
    tmp = tmp->next;
    
    if (tmp == NULL)
    {
			str[i+1] = '\0';
      debugf("(listy_to_string) Encountered a NULL node at index %d (loop stopping)\n", i);
      break;
    }
  }
  
  // Terminate our character array with a null sentinel
  str[list->length] = '\0';
  
  debugf("(listy_to_string) [new string] Triumphantly returning a new string\n");
  debugf("(listy_to_string) --- exit\n");
  return str;
}

ListyNode *get_listy_tail(ListyString *list)
{
  ListyNode *tmp;
  int i;
  
  debugf("(get_listy_tail) --- enter\n");
	
  if (list == NULL)
  {
    debugf("(get_listy_tail) [NULL] ERROR: Terminating early due to NULL pointer!\n");
    debugf("(get_listy_tail) --- exit\n");
    return NULL;
  }
  
	if (list->head == NULL)
	{
		debugf("(get_listy_tail) [list->head] Triumphantly returning HEAD node of ListyString\n");
		debugf("(get_listy_tail) --- exit\n");
		return list->head;
	}
	
  tmp = list->head;
  
  // Verify bounds checking here
  for (i = 1; i < list->length; i++)
  {
    tmp = tmp->next;
    
    if (tmp == NULL)
    {
      debugf("(get_listy_tail) [NULL] ERROR: Recieved malformed listy string- encountered a NULL node at index %d\n", i);
      debugf("(get_listy_tail) --- exit\n");
      return NULL;
    }
  }
  
  debugf("(get_listy_tail) [%p] Triumphantly returning the address of the tail ListyNode\n", tmp);
  debugf("(get_listy_tail) --- exit\n");
  return tmp;
}

ListyNode *find_listy_char(ListyString *list, char key)
{
  ListyNode *tmp;
  int i;
  
  debugf("(find_listy_char) --- enter\n");
  if (list == NULL || list->head == NULL)
  {
    debugf("(find_listy_char) [NULL] ERROR: Terminating early - malformed listy string or head is NULL\n");
    debugf("(find_listy_char) --- exit\n");
    return NULL;
  }
  
  tmp = list->head;
  
  // Verify bounds checking here
  for (i = 1; i < list->length - 1; i++)
  {
    if (tmp == NULL)
    {
      debugf("(find_listy_char) [NULL] Triumphantly returning NULL, no nodes found containing %c (hit NULL next pointer)\n", key);
      debugf("(find_listy_char) --- exit\n");
      return NULL;
    }
    
    if (tmp->data == key)
    {
      debugf("(find_listy_char) [%p] Success! Triumphantly returning address of node containing %c\n", tmp, key);
      debugf("(find_listy_char) --- exit\n");
      return tmp;
    }
    tmp = tmp->next;
  }
  
  debugf("(find_listy_char) [NULL] Triumphantly returning NULL, no nodes found containing %c (hit end of list length)\n", key);
  debugf("(find_listy_char) --- exit\n");
  return NULL;
}

void print_usage(char *name)
{
	printf("Usage: %s  [input file]\n", name);
}


double difficultyRating(void)
{
  return 4.0;
}


double hoursSpent(void)
{
  return 5.0;
}
