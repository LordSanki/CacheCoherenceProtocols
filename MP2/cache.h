/*******************************************************
                          cache.h
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>
#include <vector>
#include <CustomTypes.h>
#include <stdlib.h>
typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;

namespace Snooper
{
  enum BusEvent
  {
    e_BusRd,
    e_BusRdX,
    e_Flush,
    e_FlushOpt,
    e_BusUpgr,
    e_BusUpdt
  };
  typedef void* SnoopArgument;
  struct SnooperEvent {
    SnoopArgument sender; ulong addr; BusEvent busEvent;
    SnooperEvent(ulong addr_, BusEvent e_, SnoopArgument sender_)
    {sender = sender_; addr = addr_; busEvent = e_;}
  };
  typedef void(*SnoopEventFunc)(SnooperEvent, SnoopArgument);
  struct SnooperEntry { SnoopEventFunc func; SnoopArgument arg;};
};

class SharedBus
{
  public:
    void regSnoopEvent(Snooper::SnoopEventFunc func, Snooper::SnoopArgument arg)
    {
      //DO((int*)func);DE((int*)arg);
      Snooper::SnooperEntry s; s.func = func; s.arg = arg;
      _snoopers.push_back(s);
    }
    void postEvent(Snooper::SnooperEvent e)
    {
      //DE((int*)e.sender);
      for(uint i=0; i<_snoopers.size(); i++)
      {
        _snoopers[i].func(e, _snoopers[i].arg);
      }
    }
    
  private:
    std::vector<Snooper::SnooperEntry> _snoopers;
};

/****add new states, based on the protocol****/
enum CacheState{
	INVALID = 0,
	VALID,
	DIRTY
};
enum ShareState{
  e_I = 0,
  e_S = 1,
  e_M = 2,
  e_E = 3,
  e_Sc = 4,
  e_Sm = 5
};
class cacheLine 
{
protected:
   ulong tag;
   CacheState Flags;   // 0:invalid, 1:valid, 2:dirty 
   ulong seq;
   ShareState shareState;
 
public:
   cacheLine()            { tag = 0; Flags = INVALID; shareState = e_I;}
   ulong getTag()         { return tag; }
   CacheState getFlags()			  { return Flags;}
   ulong getSeq()         { return seq; }
   ShareState getShareState()  { return shareState; }
   void setSeq(ulong Seq)			{ seq = Seq;}
   void setFlags(CacheState flags)			{  Flags = flags;}
   void setTag(ulong a)   { tag = a; }
   void invalidate()      { tag = 0; Flags = INVALID; shareState = e_I;}//useful function
   bool isValid()         { return ((Flags != INVALID) && (shareState !=e_I)); }
   void setShareState(ShareState s) {shareState = s; }
};

class Cache
{
protected:
   ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   ulong reads,readMisses,writes,writeMisses,writeBacks;
   ulong invalidations,interventions;
   ulong flushes;
   ulong numBusRdX;
   SharedBus &bus;
   //******///
   //add coherence counters here///
   //******///

   cacheLine **cache;
   ulong calcTag(ulong addr)     { return (addr >> (log2Blk) );}
   ulong calcIndex(ulong addr)  { return ((addr >> log2Blk) & tagMask);}
   ulong calcAddr4Tag(ulong tag)   { return (tag << (log2Blk));}
   
public:
    ulong currentCycle;  
     
    Cache(int,int,int, SharedBus &shared_bus);
   ~Cache() { delete cache;}
   
   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine * findLine(ulong addr);
   cacheLine * getLRU(ulong);
   
   ulong getRM(){return readMisses;} ulong getWM(){return writeMisses;} 
   ulong getReads(){return reads;}ulong getWrites(){return writes;}
   ulong getWB(){return writeBacks;}
   
   void writeBack(ulong)   {writeBacks++;}
   void Access(ulong,uchar);
   void printStats();
   void updateLRU(cacheLine *);
   void updateOnSnoop(Snooper::SnooperEvent e);
   static void snooperCallback(Snooper::SnooperEvent e, Snooper::SnoopArgument arg);
   //******///
   //add other functions to handle bus transactions///
   //******///

private:
   void updateMSI(cacheLine *line, Snooper::SnooperEvent e);
   void hitMSI(cacheLine *line, ulong addr, uchar op);
   void missMSI(cacheLine *line, ulong addr, uchar op);

};

#endif
