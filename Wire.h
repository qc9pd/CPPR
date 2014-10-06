#ifndef WIRE_HEADER
#define WIRE_HEADER

#include <iostream>
#include <string>
#include "Pin.h"


class Wire
{
 private:
  string WireName;
  double EarlyDelay;
  double LateDelay;
  Pin* source;
  Pin* sink;
  Wire* INextWire;
  Wire* ONextWire;
  Wire* Former;

 public:
  Wire(string n1, string n2, double e, double l);
  Wire(double e, double l, string n1, string n2);
  ~Wire();
  const double GetEarlyDelay();
  const double GetLateDelay();
  Pin* GetSource() const;
  Pin* GetSink() const;
  void SetSource(Pin* s);
  void SetSink(Pin* s);
  Wire* GetINextWire() const;
  Wire* GetONextWire() const;
  string GetWireName();
  void SetINextWire(Wire* ww);
  void SetONextWire(Wire* ww);
  void SetFormerWire(Wire* ww);
  Wire* GetFormerWire() const;
};

#endif
