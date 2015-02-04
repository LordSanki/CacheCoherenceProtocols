/*******************************************************
                          main.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <TraceReader.h>
using namespace std;

#include "cache.h"

int main(int argc, char *argv[])
{
	
  const char * protocol_name[3] = {"MSI","MESI","Dragon"};
	if(argv[1] == NULL){
		 printf("input format: ");
		 printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
		 exit(0);
        }

	/*****uncomment the next five lines*****/
	int cache_size = atoi(argv[1]);
	int cache_assoc= atoi(argv[2]);
	int blk_size   = atoi(argv[3]);
	int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
	int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:Dragon*/

 
	//*********************************************//
  //*****create an array of caches here**********//
  // creating shared bus
  SharedBus bus;
  //creating array of caches
  Cache **cacheArray = new Cache* [num_processors];
  // now creating caches
  for(int i=0;i<num_processors;i++)
  {
    cacheArray[i] = new Cache(cache_size, cache_assoc, blk_size, bus);
    cacheArray[i]->setCoherenceProtocol(protocol);
  }

	///******************************************************************//
	//**read trace file,line by line,each(processor#,operation,address)**//
  for(TraceReader tr(argv[6]); tr; tr++)
  {
    //*****propagate each request down through memory hierarchy**********//
    //*****by calling cacheArray[processor#]->Access(...)***************//
    ///******************************************************************//
    cacheArray[tr.proc()]->Access(tr.addr(), tr.op());
  }

  ////****************************************************//
	//*******print out simulator configuration here*******//
	//****************************************************//
  printf("===== 506 Personal information =====\n");
  printf("Prashant Solanki\n");
  printf("UnityID psolank\n");
  printf("Section ECE 506-001\n");
  printf("===== 506 SMP Simulator configuration =====\n");
  printf("L1_SIZE:    %d\n",cache_size);
  printf("L1_ASSOC:   %d\n",cache_assoc);
  printf("L1_BLOCKSIZE:   %d\n",blk_size);
  printf("NUMBER OF PROCESSORS: %d\n",num_processors);
  printf("COHERENCE PROTOCOL: %s\n", protocol_name[protocol]);
  printf("TRACE FILE:   %s\n",argv[6]);
  //********************************//
	//print out all caches' statistics //
	//********************************//
  for(int i=0; i<num_processors; i++)
  {
    printf("============ Simulation results (Cache %d) ============\n",i);
    cacheArray[i]->printStats();
  }
}

