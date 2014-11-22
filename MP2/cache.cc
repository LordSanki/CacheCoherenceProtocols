/*******************************************************
                          cache.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cache.h"
#include <CustomTypes.h>
using namespace std;
using namespace Snooper;
#define CALL_FUNC(FUNC_PTR) (this->*(FUNC_PTR))
Cache::Cache(int s,int a,int b, SharedBus &shared_bus )
  :bus(shared_bus)
{
  ulong i, j;
  reads = readMisses = writes = 0; 
  writeMisses = writeBacks = currentCycle = 0;
  flushOpts = flushes = interventions = invalidations = 0;
  busUpgrades = 0;
  size       = (ulong)(s);
  lineSize   = (ulong)(b);
  assoc      = (ulong)(a);   
  sets       = (ulong)((s/b)/a);
  numLines   = (ulong)(s/b);
  log2Sets   = (ulong)(log2(sets));   
  log2Blk    = (ulong)(log2(b));   

  //*******************//
  //initialize your counters here//
  //*******************//

  tagMask =0;
  for(i=0;i<log2Sets;i++)
  {
    tagMask <<= 1;
    tagMask |= 1;
  }

  /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
  cache = new cacheLine*[sets];
  for(i=0; i<sets; i++)
  {
    cache[i] = new cacheLine[assoc];
    for(j=0; j<assoc; j++) 
    {
      cache[i][j].invalidate();
    }
  }      
  bus.regSnoopEvent(snooperCallback,copyExistCallback,this);
  setCoherenceProtocol(0);
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
void Cache::Access(ulong addr,uchar op)
{

	currentCycle++;/*per cache global counter to maintain LRU order 
			among cache ways, updated on every cache access*/
	if(op == 'w') writes++;
	else          reads++;
	
	cacheLine * line = findLine(addr);
	if(line == NULL)/*miss*/
	{
		cacheLine *newline = fillLine(addr);
    if(op == 'w')
    {
      writeMisses++;
      newline->setFlags(DIRTY);
    }
    else
      readMisses++;
    CALL_FUNC(miss)(newline, addr, op);
	}
	else
	{
		/**since it's a hit, update LRU and update dirty flag**/
		updateLRU(line);
    if(op == 'w')
    {
      line->setFlags(DIRTY);
    }
    CALL_FUNC(hit)(line, addr, op);
	}
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
  ulong i, j, tag, pos;

  pos = assoc;
  tag = calcTag(addr);
  i   = calcIndex(addr);

  for(j=0; j<assoc; j++)
    if(cache[i][j].isValid())
      if(cache[i][j].getTag() == tag)
      {
        pos = j; break; 
      }
  if(pos == assoc)
    return NULL;
  else
    return &(cache[i][pos]); 
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
  ulong i, j, victim, min;

  victim = assoc;
  min    = currentCycle;
  i      = calcIndex(addr);

  for(j=0;j<assoc;j++)
  {
    if(false == cache[i][j].isValid()) return &(cache[i][j]);
  }   
  for(j=0;j<assoc;j++)
  {
    if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
  }
  assert(victim != assoc);

  return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
  cacheLine * victim = getLRU(addr);
  updateLRU(victim);

  return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
  ulong tag;

  cacheLine *victim = findLineToReplace(addr);
  assert(victim != 0);
  if(victim->getFlags() == DIRTY) writeBack(addr);

  tag = calcTag(addr);
  victim->setTag(tag);
  victim->setFlags(VALID);
  /**note that this cache line has been already 
    upgraded to MRU in the previous function (findLineToReplace)**/

  return victim;
}

void Cache::printStats()
{
  double miss_rate = (double)(readMisses+writeMisses);
  miss_rate = miss_rate/((double)(reads+writes));
  miss_rate = miss_rate*100.0;
  printf("01. number of reads:        %lu\n",reads);
  printf("02. number of read misses:      %lu\n",readMisses);
  printf("03. number of writes:         %lu\n",writes);
  printf("04. number of write misses:     %lu\n",writeMisses);
  printf("05. total miss rate:        %1.2f%c\n",miss_rate,'%');
  printf("06. number of writebacks:       %lu\n",writeBacks);
  printf("07. number of cache-to-cache transfers:   %lu\n",flushOpts);
  printf("08. number of memory transactions:    %lu\n",readMisses+writeMisses+writeBacks+busUpgrades-flushOpts);
  printf("09. number of interventions:      %lu\n",interventions);
  printf("10. number of invalidations:      %lu\n",invalidations);
  printf("11. number of flushes:        %lu\n",flushes);

  /****print out the rest of statistics here.****/
  /****follow the ouput file format**************/
}

void Cache::setCoherenceProtocol(int choice)
{
  switch(choice)
  {
    case 0:
      hit = &Cache::hitMSI;
      miss = &Cache::missMSI;
      update = &Cache::updateMSI;
      break;
    case 1:
      hit = &Cache::hitMESI;
      miss = &Cache::missMESI;
      update = &Cache::updateMESI;
      break;
    case 2:
      hit = &Cache::hitDragon;
      miss = &Cache::missDragon;
      update = &Cache::updateDragon;
      break;
    default:
      hit = &Cache::hitMSI;
      miss = &Cache::missMSI;
      update = &Cache::updateMSI;
      break;
  }
}

void Cache::snooperCallback(SnooperEvent e, SnoopArgument arg)
{
  ((Cache*)arg)->updateOnSnoop(e);
}

bool Cache::copyExistCallback(SnooperEvent e, SnoopArgument arg)
{
  if( ((Cache*)arg) == ((Cache*)e.sender) ) return false;
  if( NULL == ((Cache*)arg)->findLine(e.addr) ) return false;
  return true;
}

