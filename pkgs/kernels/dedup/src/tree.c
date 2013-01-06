/* Based on Data Structures and Algorithm Analysis in C (Second Edition)
 * by Mark Allen Weiss.
 *
 * Modified by Minlan Yu.
 */
#include <stdlib.h>
#include <stdio.h>

#include "tree.h"
#include "util.h"

#ifdef ENABLE_DMALLOC
#include <dmalloc.h>
#endif //ENABLE_DMALLOC

struct TreeNode * pmin;

SearchTree TreeMakeEmpty( SearchTree T ) {
  if( T != NULL ) {
    TreeMakeEmpty( T->Left );
    TreeMakeEmpty( T->Right );
    free( T );
    pmin = NULL;
  }
  return NULL;
}

Position TreeFind( int value, SearchTree T ) {
  if( T == NULL ) return NULL;
  if( value < T->Element.l1num ) {
    return TreeFind( value, T->Left );
  } else {
    if( value > T->Element.l1num ) {
      return TreeFind(value, T->Right );
    } else {
      return T;
    }
  }
}

Position TreeFindMin( SearchTree T ) {
  if (pmin != NULL) return pmin;
  if( T == NULL ) {
    return NULL;
  } else {
    if( T->Left == NULL ) {
      return T;
    } else {
      return TreeFindMin( T->Left );
    }
  }
}

Position TreeFindMax( SearchTree T ) {
  if( T != NULL ) {
    while( T->Right != NULL ) {
      T = T->Right;
    }
  }
  return T;
}

SearchTree TreeInsert( TreeElementType X, SearchTree T ) {
  if (pmin!= NULL && X.l1num < pmin->Element.l1num) pmin = NULL;
  if( T == NULL ) {
    /* Create and return a one-node tree */
    T = malloc( sizeof( struct TreeNode ) );
    if( T == NULL ) {
      perror( "Out of space!!!" );
    } else {
      T->Element = X;
      T->Left = T->Right = NULL;
    }
  } else {
    if( X.l1num < T->Element.l1num ) {
      T->Left = TreeInsert( X, T->Left );
    } else {
      if( X.l1num > T->Element.l1num ) {
        T->Right = TreeInsert( X, T->Right );
      }
      /* Else X is in the tree already; we'll do nothing */
    }
  }
  return T;  /* Do not forget this line!! */
}

SearchTree TreeDelete( TreeElementType X, SearchTree T ) {
  Position TmpCell;
            
  if (pmin != NULL && X.l1num == pmin->Element.l1num) pmin = NULL;

  if( T == NULL ) {
    perror( "Element not found" );
  } else {
    if( X.l1num < T->Element.l1num ) {
      /* Go left */
      T->Left = TreeDelete( X, T->Left );
    } else {
      if( X.l1num > T->Element.l1num ) {
        /* Go right */
        T->Right = TreeDelete( X, T->Right );
      } else {
        /* Found element to be deleted */
        if( T->Left && T->Right ) {
          /* Two children */
          /* Replace with smallest in right subtree */
          TmpCell = TreeFindMin( T->Right );
          T->Element = TmpCell->Element;
          T->Right = TreeDelete( T->Element, T->Right );
        } else {
          /* One or zero children */
          TmpCell = T;
          if( T->Left == NULL ) {
            /* Also handles 0 children */
            T = T->Right;
          } else {
            if( T->Right == NULL ) {
              T = T->Left;
            }
          }
          free( TmpCell );
        }
      }
    }
  }

  return T;
}

TreeElementType Retrieve( Position P ) {
  return P->Element;
}

