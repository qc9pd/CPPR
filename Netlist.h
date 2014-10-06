#ifndef NETLIST_HEADER
#define NETLIST_HEADER

#include "Pin.h"
#include "Wire.h"
#include "Path.h"
#include <cstring>
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;

class Netlist
{
 private:
  string DelayFileName;
  string TimingFileName;
  string OutputFileName;
  double T;
  Pin* CLKSource;
  int NUM_TEST;
  int NUM_PATH;
  string Mode;
  unordered_map<string, Pin*> PinMap;
  unordered_map<Pin*,Pin*> DCKMap;
  unordered_map<string, Wire*> WireMap;
  vector<Pin*> DPins;
  vector<Pin*> DPins_Copy;
  vector<Pin*> CKPins;  
  vector<Pin*> PrimaryInputs;
  vector<Pin*> PInputs;
  //vector<Path*> PathV;
  //unordered_map<string,int> CLKPath;  // if the value is 1, then it is a element in the early clk, if it is 2, then it is a element in the late clk, 3 if in both;
  vector<string> LatchV;
  pthread_mutex_t VectorMutex;
  pthread_mutex_t SlackMutex;
  pthread_mutex_t PathVMutex;
  pthread_mutex_t TestNumMutex;
  
 public:
  Netlist(string df, string tf, string output, string mode, int num_test, int num_path);
  ~Netlist();
  unordered_map<string,double> SetTestNum;
  unordered_map<string,double> HoldTestNum;
  void ReadDelayFile();
  void ReadTimingFile();
  unsigned GetDPinsSize();
  void TestHashTable(string n,string m);  
  void SetCLKSource(Pin* n);
  void FindCLKPath(Pin* Start,unordered_map<string,string> &Former);
  void* CLKPathEnumeration(void* t);
  Pin* GetCLKSource() const;
  void FindDataPath(Pin* Start, unordered_map<string,string> &Former);
  void* DataPathEnumeration(void* t);
  void SetAT(Pin* Start);
  void* SetATForAllPins(void* t);
  void* CalculateSlackSetup(void* t);
  void* CalculateSlackHold(void* t);
  void GenerateSetupReport();
  void GenerateHoldReport();
  void CopyDPins();
  void CleaningProcess();
  void VectorDuplicate();
  void RenameBoth();
  void RenameSetup();
  void RenameHold();
};

typedef void* (Netlist::*MEMBER_FUN)(void*);
typedef void* (*NORMAL_FUN)(void*);


#endif