void Cache::updateOnSnoop(SnooperEvent e)
{
  if(((Cache*)e.sender) == this) return;
  cacheLine *line = findLine(e.addr);
  if(line == NULL) return;
  CALL_FUNC(update)(line, e);
}

void Cache::updateMSI(cacheLine *line, SnooperEvent e)
{
  if(line->getShareState() == e_S)
    // Shared state
  {
    if( e_BusRdX == e.busEvent )
    {
      invalidate(line);
    }
  }
  if(line->getShareState() == e_M)
    // Modified state
  {
    //DO(currentCycle); DO(e.addr); DO(e.busEvent); DE(this);
    if( e_BusRdX == e.busEvent )
      // transition to invalid
    {
      writeBack(e.addr);
      invalidate(line);
      bus.postEvent(SnooperEvent(e.addr, e_Flush, this));
      flushes++;
    }
    if( e_BusRd == e.busEvent )
      // transition to shared
    {
      writeBack(e.addr);
      line->setFlags(VALID);
      line->setShareState(e_S);
      bus.postEvent(SnooperEvent(e.addr, e_Flush,this));
      flushes++;
      interventions++;
    }
  }
}

void Cache::missMSI(cacheLine *line, ulong addr, uchar op)
{
  if(op == 'w')
  {
    bus.postEvent(SnooperEvent(addr, e_BusRdX,this));
    line->setShareState(e_M);
  }
  else
  {
    bus.postEvent(SnooperEvent(addr, e_BusRd,this));
    line->setShareState(e_S);
  }
}

void Cache::hitMSI(cacheLine *line, ulong addr, uchar op)
{
  if(op == 'w')
  {
    if(line->getShareState() == e_S)
    {
      bus.postEvent(SnooperEvent(addr,e_BusRdX,this));
      busUpgrades++;
      line->setShareState(e_M);
    }
  }
  else
  {
  }
}

void Cache::updateMESI(cacheLine *line, SnooperEvent e)
{
  if(line->getShareState() == e_S)
  // Shared state
  {
    if( e_BusRdX == e.busEvent || e_BusUpgr == e.busEvent)
    {
      invalidate(line);
    }
    if( e_BusRdX == e.busEvent || e_BusRd == e.busEvent)
    {
      //flushOpts++;
    }
  }
  if(line->getShareState() == e_E)
  {
    if( e_BusRdX == e.busEvent )
    {
      //flushOpts++;
      invalidate(line);
    }
    if( e_BusRd == e.busEvent )
    {
      //flushOpts++;
      interventions++;
      line->setShareState(e_S);
    }
  }
  updateMSI(line, e);
}

void Cache::hitMESI(cacheLine *line, ulong addr, uchar op)
{
  if(op == 'w')
  {
    if(line->getShareState() == e_S)
    {
      bus.postEvent(SnooperEvent(addr,e_BusUpgr,this));
      //busUpgrades++;
      line->setShareState(e_M);
    }

    if(line->getShareState() == e_E)
    {
      line->setShareState(e_M);
    }
  }
}

void Cache::missMESI(cacheLine *line, ulong addr, uchar op)
{
  bool copy_exists = bus.copyExist(SnooperEvent(addr,e_BusRd,this));
  missMSI(line, addr, op);
  if(copy_exists)
      flushOpts++;
  else if(op == 'r')
      line->setShareState(e_E);
}

void Cache::updateDragon(cacheLine *line, Snooper::SnooperEvent e)
{
  if(line->getShareState() == e_E)
  // Shared state
  {
    if(e_BusRd == e.busEvent)
    {
      interventions++;
      line->setShareState(e_Sc);
      //flushOpts++;
    }
  }
  if(line->getShareState() == e_Sm)
  {
    if( e_BusUpdt == e.busEvent )
    {
      line->setShareState(e_Sc);
      line->setFlags(VALID);
    }
    if( e_BusRd == e.busEvent )
    {
      bus.postEvent(SnooperEvent(e.addr, e_Flush,this));
      writeBacks++;
      flushes++;
    }
  }
  if(line->getShareState() == e_M)
  {
    if( e_BusRd == e.busEvent )
    {
      bus.postEvent(SnooperEvent(e.addr, e_Flush,this));
      line->setShareState(e_Sm);
      interventions++;
      writeBacks++;
      flushes++;
    }
  }
}

void Cache::hitDragon(cacheLine *line, ulong addr, uchar op)
{
  if(op == 'w')
  {
    if(line->getShareState() == e_Sc)
    {
      bus.postEvent(SnooperEvent(addr, e_BusUpdt,this));
      if( bus.copyExist(SnooperEvent(addr,e_BusRd,this)) )
      {
        line->setShareState(e_Sm);
      }
      else
      {
        line->setShareState(e_M);
      }
    }
    if(line->getShareState() == e_E)
    {
      line->setShareState(e_M);
    }
    if(line->getShareState() == e_Sm)
    {
      bus.postEvent(SnooperEvent(addr, e_BusUpdt,this));
      if(! bus.copyExist(SnooperEvent(addr,e_BusRd,this)) )
        line->setShareState(e_M);
    }
  }
}

void Cache::missDragon(cacheLine *line, ulong addr, uchar op)
{
  bool copy_exists = bus.copyExist(SnooperEvent(addr,e_BusRd,this));
  bus.postEvent(SnooperEvent(addr, e_BusRd,this));

  if(op == 'w')
  {
    if(copy_exists)
    {
      line->setShareState(e_Sm);
      bus.postEvent(SnooperEvent(addr, e_BusUpdt,this));
      //flushOpts++;
    }
    else
    {
      line->setShareState(e_M);
    }
  }
  else
  {
    if(copy_exists)
    {
      line->setShareState(e_Sc);
      //flushOpts++;
    }
    else
    {
      line->setShareState(e_E);
    }
  }
}


