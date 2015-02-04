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
  // possible bus events
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
  typedef bool(*CopyEventFunc)(SnooperEvent, SnoopArgument);
};

// class to propagate signals between caches
class SharedBus
{
  struct SnooperEntry { Snooper::SnoopEventFunc seFunc; Snooper::CopyEventFunc ceFunc; Snooper::SnoopArgument arg;};
  public:
  // function to register snooper functions with bus
    void regSnoopEvent(Snooper::SnoopEventFunc func1, Snooper::CopyEventFunc func2, Snooper::SnoopArgument arg)
    {
      //DO((int*)func);DE((int*)arg);
      SnooperEntry s; 
      s.seFunc = func1; s.ceFunc = func2; s.arg = arg;
      _snoopers.push_back(s);
    }
    // function to post event to bus
    void postEvent(Snooper::SnooperEvent e)
    {
      //DE((int*)e.sender);
      for(uint i=0; i<_snoopers.size(); i++)
      {
        _snoopers[i].seFunc(e, _snoopers[i].arg);
      }
    }
    // function to check if copy exists in other caches
    bool copyExist(Snooper::SnooperEvent e)
    {
      for(uint i=0; i<_snoopers.size(); i++)
      {
        if( _snoopers[i].ceFunc(e, _snoopers[i].arg) ) return true;
      }
      return false;
    }
  private:
    std::vector<SnooperEntry> _snoopers;
};

/****add new states, based on the protocol****/
enum CacheState{
	INVALID = 0,
	VALID,
	DIRTY
};
// Share states of a cache line
enum ShareState{
  e_I = 0, // Invalid state
  e_S = 1, // SHared state
  e_M = 2, // modified state
  e_E = 3, // exclusive state
  e_Sc = 4, // Shared Clean (Dragon)
  e_Sm = 5 // Shared modified (Dragon)
};
class cacheLine 
{
protected:
   ulong tag;
   CacheState Flags;   // 0:invalid, 1:valid, 2:dirty 
   ulong seq;
   ShareState shareState; // share states refer enum for details
 
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
  typedef void(Cache::*CacheEventFuncPtr)(cacheLine*,ulong,uchar);
  typedef void(Cache::*BusEventFuncPtr)(cacheLine*, Snooper::SnooperEvent);
protected:
   ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   ulong reads,readMisses,writes,writeMisses,writeBacks;
   //******///
   //add coherence counters here///
   ulong invalidations,interventions;
   ulong flushes, flushOpts;
   // will count busRdx issued when we already have data in MSI
   ulong busUpgrades;

   SharedBus &bus;

   CacheEventFuncPtr hit;
   CacheEventFuncPtr miss;
   BusEventFuncPtr update;
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
   //******///
   //add other functions to handle bus transactions///
   void setCoherenceProtocol(int);
   static void snooperCallback(Snooper::SnooperEvent e, Snooper::SnoopArgument arg);
   static bool copyExistCallback(Snooper::SnooperEvent e, Snooper::SnoopArgument arg);

private:
   void updateOnSnoop(Snooper::SnooperEvent e);
   // function to invalidate a line
   void invalidate(cacheLine *line) { line->invalidate(); invalidations++; }

   // functions for MSI protocol
   void updateMSI(cacheLine *line, Snooper::SnooperEvent e);
   void hitMSI(cacheLine *line, ulong addr, uchar op);
   void missMSI(cacheLine *line, ulong addr, uchar op);
   // functions for MESI protocol
   void updateMESI(cacheLine *line, Snooper::SnooperEvent e);
   void hitMESI(cacheLine *line, ulong addr, uchar op);
   void missMESI(cacheLine *line, ulong addr, uchar op);
   // functions for Dragon protocol
   void updateDragon(cacheLine *line, Snooper::SnooperEvent e);
   void hitDragon(cacheLine *line, ulong addr, uchar op);
   void missDragon(cacheLine *line, ulong addr, uchar op);

   //******///

};

#endif
