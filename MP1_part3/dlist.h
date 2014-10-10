
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

#ifndef DLIST_H
#define DLIST_H

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<omp.h>

 
#define NUM_ELEMENT 500
#define SEED 0

/* IntListNode structure defines a node in the linked list */
typedef struct tagIntListNode{
  int data;
  struct tagIntListNode *next;
  struct tagIntListNode *prev;
  omp_lock_t lock;
  int deleted;
} IntListNode;
typedef IntListNode *pIntListNode;


/* IntList is a list of integer values, ascendingly sorted, with the 
   smallest integer value at the head of the list */
typedef struct {
  pIntListNode  head;
  omp_lock_t head_lock;
} IntList;
typedef IntList* pIntList;

typedef struct {
  pIntListNode nodes[NUM_ELEMENT];
  int curPtr;
  int numElmt;
} ArrNode;
typedef ArrNode *pArrNode;



pIntListNode IntListNode_Create(int x);

void IntList_Init(pIntList list);
void IntList_Insert(pIntList pList, int x, pArrNode an);
void IntList_Delete(pIntList pList, int x);
void IntList_Print(pIntList list);

void ArrNode_Init(pArrNode an, long num);
pIntListNode ArrNode_getNode(pArrNode an);

#endif
