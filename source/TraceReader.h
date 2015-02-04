#ifndef __TRACE_READER_H__
#define __TARCE_READER_H__
#include <CustomTypes.h>
#include <cache.h>
#include <cstdio>
#include <exception>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#define INVALID_CHAR 255U
class TraceReader
{
  enum InstructionType
  {
    e_Read  =0,
    e_Write =1,
    e_Null  =2
  };
  private:
  FILE *_fp;
  InstructionType _nextInst;
  ulong _nextAddr;
  char _nextOp;
  uint _nextProc;
  ushort _hexMap[256];
  ulong Ascii2Hex(uchar buff [])
  {
    ulong num=0U;
    int k=0;
    while(buff[k])
    {
      if(_hexMap[(int)buff[k]] != INVALID_CHAR)
      {
        num = (num<<4U) + _hexMap[(int)buff[k]];
      }
      k++;
    }
    return num;
  }
  void parse()
  {
    uchar buff[20] = {0};
    _nextInst = e_Null;
    _nextAddr = 0U;
    if(!feof(_fp))
    {
      _nextProc = INVALID_CHAR;
      _nextOp = 0;
      _nextAddr = 0;
      fscanf(_fp, "%d %c", &_nextProc,&_nextOp);
      if(_nextProc == INVALID_CHAR) return;
      fgets((char*)buff, 20, _fp);
      _nextAddr = Ascii2Hex(buff);
    }
  }
  public:
  TraceReader(char *filename)
    :_fp(NULL)
  {
    for(ushort i=1; i<256;i++)
      _hexMap[i] = INVALID_CHAR;
    _hexMap[0] = 0;
    uchar val=0;
    for (uchar i='0'; i<='9'; i++)
      _hexMap[i] = val++;
    val=0xa;
    for(uchar i='a'; i<='f'; i++)
      _hexMap[i] = val++;
    val=0xa;
    for(uchar i='A'; i<='F'; i++)
      _hexMap[i] = val++;

    _fp = fopen(filename, "rb");
    if(_fp == NULL) throw "Unable to open trace file";
    parse();
  }
  ~TraceReader()
  {
    if(_fp)
      fclose(_fp);
    _fp = NULL;
  }
  operator bool() const
  {
    if(feof(_fp)) return false;
    if(_nextProc == INVALID_CHAR) return false;
    if(_nextOp == 0) return false;
    if(_nextAddr == 0) return false;
    return true;
  }
  void operator ++(int)
  {
    parse();
  }
  int proc()
  {
    return _nextProc;
  }
  unsigned char op()
  {
    return (unsigned char)_nextOp;
  }
  ulong addr()
  {
    return _nextAddr;
  }
};
#endif //__TARCE_READER_H__
