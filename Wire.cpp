#include "Wire.h"
#include <iostream>

Wire::Wire(string n1, string n2, double e, double l)
{
  string wirename = n1;
  wirename.append(" ");
  wirename.append(n2);
  WireName = wirename;
  EarlyDelay = e;
  LateDelay = l;
  source = NULL;
  INextWire = NULL;
  ONextWire = NULL;
  sink = NULL;
  Former = NULL;
}

Wire::Wire(double e, double l, string n1, string n2)
{
  string wirename = n1;
  wirename.append(" ");
  wirename.append(n2);
  WireName = wirename;
  EarlyDelay = e;
  LateDelay = l;
  source = NULL;
  sink = NULL;
  INextWire = NULL;
  ONextWire = NULL;
  Former = NULL;
}


Wire::~Wire()
{
  delete source;
  delete sink;
}

const double Wire::GetEarlyDelay()
{
  return EarlyDelay;
}

const double Wire::GetLateDelay()
{
  return LateDelay;
}

Pin* Wire::GetSource() const
{
  return source;
}

Pin* Wire::GetSink() const
{
  return sink;
}

void Wire::SetSource(Pin* s)
{
  source = s;
}

void Wire::SetSink(Pin* s)
{
  sink = s;
}

string Wire::GetWireName()
{
  return WireName;
}

Wire* Wire::GetINextWire() const
{
  return INextWire;
}

Wire* Wire::GetONextWire() const
{
  return ONextWire;
}

void Wire::SetINextWire(Wire* ww)
{
  INextWire = ww;
}

void Wire::SetONextWire(Wire* ww)
{
  ONextWire = ww;
}

void Wire::SetFormerWire(Wire* ww)
{
  Former = ww;
}

Wire* Wire::GetFormerWire() const
{
  return Former;
}
