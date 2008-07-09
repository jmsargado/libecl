#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <util.h>
#include <stringlist.h>

/**
   This file implements a very thin wrapper around a list (vector) of
   strings, and the total number of strings. It is mostly to avoid
   sending both argc and argv.

   Most of the functionality is implemented through stateless
   functions in util.c
*/
   
typedef enum {ref         = 0,    /* Means that the stringlist only has reference to the item,
				     and it is the responsibility of the calling scope to free 
				     the memory of this string. */
	      list_owned  = 1 ,   /* The stringlist has taken ownership to a reference , i.e. it
				     is the stringlist's responsibility to free this memory.*/
	      copy        = 2}    /* The stringlist has taken a copy of the string, it is
				     obsviously the responsibility of the stringlist to
				     free this memory, whereas it is the responsibility of
				     the calling scope to handle the input string.*/ owner_type;



struct stringlist_struct {
  int           size;       /* The number of elements */
  char        **strings;    /* The actual strings - NB we allow NULL strings */
  owner_type * owner;
};





/**
   this function appends num_append new items to the
   stringlist. observe that this functions does not assign values
   (apart from trivial null initialization) to the newly appended
   memory, i.e. it is esential that the calling routine (from this file) has

   stringlist->xx = yy;

   statements.
*/

   
void static stringlist_grow__(stringlist_type * stringlist , int num_append) {
  int old_size = stringlist->size;
  stringlist->size += num_append;
  stringlist->strings = util_realloc(stringlist->strings , stringlist->size * sizeof * stringlist->strings , __func__);
  stringlist->owner   = util_realloc(stringlist->owner   , stringlist->size * sizeof * stringlist->owner   , __func__);
  {
    int i;
    for (i = old_size; i < stringlist->size; i++) {
      stringlist->strings[i] = NULL;
      stringlist->owner[i]   = ref;
    }
  }
}


/**
   Sets element nr i in the stringlist to the input string 's'. If the
   list does not have that many elements we die (could grow as well??).

*/

static void stringlist_iset__(stringlist_type * stringlist , int index , const char * s , owner_type owner) {
  if (index >= stringlist->size) 
    util_abort("%s: sorry length(stringlist) = %d - index:%d invalid. \n",__func__ , stringlist->size , index); 

  if (stringlist->owner[index] != ref)
    util_safe_free(stringlist->strings[index]);
  
  stringlist->owner[index] = owner;
  if (owner == copy)
    stringlist->strings[index] = util_alloc_string_copy(s);
  else
    stringlist->strings[index] = (char *) s;
}




void stringlist_iset_copy(stringlist_type * stringlist , int index , const char *s) {
  stringlist_iset__(stringlist , index , s , copy);
}

void stringlist_iset_ref(stringlist_type * stringlist , int index , const char *s) {
  stringlist_iset__(stringlist , index , s , ref);
}

void stringlist_iset_owned_ref(stringlist_type * stringlist , int index , const char *s) {
  stringlist_iset__(stringlist , index , s , list_owned);
}



/**
   This function appends a copy of s into the stringlist.
*/
void stringlist_append_copy(stringlist_type * stringlist , const char * s) {
  stringlist_grow__(stringlist , 1);
  stringlist_iset_copy(stringlist , stringlist->size - 1 , s);
}

void stringlist_append_ref(stringlist_type * stringlist , const char * s) {
  stringlist_grow__(stringlist , 1);
  stringlist_iset_ref(stringlist , stringlist->size - 1 , s);
}

void stringlist_append_owned_ref(stringlist_type * stringlist , const char * s) {
  stringlist_grow__(stringlist , 1);
  stringlist_iset_owned_ref(stringlist , stringlist->size - 1 , s);
}



static stringlist_type * stringlist_alloc_empty() {
  stringlist_type * stringlist = util_malloc(sizeof * stringlist , __func__);
  stringlist->strings = NULL;
  stringlist->size    = 0;
  stringlist->owner   = NULL;
  return stringlist;
}


/*
static stringlist_type * stringlist_alloc__(int size , const char ** strings) {


}
*/



stringlist_type * stringlist_alloc_new() {
  return stringlist_alloc_empty();
}





void stringlist_free(stringlist_type * stringlist) {
  int i;
  if (stringlist->size > 0) {

    for (i = 0; i < stringlist->size; i++)
      if (stringlist->owner[i] != ref)
	util_safe_free(stringlist->strings[i]);
    free(stringlist->strings);
    free(stringlist->owner);

  }
  free(stringlist);
}






