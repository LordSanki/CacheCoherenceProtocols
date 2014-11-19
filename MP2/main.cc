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
	
//	ifstream fin;
//	FILE * pFile;
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

	
	//****************************************************//
	//**printf("===== Simulator configuration =====\n");**//
	//*******print out simulator configuration here*******//
	//****************************************************//

 
	//*********************************************//
  //*****create an array of caches here**********//
  SharedBus bus;
  Cache **cacheArray = new Cache* [num_processors];
  for(int i=0;i<num_processors;i++)
  {
    cacheArray[i] = new Cache(cache_size, cache_assoc, blk_size, bus);
    cacheArray[i]->setCoherenceProtocol(protocol);
  }
	//*********************************************//	

	//pFile = fopen (fname,"r");
	//if(pFile == 0)
	//{   
	//	printf("Trace file problem\n");
	//	exit(0);
	//}
	///******************************************************************//
	//**read trace file,line by line,each(processor#,operation,address)**//
  for(TraceReader tr(argv[6]); tr; tr++)
  {
    cacheArray[tr.proc()]->Access(tr.addr(), tr.op());
  }
	//*****propagate each request down through memory hierarchy**********//
	//*****by calling cacheArray[processor#]->Access(...)***************//
	///******************************************************************//
	//fclose(pFile);

	//********************************//
	//print out all caches' statistics //
	//********************************//
#if 1
  printf("===== 506 Personal information =====\n");
  printf("FirstName (MiddleNames) LastName (Change it to your own name)\n");
  printf("UnityID (Change it to your own UID)\n");
  printf("Section ECE/CSC00x (Change it according to your own section)\n");
  printf("===== 506 SMP Simulator configuration =====\n");
  printf("L1_SIZE:    %d\n",cache_size);
  printf("L1_ASSOC:   %d\n",cache_assoc);
  printf("L1_BLOCKSIZE:   %d\n",blk_size);
  printf("NUMBER OF PROCESSORS: %d\n",num_processors);
  printf("COHERENCE PROTOCOL: %s\n", protocol_name[protocol]);
  printf("TRACE FILE:   %s\n",argv[6]);
  for(int i=0; i<num_processors; i++)
  {
    printf("============ Simulation results (Cache %d) ============\n",i);
    cacheArray[i]->printStats();
  }
#endif
}

