
/*********************************************************************
 *
 * This file is a part of source code package created by Yan Solihin for 
 * training purposes. 
 *
 * Copyright (C) 2008, 2009, 2010 Solihin Publishing & Consulting LLC
 *
 * This source file is distributed "as is", and comes with no warranty. 
 * The author or and copyright owner do not accept any responsibility 
 * for the consequences of its use.
 *
 * This source code is distributed for restricted non-commercial use 
 * only. Only registered participants of a course held by Solihin 
 * Publishing & Consulting LLC are granted permission to use the
 * file during the duration of the course. 
 * 
 * Other uses of this file, including copying, distributing, modifications, 
 * and incorporation into a different software package, both for commercial 
 * or non-commercial use, are prohibited except under a written permission 
 * by the copyright owner, Solihin Publishing & Consulting LLC. 
 *
 * To contact hte copyright owner, send email to yan.solihin@gmail.com
 *
 *********************************************************************/

/***************************************************************
 *  a simple doubly linked list implementation
 *     
 ****************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<omp.h>

#include "dlist.h"

// #include<omp.h>



pIntListNode IntListNode_Create(int x)
{
  pIntListNode newNode = (pIntListNode) malloc(sizeof(IntListNode));
  newNode->data = x;
  newNode->next = NULL;
  newNode->prev = NULL;
  newNode->deleted = 0;
  omp_init_lock(&(newNode->lock));
  return newNode;
}



void IntList_Init(pIntList list) 
{
  list->head = NULL;
}



void IntList_Insert(pIntList pList, int x, pArrNode an) 
{
  pIntListNode prev, p , 
               newNode = ArrNode_getNode(an);
  // assert(newNode!=NULL);
  int flag = 0;
  /* keep looping untill insertion is successfull */
  while(flag == 0)
  {  
    if (pList->head == NULL) { /* list is empty, insert the first element */
      /* locking the list as we need to change the head */
      omp_set_lock(&pList->head_lock);
      /* asserting that head is indeed null */
      if(pList->head == NULL) {
        pList->head = newNode;
        /* raising flag that insertion was successfull */
        flag = 1;
      }
      omp_unset_lock(&pList->head_lock);
    }
    else { 
      /* list is not empty, find the right place to insert element */
      p = pList->head;
      prev = NULL;
      while (p != NULL && p->data < newNode->data) {
        prev = p;
        p = p->next;
      }

      if (p == NULL) { /* insert as the last element */
        omp_set_lock(&prev->lock);
        /* asserting that prev is valid and it is the last element */
        if(prev->deleted == 0 && prev->next == NULL) {
          prev->next = newNode;
          newNode->prev = prev;
          /* raising flag that insertion was successfull */
          flag = 1;
        }
        omp_unset_lock(&prev->lock);
      }
      else if (prev == NULL) { /* insert as the first element */
        omp_set_lock(&pList->head_lock);
        omp_set_lock(&p->lock);
        /* checking if p is valid and is also the head of the list */
        if(p->deleted == 0 && p == pList->head && p->prev == NULL) {
          pList->head = newNode;
          newNode->next = p;
          p->prev = newNode;
          /* raising flag that insertion was successfull */
          flag = 1;
        }
        omp_unset_lock(&p->lock);
        omp_unset_lock(&pList->head_lock);
      }
      else { /* insert right between prev and p */
        omp_set_lock(&prev->lock);
        omp_set_lock(&p->lock);
        /* checking if prev and p are valid and they are linked */
        if( prev->deleted == 0 && p->deleted == 0 
            && prev->next == p && p->prev == prev) {
          prev->next = newNode;
          newNode->prev = prev;
          newNode->next = p;
          p->prev = newNode;
          /* raising flag that insertion was successfull */
          flag = 1;
        }
        omp_unset_lock(&prev->lock);
        omp_unset_lock(&p->lock);
      }
    }
  }
}


