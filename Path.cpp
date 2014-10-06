#include "Pin.h"
#include "Wire.h"

Pin::Pin(string name, double s)
{
  PinName = name;
  ts = s;
  th = 0;
  earlyAT = 0;
  lateAT = 0;
  PinType = 0;
  previous = NULL;
  next = NULL;
  pthread_mutex_init(&StreamMutex,NULL);
}

Pin::Pin(string name, double s, int type)
{
  PinName = name;
  ts = s;
  th = 0;
  earlyAT = 0;
  lateAT = 0;
  PinType = type;
  previous = NULL;
  pthread_mutex_init(&StreamMutex,NULL);
  next = NULL;
}

Pin::Pin(string name, Wire* w)
{
  PinName = name;
  ts = 0;
  th = 0;
  earlyAT = 0;
  lateAT = 0;
  PinType = 0;
  previous = w;
  next = NULL;
  pthread_mutex_init(&StreamMutex,NULL);
}

Pin::Pin(string name, int primary, double e, double l)
{
  PinName = name;
  ts = 0;
  th = 0;
  earlyAT = e;
  lateAT = l;
  PinType = primary;
  previous = NULL;
  next = NULL;
  pthread_mutex_init(&StreamMutex,NULL);
}

Pin::Pin(string name, int primary)
{
  PinName = name;
  ts = 0;
  th = 0;
  earlyAT = 0;
  lateAT = 0;
  PinType = primary;
  previous = NULL;
  next = NULL;
  pthread_mutex_init(&StreamMutex,NULL);
}

Pin::Pin(Wire* w, string name)
{
  PinName = name;
  ts = 0;
  th = 0;
  earlyAT = 0;
  lateAT = 0;
  PinType = 0;
  previous = NULL;
  next = w;
  pthread_mutex_init(&StreamMutex,NULL);
}

Pin::~Pin()
{
  delete previous;
  delete next;
  pthread_mutex_destroy(&StreamMutex);
}

void Pin::ChangeTS(double t)
{
  ts = t;
}

void Pin::ChangeTH(double t)
{
  th = t;
}

const double Pin::GetTH()
{
  return th;
}

const double Pin::GetTS()
{
  return ts;
}

void Pin::SetEarlyAT(double t)
{
  earlyAT = t;
}

void Pin::SetLateAT(double t)
{
  lateAT = t;
}

const double Pin::GetEarlyAT()
{
  return earlyAT;
}

const double Pin::GetLateAT()
{
  return lateAT;
}

void Pin::SetNextWire(Wire* w)
{
  next = w;
}

void Pin::SetPreviousWire(Wire *w)
{
  previous = w;
}

Wire* Pin::GetNextWire() const
{
  return next;
}

Wire* Pin::GetPreviousWire() const
{
  return previous;
}

const string Pin::GetPinName()
{
  return PinName;
}

void Pin::SetPinType(int b)
{
  PinType = b;
}

const int Pin::GetPinType() 
{
  return PinType;
}


void Pin::LockMutex()
{
  pthread_mutex_lock (&StreamMutex);
}

void Pin::UnlockMutex()
{
  pthread_mutex_unlock (&StreamMutex);
}
