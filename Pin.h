#ifndef PIN_HEADER
#define PIN_HEADER

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Wire;

class Pin{
 private:
  string PinName;
  double ts;
  double th;
  double earlyAT;
  double lateAT;
  int PinType;  //PI(Data) is 1, PO is 2, ordinary is 0, D pin is 3, clk pin is 4,clksource is 5
  Wire* previous;
  Wire* next;
  pthread_mutex_t StreamMutex;
  
 public:
  Pin(string name, double s);
  Pin(string name, double s, int type);
  Pin(string name, int primary, double e, double l);
  Pin(string name, int primary);
  Pin(string name, Wire* w);
  Pin(Wire* w,string name);
  ~Pin();
  void ChangeTS(double t);
  void ChangeTH(double t);
  void SetEarlyAT(double t);
  void SetLateAT(double t);
  const double GetTS();
  const double GetTH();
  const double GetEarlyAT();
  const double GetLateAT();
  void SetNextWire(Wire* w);
  void SetPreviousWire(Wire* w);
  Wire* GetNextWire() const;
  Wire* GetPreviousWire() const;
  const string GetPinName();
  void SetPinType(int b);
  const int GetPinType();
  void LockMutex();
  void UnlockMutex();
};

#endif