/* delete the first element that has a data value equal to x */
void IntList_Delete(pIntList pList, int x) 
{
  pIntListNode prev, p;

  if (pList->head == NULL) { /* list is empty, do nothing */
  }
  else { /* list is not empty, find the desired element */
    int flag = 0;
    /* keep looping untill deletion is succesfull */
    while (flag == 0)
    {
      p = pList->head;
      while (p != NULL && p->data != x) 
        p = p->next;

      if (p == NULL) { /* element not found, do nothing */
        flag =1;
      }
      else {
        if (p->prev == NULL) { /* delete the head element */
          omp_set_lock(&pList->head_lock);
          omp_set_lock(&p->lock);
          /* check if p is valid and is head and does not have a prev link */
          if(p->prev == NULL && p->deleted == 0 && p == pList->head) {
            if (p->next != NULL) {/* check if p has a next */
              omp_set_lock(&p->next->lock);
              /* check if next is valid and linked to p */
              if(p->next->prev == p && p->next->deleted == 0) {
                pList->head = p->next;
                p->next->prev = NULL;
                p->deleted = 1;
                flag = 1;
              }
              omp_unset_lock(&p->next->lock);
            }
            else {/* if next is not present simply delete p */
              pList->head = p->next;
              p->deleted = 1;
              flag = 1;
            }
          }
          omp_unset_lock(&p->lock);
          omp_unset_lock(&pList->head_lock);
          //free(p);
        }
        else { /* delete non-head element */
          omp_set_lock(&p->prev->lock);
          omp_set_lock(&p->lock);
          /* check if p and p->prev are valid and that both are linked */
          if(p->deleted == 0 && p->prev->next == p 
              && p->prev->deleted == 0) {
            if (p->next != NULL) {/* p is lnot last element */
              omp_set_lock(&p->next->lock);
              /* check if p->next is valid and linked to p */
              if(p->next->prev == p && p->next->deleted == 0) {
                p->next->prev = p->prev;
                p->prev->next = p->next;
                p->deleted = 1;
                flag = 1;
              }
              omp_unset_lock(&p->next->lock);
            }
            else {/* p is last element simply delete it */
              p->prev->next = p->next;
              p->deleted = 1;
              flag = 1;
            }
          }
          omp_unset_lock(&p->lock);
          omp_unset_lock(&p->prev->lock);
          //free(p);
        }
      }
    }
  }
}


void IntList_Print(pIntList list) 
{
  pIntListNode p = list->head;
  int i=0;

#if 0
  if (p != NULL) {
    printf("\n --- Content of List --- \n");
    i = 0;
    printf("list's first element is %d: %d\n", i, p->data);
  }
#else
  while (p != NULL) {
    printf("list element %d: %d\n", i, p->data);
    p = p->next;
    i++;
  }
#endif
}




void ArrNode_Init(pArrNode an, long num) 
{
  int i;
  
  assert(num <= NUM_ELEMENT);

  for (i=0; i<num; i++) {
    int val = (int) 10000 * ((double)rand() / ((double)(RAND_MAX)+(double)(1)) ) ;    
    an->nodes[i] = (pIntListNode) IntListNode_Create(val);
  }
  an->curPtr = 0;
  an->numElmt = num;
}

void ArrNode_Print(pArrNode an, long num)
{
  int i;

  for (i=0; i<num; i++) 
    printf("ArrNode_Print: element %d points to %p\n", i, an->nodes[i]);
}


pIntListNode ArrNode_getNode(pArrNode an)
// return the value of the current node, advance curPtr
{
  assert(an->curPtr < NUM_ELEMENT);
  pIntListNode p = NULL;
  do {
    p = an->nodes[an->curPtr];
    an->curPtr++;
  }while(p->deleted == 1);
  return p; 
}


int main() 
{
  int i;
  pIntList pList = (pIntList) malloc(sizeof(IntList));
  pArrNode an1 = (pArrNode) malloc(sizeof(ArrNode));
  pArrNode an2 = (pArrNode) malloc(sizeof(ArrNode));

  IntList_Init(pList);

  srand(SEED);

  ArrNode_Init(an1, NUM_ELEMENT);
  ArrNode_Init(an2, NUM_ELEMENT);
  // ArrNode_Print(an);
  omp_init_lock(&pList->head_lock); 

#pragma omp parallel sections default(shared) private(i)
  {
#pragma omp section
    {
      for (i=0; i<NUM_ELEMENT; i++)
        IntList_Insert(pList, i, an1);
    }
#pragma omp section
    {
      for (i=0; i<NUM_ELEMENT; i++)
        IntList_Insert(pList, NUM_ELEMENT-i, an2);
    }
#pragma omp section
    {
      for (i=0; i<NUM_ELEMENT; i++)
        IntList_Delete(pList, 79*i % NUM_ELEMENT);
    }
#pragma omp section
    {
      for (i=0; i<NUM_ELEMENT; i++)
        IntList_Delete(pList, 7919 * i % NUM_ELEMENT);
    }
  }

  IntList_Print(pList);

}


