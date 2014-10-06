#include "Netlist.h"
#include <fstream>
#include <algorithm>
#include <iostream>

using namespace std;

Netlist::Netlist(string df, string tf, string output, string mode, int num_test, int num_path)
{
  DelayFileName = df;
  TimingFileName = tf;
  OutputFileName = output;
  T=0;
  CLKSource = NULL;
  Mode = mode;
  NUM_TEST = num_test;
  NUM_PATH = num_path;
  pthread_mutex_init(&VectorMutex,NULL);
  pthread_mutex_init(&SlackMutex,NULL);
  pthread_mutex_init(&PathVMutex,NULL);
  pthread_mutex_init(&TestNumMutex,NULL);
}

Netlist::~Netlist()
{
  pthread_mutex_destroy (&VectorMutex);
  pthread_mutex_destroy (&SlackMutex);
  pthread_mutex_destroy (&PathVMutex);
  pthread_mutex_destroy (&TestNumMutex);
}

void Netlist::ReadDelayFile()
{
  fstream DelayFile;
  string line;
  char* line_b;
  char delim[]=" ";
  char* token;
  DelayFile.open(DelayFileName.c_str(),fstream::in);

  if (!DelayFile.is_open()){
    cout<<"error: open file "<<DelayFileName<<" failed!"<<endl;}

  else{
    while(!DelayFile.eof()){
      getline(DelayFile,line);
      
      if (line.length()==0){
	continue;}

      line_b = new char [line.length()+1];
      strcpy(line_b, line.c_str());
      token = strtok(line_b,delim);
      //parsing the input definement part
      if (strcmp(token,"input")==0){
	Pin* PrimaryI;
	token=strtok(NULL,delim);
	string PI(token);
	string pinname="vsource:";
	token=strtok(NULL,delim);
	pinname.append(PI);
	PrimaryI = new Pin(pinname,1);
	PinMap[pinname]=PrimaryI;
	PrimaryInputs.push_back(PrimaryI);
	cout<<"the source pin is "<<pinname<<endl;}
      
      //parsing the output definement part
      else if(strcmp(token,"output")==0){
	token=strtok(NULL,delim);
	string PO(token);
	string pinname="vsink:";
	pinname.append(PO);
	cout<<"the sink pin is "<<pinname<<endl;}
      
      //parsing the setup part
      else if(strcmp(token,"setup")==0){
	Pin* DPin;
	Pin* CKPin;
	token=strtok(NULL,delim);
	string LatchD(token);
	token=strtok(NULL,delim);
	string LatchCLK(token);
	token=strtok(NULL,delim);
	string tse(token);	
	double ts = atof(tse.c_str());
	
	//for double check if stores D in LatchCK, CK in LatchD
	if ((LatchD.at(LatchD.length()-1)!='D') && (LatchCLK.at(LatchCLK.length()-1)!='K')){
	  string temp;
	  temp = LatchD;
	  LatchD = LatchCLK;
	  LatchCLK = temp;}
	
	if(PinMap[LatchD]!=NULL){
	  PinMap[LatchD]->ChangeTS(ts);
	  PinMap[LatchD]->SetPinType(3);
	  DPins.push_back(PinMap[LatchD]);}
	else if(PinMap[LatchD]==NULL){
	  DPin = new Pin(LatchD,ts,3);
	  PinMap[LatchD] = DPin;
	  DPins.push_back(PinMap[LatchD]);}

	if(PinMap[LatchCLK]!=NULL){
	  PinMap[LatchCLK]->ChangeTS(ts);
	  PinMap[LatchCLK]->SetPinType(4);
	  CKPins.push_back(PinMap[LatchCLK]);}
	else if(PinMap[LatchCLK]==NULL){
	  CKPin = new Pin(LatchCLK,ts,4);
	  PinMap[LatchCLK] = CKPin;
	  CKPins.push_back(PinMap[LatchCLK]);}

	cout<<"the set up time of the "<<LatchD<<" and "<<LatchCLK<<" is "<<ts<<endl;}


      //parsing the hold part
      else if(strcmp(token,"hold")==0){
	token=strtok(NULL,delim);
	string LatchD(token);
	token=strtok(NULL,delim);
	string LatchCLK(token);
	token=strtok(NULL,delim);
	string tho(token);
	double th = atof(tho.c_str());

	if ((PinMap[LatchD]!=NULL) && (PinMap[LatchCLK]!=NULL)){
	  PinMap[LatchD]->ChangeTH(th);
	  PinMap[LatchCLK]->ChangeTH(th);}

	else{
	  cout<<"Error: "<<LatchD<<" and "<<LatchCLK<<"has not been created yet! please check the code on setup grammar part"<<endl;}
	cout<<"the hold time of the "<<LatchD<<" and "<<LatchCLK<<" is "<<th<<endl;}

      //parsing the wire part
      else{
	Wire* ww;
       	string SourcePin(token);
	token=strtok(NULL,delim);
	string SinkPin(token);
	token=strtok(NULL,delim);
	string Early(token);
	double EarlyDelay = atof(Early.c_str());
	token = strtok(NULL,delim);
	string Late(token);
	double LateDelay = atof(Late.c_str());
       
	if((PinMap[SinkPin]==NULL) && (PinMap[SourcePin]!=NULL)){
	  ww  =  new Wire(SourcePin,SinkPin,EarlyDelay,LateDelay);
	  string WireName=SourcePin;
	  WireName.append(" ");
	  WireName.append(SinkPin);
	  Pin* sink = new Pin(SinkPin,ww);
	  PinMap[SinkPin] = sink;
	  ww->SetSink(sink);
	  if (PinMap[SourcePin]->GetNextWire()==NULL){
	    PinMap[SourcePin]->SetNextWire(ww);}
	  else if (PinMap[SourcePin]->GetNextWire()!=NULL){
	    Wire* temp = PinMap[SourcePin]->GetNextWire();       //big concern about it
	    while(temp->GetONextWire()!=NULL){
	      temp = temp->GetONextWire();}
	    temp->SetONextWire(ww);}
	    
	  ww->SetSource(PinMap[SourcePin]);
	  WireMap[WireName] = ww;}

	else if((PinMap[SinkPin]!=NULL) && (PinMap[SourcePin]!=NULL)){
	  ww = new Wire(EarlyDelay, LateDelay, SourcePin, SinkPin);
	  string WireName=SourcePin;
	  WireName.append(" ");
	  WireName.append(SinkPin);
	  if (PinMap[SourcePin]->GetNextWire()==NULL){
	    PinMap[SourcePin]->SetNextWire(ww);}
	  else if (PinMap[SourcePin]->GetNextWire()!=NULL){
	    Wire* temp =  PinMap[SourcePin]->GetNextWire();       //big concern about it
	    while(temp->GetONextWire()!=NULL){
	      temp = temp->GetONextWire();}

	    temp->SetONextWire(ww);}

	  if (PinMap[SinkPin]->GetPreviousWire()==NULL){
	    PinMap[SinkPin]->SetPreviousWire(ww);}
	  else if (PinMap[SinkPin]->GetPreviousWire()!=NULL){
	    Wire* temp = PinMap[SinkPin]->GetPreviousWire();
	    while (temp->GetINextWire()!=NULL){
	      temp = temp->GetINextWire();}
	    temp->SetINextWire(ww) ;}
	  ww->SetSource(PinMap[SourcePin]);
	  ww->SetSink(PinMap[SinkPin]);
	  WireMap[WireName] = ww;} 

	else if ((PinMap[SinkPin]==NULL) && (PinMap[SourcePin]==NULL)){
	  ww  =  new Wire(SourcePin,SinkPin,EarlyDelay,LateDelay);
	  string WireName=SourcePin;
	  WireName.append(" ");
	  WireName.append(SinkPin);	 
	  Pin* source = new Pin(ww,SourcePin);
	  Pin* sink = new Pin(SinkPin,ww);
	  PinMap[SinkPin] = sink;
	  ww->SetSink(sink);
	  PinMap[SourcePin] = source;
	  ww->SetSource(source);
	  WireMap[WireName] = ww;}

	else if ((PinMap[SinkPin]!=NULL) && (PinMap[SourcePin]==NULL)){
	  ww = new Wire(SourcePin,SinkPin,EarlyDelay,LateDelay);
	  string WireName=SourcePin;
	  WireName.append(" ");
	  WireName.append(SinkPin);
	  Pin* source = new Pin(ww,SourcePin);
	  PinMap[SourcePin] = source;
	  ww->SetSource(source);
	  if (PinMap[SinkPin]->GetPreviousWire()==NULL){
	    PinMap[SinkPin]->SetPreviousWire(ww);}
	  else if (PinMap[SinkPin]->GetPreviousWire()!=NULL){
	    Wire* temp = PinMap[SinkPin]->GetPreviousWire();       //big concern about it
	    while(temp->GetINextWire()!=NULL){
	      temp = temp->GetINextWire();}
	    temp->SetINextWire(ww);}
	    
	  ww->SetSink(PinMap[SinkPin]);
	  WireMap[WireName] = ww;}
	  
	  
	

	else{
	  cout<<"Error: "<<SinkPin<< "parsing delay file error! Seriously??"<<endl;}
	
	
	cout<<"the wire from "<<SourcePin<<" to "<<SinkPin<<endl;
	cout<<"Early Delay: "<<EarlyDelay<<endl;
	cout<<"Late Delay: "<<LateDelay<<endl;}
    }
  }
  delete line_b;
}

void Netlist::SetCLKSource(Pin *p)
{
  CLKSource = p;
}


void Netlist::ReadTimingFile()
{
  fstream Timing;
  char delim[]=" ";
  char* line_b;
  string line;
  char* token2;
  Timing.open(TimingFileName.c_str(),fstream::in);

  if(!Timing.is_open()){
    return;}

  while(!Timing.eof()){
    getline(Timing,line);

    if (line.length()==0){
      continue;}

    line_b = new char[line.length()+1];
    strcpy(line_b,line.c_str());
    token2 = strtok(line_b,delim);
    
    if (strcmp(token2,"clock")==0){
      token2 = strtok(NULL,delim);
      string PartialCLKSource(token2);
      string CLKSource = "vsource:";
      CLKSource.append(PartialCLKSource);
      PinMap[CLKSource]->SetPinType(5);
      SetCLKSource(PinMap[CLKSource]);
      token2 = strtok(NULL,delim);
      string Period(token2);
      T = atof(Period.c_str());}
    
    else if (strcmp(token2,"at")==0){
      token2 = strtok(NULL,delim);
      string PartialPin(token2);
      string Pin = "vsource:";
      Pin.append(PartialPin);
      token2 = strtok(NULL,delim);
      string e_at(token2);
      token2 = strtok(NULL,delim);
      string l_at(token2);
      PinMap[Pin]->SetEarlyAT(atof(e_at.c_str()));
      PinMap[Pin]->SetLateAT(atof(l_at.c_str()));}
    
  }
}

unsigned Netlist::GetDPinsSize()
{
  return DPins.size();
}

void Netlist::FindDataPath(Pin* Start, unordered_map<string, string> &Former )
{
  //unordered_map<string, string> Former;
  Wire* next;
  next = Start->GetNextWire();
  while (next != NULL){
    Wire* cW;
    cW = next->GetSink()->GetNextWire();
    if (cW!=NULL){
      Former[cW->GetWireName()] = next->GetWireName();
      }
    Pin* CPin;
    CPin = next->GetSink();
    if (next->GetSink()->GetPinType() == 3){
      fstream DataPath;
      Wire* current;
      string FileName = next->GetSink()->GetPinName();
      next->GetSink()->LockMutex();
      DataPath.open(FileName.c_str(),fstream::app|fstream::out);
      DataPath<<next->GetWireName();
      //current = next->GetFormerWire();
      current = WireMap[Former[next->GetWireName()]];
      while (current != NULL){
	DataPath<<"\t\t";
	DataPath<<current->GetWireName();
	//current = current->GetFormerWire();
	current = WireMap[Former[current->GetWireName()]];
      }
      DataPath<<"\n"<<endl;
      DataPath.close();
      next->GetSink()->UnlockMutex();}
    else{
      FindDataPath(CPin,Former);}

    //Wire* LastFormer = next->GetFormerWire();
    Wire* LastFormer = WireMap[Former[next->GetWireName()]];
    next = next->GetONextWire();    //
    if (next!=NULL){
      if (LastFormer!=NULL){
      //next->SetFormerWire(LastFormer);}  //keep this destination node linked with the previous part
	Former[next->GetWireName()] = LastFormer->GetWireName();} }
  }
}

void Netlist::VectorDuplicate()
{
  PInputs = PrimaryInputs;
  //PInputs.push_back(CLKSource);
}

void* Netlist::DataPathEnumeration(void* t)
{
  unordered_map<string,string> Former;
  
  while (!PInputs.empty()){  
    pthread_mutex_lock (&VectorMutex);
    //cout<<"Right Now "<<PInputs.back()->GetPinName()<<endl;
    Pin* pPin = PInputs.back();
    PInputs.pop_back();
    pthread_mutex_unlock (&VectorMutex);
    // cout<<pPin->GetPinName()<<endl;
    FindDataPath(pPin,Former);
    Former.clear();
    
  }
  /*
  for (unsigned i=0;i<PrimaryInputs.size();i++){
    if (PrimaryInputs[i]->GetPinType() == 1){
      Pin* pPin = PrimaryInputs[i];
      FindDataPath(pPin,Former);
      Former.clear();}}
  FindDataPath(CLKSource,Former);
  Former.clear();
  */
  pthread_exit((void*) 0);
}

void Netlist::FindCLKPath(Pin* Start,unordered_map<string,string> &Former)
{
  Wire* next;
  next = Start->GetNextWire();
  while (next != NULL){
    Wire* cW;
    cW = next->GetSink()->GetNextWire();
    if (cW!=NULL){
      //cW->SetFormerWire(next);}
      Former[cW->GetWireName()] = next->GetWireName(); }
    Pin* CPin;
    CPin = next->GetSink();
    if (next->GetSink()->GetPinType() == 4){
      fstream CLKPath;
      Wire* current;
      string FileName = next->GetSink()->GetPinName();
      CLKPath.open(FileName.c_str(),fstream::app|fstream::out);
      CLKPath<<next->GetWireName();
      //current = next->GetFormerWire();
      current = WireMap[Former[next->GetWireName()]];
      while (current != NULL){
	CLKPath<<"\t\t";
	CLKPath<<current->GetWireName();
	current = WireMap[Former[current->GetWireName()]];}
      CLKPath<<"\n"<<endl;
      CLKPath.close();}
    else{
      FindCLKPath(CPin,Former);}

    Wire* LastFormer = WireMap[Former[next->GetWireName()]];
    next = next->GetONextWire();
    if (next!=NULL){
      // next->SetFormerWire(LastFormer);}  //keep this destination node linked with the previous part
      if (LastFormer!=NULL){
	Former[next->GetWireName()] = LastFormer->GetWireName();}}
  }
}

Pin* Netlist::GetCLKSource() const
{
  return CLKSource;
}

void* Netlist::CLKPathEnumeration(void *t)
{
  unordered_map<string,string> Former;
  FindCLKPath(CLKSource,Former);
  Former.clear();
  pthread_exit((void*) 0);
}


void Netlist::SetAT(Pin* Start)
{
  Wire* next;
  next = Start->GetNextWire();
  while (next != NULL){
    Wire* cW;
    cW = next->GetSink()->GetNextWire();
    if (cW!=NULL){
      cW->SetFormerWire(next);}
    Pin* CPin;
    CPin = next->GetSink();

    if ((CPin->GetEarlyAT() == 0 ) || (CPin->GetEarlyAT() > Start->GetEarlyAT() + next->GetEarlyDelay())){
      double t;
      t = Start->GetEarlyAT() + next->GetEarlyDelay();
      CPin -> SetEarlyAT(t);}

    if ((CPin->GetLateAT() == 0) || (CPin->GetLateAT() < Start->GetLateAT() + next->GetEarlyDelay())){
      double t;
      t = Start->GetLateAT() + next->GetLateDelay();
      CPin -> SetLateAT(t);}

    if (cW != NULL){
      SetAT(CPin);}

    Wire* LastFormer = next->GetFormerWire();
    next = next->GetONextWire();
    if (next!=NULL){
      next->SetFormerWire(LastFormer);}  //keep this destination node linked with the previous part
  }
}

void* Netlist::SetATForAllPins(void* t)
{
  for (unsigned i=0; i<PrimaryInputs.size();i++){
    SetAT(PrimaryInputs[i]);}
  SetAT(CLKSource);
  pthread_exit((void*) t);
}

bool ComparePostSetupPath(Path* A, Path* B)
{
  return (A->GetPostSlackSetup() < B->GetPostSlackSetup());
}

bool ComparePreSetupPath(Path* A, Path* B)
{
  return (A->GetPreSlackSetup() < B->GetPreSlackSetup());
}

bool ComparePreHoldPath(Path* A, Path* B)
{
  return (A->GetPreSlackHold() < B->GetPreSlackHold());
}

bool ComparePostHoldPath(Path* A, Path* B)
{
  return (A->GetPostSlackHold() < B->GetPostSlackHold());
}

/*
  bool SetupTest(string A, string B)
  {
  return (Netlist::SetTestNum[A] < Netlist::SetTestNum[B]);
  }

  bool HoldTest(string A, string B)
  {
  return (Netlist::HoldTestNum[A] < Netlist::HoldTestNum[B]);
  }
*/

void Netlist::CopyDPins()
{
  DPins_Copy = DPins;
}

void* Netlist::CalculateSlackSetup(void* t)
{
  fstream CLKFile;
  fstream DFile;
  string DLine;
  string CLKLine;
  char* DLine_b;
  char* CLKLine_b;
  char* token1;
  char* token2;
  char delim[]="\t";
  
  string LateCLK;
  string EarlyCLK;
  double l0_late;
  double l0_early;
  double pre_slack_hold;
  double pre_slack_setup;
  double credit_hold;
  double credit_setup;
  double post_slack_setup;
  double post_slack_hold;
  unordered_map<string, int> CLKPath;
  vector<Path*> PathV;
  char* posptr;
  
  //for (unsigned i=0; i<DPins.size(); i++){
  while(!DPins_Copy.empty()){
    pthread_mutex_lock (&SlackMutex);
    Pin* CurrentDPin = DPins_Copy.back();
    DPins_Copy.pop_back();
    pthread_mutex_unlock (&SlackMutex);
    if (CurrentDPin == NULL){
      pthread_exit ((void*) t);}
    string DPin = CurrentDPin->GetPinName();
    /*
      if (DPin == "DFFR_X2_1:D"){
      cout<<"CLK check!"<<endl;}
    */
    string CLKPin = DPin;
    size_t npos;
    npos = CLKPin.find(':');
    CLKPin.erase(CLKPin.begin()+npos+1 , CLKPin.end());
    CLKPin.append("CK");
    /*
      This Part is for Mac use(Mac FileName don't allow ":")! Remove it when use Linux to compile
    */
    //replace(DPin.begin(), DPin.end(), ':', '/');    
    //replace(CLKPin.begin(), CLKPin.end(), ':', '/');
    /*
      This Part is over
    */
    DFile.open(DPin.c_str(), fstream::in);
    CLKFile.open(CLKPin.c_str(), fstream::in);
    
    if ((!DFile.is_open()) || (!CLKFile.is_open())){
      cout<<"Error: Cannot Find One of the Data File or Clock File or both!"<<endl;
      pthread_exit((void*) 0);}
    else {
      fstream output;
      string outfile = CurrentDPin->GetPinName();
      //outfile.erase(outfile.end()-1);
      outfile.append("OUT");
      pthread_mutex_lock (&SlackMutex);
      LatchV.push_back(outfile);
      pthread_mutex_unlock (&SlackMutex);
      output.open(outfile.c_str(),fstream::app|fstream::out);
      if (!output.is_open()){
	cout<<"file not open!"<<endl;}
      l0_early = 0;
      l0_late = 0;
      LateCLK = "";
      EarlyCLK = "";
      while (!CLKFile.eof()){
	getline(CLKFile,CLKLine);
	if (CLKLine.length()==0){
	  continue;}
	CLKLine_b = new char [CLKLine.length()+1];
	strcpy(CLKLine_b,CLKLine.c_str());
	token1 = strtok_r(CLKLine_b,delim,&posptr);
	double early = 0;
	double late = 0;
	while (token1!=NULL){
	  string Wire1(token1);
	  early += WireMap[Wire1]->GetEarlyDelay();
	  late += WireMap[Wire1]->GetLateDelay();
	  if (WireMap[Wire1]->GetSource()->GetPinType() == 5){
	    early += WireMap[Wire1]->GetSource()->GetEarlyAT();
	    late += WireMap[Wire1]->GetSource()->GetLateAT();}
	  token1 = strtok_r(NULL,delim,&posptr);}
	
	if ((l0_early == 0) && (l0_late ==0 )){
	  l0_early = early;
	  l0_late = late;
	  LateCLK = CLKLine;
	  EarlyCLK = CLKLine;}
	
	else{
	  if (l0_early > early){
	    l0_early = early;
	    EarlyCLK = CLKLine;}
	  if (l0_late < late){
	    l0_late = late;
	    LateCLK = CLKLine;}}
      }
      
      char* LineChar;
      char* token3;
      LineChar = new char [EarlyCLK.length()+1];
      strcpy(LineChar,EarlyCLK.c_str());
      token3 = strtok_r(LineChar,delim,&posptr);
      while (token3!=NULL){
	string Wire3(token3);
	CLKPath[Wire3] = 1;
	token3 = strtok_r(NULL,delim,&posptr);}
      delete LineChar;
      LineChar = new char [LateCLK.length()+1];
      strcpy(LineChar,LateCLK.c_str());
      token3 = strtok_r(LineChar,delim,&posptr);
      while (token3!=NULL){
	string Wire3(token3);
	if (CLKPath[Wire3] == 1){
	  CLKPath[Wire3] = 3;}
	else{
	  CLKPath[Wire3] = 2;}
	token3 = strtok_r(NULL,delim,&posptr);}
      delete LineChar;

      
      while (!DFile.eof()){
	getline(DFile,DLine);
	if (DLine.length() == 0){
	  continue;}
	DLine_b = new char [DLine.length()+1];
	strcpy(DLine_b,DLine.c_str());
	token2 = strtok_r(DLine_b,delim,&posptr);
	pre_slack_setup = 0;
	post_slack_setup = 0;
	credit_setup = 0;
	double t_setup;

	while (token2 != NULL){
	  string Wire2(token2);

	  if (WireMap[Wire2]->GetSink()->GetPinType() == 3){
	    t_setup = WireMap[Wire2]->GetSink()->GetTS();}
	  if ((WireMap[Wire2]->GetSource()->GetPinType() == 1) || (WireMap[Wire2]->GetSource()->GetPinType() == 5)){
	    pre_slack_setup += WireMap[Wire2]->GetSource()->GetLateAT();}

	  pre_slack_setup += WireMap[Wire2]->GetLateDelay();
	    
	  if ((CLKPath[Wire2] == 1) || (CLKPath[Wire2] == 3)){
	    credit_setup += WireMap[Wire2]->GetLateDelay() - WireMap[Wire2]->GetEarlyDelay();}
	    
	  token2 = strtok_r(NULL,delim,&posptr);}

	
	pre_slack_setup = -pre_slack_setup + l0_early - t_setup + T;
	post_slack_setup = pre_slack_setup + credit_setup;
	if (pre_slack_setup < 0){
	  //cout<<DLine<<endl;
	  if (DLine == "")
	    {
	      cout<<"wow!"<<endl;}
	  Path* pPath = new Path(pre_slack_setup, DLine);
	  pPath->SetPostSlackSetup(post_slack_setup);
	  pthread_mutex_lock (&PathVMutex);
	  PathV.push_back(pPath);
	  pthread_mutex_unlock (&PathVMutex);}
      }
      //cout<<"PathV size is "<<PathV.size()<<endl;
      if (PathV.size()>0){
	Path* PreMin = *min_element(PathV.begin(),PathV.end(),ComparePreSetupPath);
	sort(PathV.begin(),PathV.end(),ComparePostSetupPath);
	pthread_mutex_lock (&TestNumMutex);
	SetTestNum[outfile] = PathV[0]->GetPostSlackSetup();
	pthread_mutex_unlock (&TestNumMutex);
	  
	  
	int HowManyPath;
	int PathVSize = (int) PathV.size();
	HowManyPath = min(NUM_PATH , PathVSize);
	output<<"setup "<<PreMin->GetPreSlackSetup()<<" "<<PathV[0]->GetPostSlackSetup()<<" "<<HowManyPath<<endl;
	for (unsigned j=0;j<HowManyPath;j++){
	  string CPath = PathV[j]->GetPathName();
	  int NumOft = count(CPath.begin() , CPath.end() , '\t');
	  int NumofItem = NumOft/2 + 2;
	  output<<PathV[j]->GetPreSlackSetup()<<" "<<PathV[j]->GetPostSlackSetup()<<" "<<NumofItem<<endl;
	  char* token_t;
	  char* PathLine;
	  char delim_t[]="\t ";
	  
	  PathLine = new char [CPath.length()+1];
	  strcpy(PathLine,CPath.c_str());
	  token_t = strtok_r(PathLine,delim_t,&posptr);
	  string Buffer(token_t);
	  token_t = strtok_r(NULL,delim_t,&posptr);
	  output<<token_t<<endl;
	  token_t = strtok_r(NULL,delim_t,&posptr);
	  string SourceNode;
	  while (token_t!=NULL){
	    token_t = strtok_r(NULL,delim_t,&posptr);
	    output<<token_t<<endl;
	    token_t = strtok_r(NULL,delim_t,&posptr);
	    if (token_t!=NULL){
	      SourceNode = string(token_t);}
	  }
	  //SourceNode.erase(SourceNode.begin(),SourceNode.begin()+8);
	  size_t vsourcepos = SourceNode.find("vsource:");
	  if (vsourcepos != string::npos){
	    SourceNode.erase(SourceNode.begin() , SourceNode.begin()+8);}
	  output<<SourceNode<<endl;  
	  delete PathLine;
	}
      }
      

      for (unsigned i=0;i<PathV.size();i++){
	delete PathV[i];}
      PathV.clear();
      CLKPath.clear();
      delete DLine_b;
      delete CLKLine_b;
      DFile.close();
      CLKFile.close();   
    }
  }
  pthread_exit((void*) t);
}


void* Netlist::CalculateSlackHold(void* t)
{
  fstream CLKFile;
  fstream DFile;
  string DLine;
  string CLKLine;
  char* DLine_b;
  char* CLKLine_b;
  char* token1;
  char* token2;
  char delim[]="\t";
  
  string LateCLK;
  string EarlyCLK;
  double l0_late;
  double l0_early;
  double pre_slack_hold;
  double pre_slack_setup;
  double credit_hold;
  double credit_setup;
  double post_slack_setup;
  double post_slack_hold;
  vector<Path*> PathV;
  unordered_map<string,int> CLKPath;
  char* posptr;
  
  //for (unsigned i=0; i<DPins.size(); i++){
  while(!DPins_Copy.empty()){
    pthread_mutex_lock (&SlackMutex);
    Pin* CurrentDPin = DPins_Copy.back();
    DPins_Copy.pop_back();
    pthread_mutex_unlock (&SlackMutex);
    if (CurrentDPin == NULL){
      pthread_exit ((void*) t);}
    string DPin = CurrentDPin->GetPinName();
    /*
      if (DPin == "DFFR_X2_1:D"){
      cout<<"CLK check!"<<endl;}
    */
    string CLKPin = DPin;
    size_t npos;
    npos = CLKPin.find(':');
    CLKPin.erase(CLKPin.begin()+npos+1 , CLKPin.end());
    CLKPin.append("CK");
    /*
      This Part is for Mac use(Mac FileName don't allow ":")! Remove it when use Linux to compile
    */
    //replace(DPin.begin(), DPin.end(), ':', '/');    
    //replace(CLKPin.begin(), CLKPin.end(), ':', '/');
    /*
      This Part is over
    */
    DFile.open(DPin.c_str(), fstream::in);
    CLKFile.open(CLKPin.c_str(), fstream::in);
    
    if ((!DFile.is_open()) || (!CLKFile.is_open())){
      cout<<"Error: Cannot Find One of the Data File or Clock File or both!"<<endl;
      pthread_exit((void*) 0);}
    else {
      fstream output;
      string outfile = CurrentDPin->GetPinName();
      //outfile.erase(outfile.end()-1);
      outfile.append("OUT");
      pthread_mutex_lock (&SlackMutex);
      
      LatchV.push_back(outfile);
      pthread_mutex_unlock (&SlackMutex);
      output.open(outfile.c_str(),fstream::app|fstream::out);
      if (!output.is_open()){
	cout<<"file not open!"<<endl;}
      l0_early = 0;
      l0_late = 0;
      LateCLK = "";
      EarlyCLK = "";
      while (!CLKFile.eof()){
	getline(CLKFile,CLKLine);
	if (CLKLine.length()==0){
	  continue;}
	CLKLine_b = new char [CLKLine.length()+1];
	strcpy(CLKLine_b,CLKLine.c_str());
	token1 = strtok_r(CLKLine_b,delim,&posptr);
	double early = 0;
	double late = 0;
	while (token1!=NULL){
	  string Wire1(token1);
	  early += WireMap[Wire1]->GetEarlyDelay();
	  late += WireMap[Wire1]->GetLateDelay();
	  if (WireMap[Wire1]->GetSource()->GetPinType() == 5){
	    early += WireMap[Wire1]->GetSource()->GetEarlyAT();
	    late += WireMap[Wire1]->GetSource()->GetLateAT();}
	  token1 = strtok_r(NULL,delim,&posptr);}
	
	if ((l0_early == 0) && (l0_late ==0 )){
	  l0_early = early;
	  l0_late = late;
	  LateCLK = CLKLine;
	  EarlyCLK = CLKLine;}
	
	else{
	  if (l0_early > early){
	    l0_early = early;
	    EarlyCLK = CLKLine;}
	  if (l0_late < late){
	    l0_late = late;
	    LateCLK = CLKLine;}}
      }
      
      char* LineChar;
      char* token3;
      LineChar = new char [EarlyCLK.length()+1];
      strcpy(LineChar,EarlyCLK.c_str());
      token3 = strtok_r(LineChar,delim,&posptr);
      while (token3!=NULL){
	string Wire3(token3);
	CLKPath[Wire3] = 1;
	token3 = strtok_r(NULL,delim,&posptr);}
      delete LineChar;
      LineChar = new char [LateCLK.length()+1];
      strcpy(LineChar,LateCLK.c_str());
      token3 = strtok_r(LineChar,delim,&posptr);
      while (token3!=NULL){
	string Wire3(token3);
	if (CLKPath[Wire3] == 1){
	  CLKPath[Wire3] = 3;}
	else{
	  CLKPath[Wire3] = 2;}
	token3 = strtok_r(NULL,delim,&posptr);}
      delete LineChar;

      while (!DFile.eof()){
	getline(DFile,DLine);
	/*
	  if (DLine == "NOR2_X2_3:ZN DFFR_X2_1:D\t\tNOR2_X2_3:A1 NOR2_X2_3:ZN\t\tvsource:G2 NOR2_X2_3:A1"){
	  cout<<"finally here!"<<endl;	
	  }*/
	if (DLine.length() == 0){
	  continue;}
	DLine_b = new char [DLine.length()+1];
	strcpy(DLine_b,DLine.c_str());
	token2 = strtok_r(DLine_b,delim,&posptr);
	credit_hold = 0;
	post_slack_hold = 0;
	pre_slack_hold = 0;
	double t_hold;
	int CommonPointMark = 0;
	while (token2 != NULL){
	  string Wire2(token2);
       
	  if (WireMap[Wire2]->GetSink()->GetPinType() == 3){
	    t_hold = WireMap[Wire2]->GetSink()->GetTH();}
	  if ((WireMap[Wire2]->GetSource()->GetPinType() == 1) || (WireMap[Wire2]->GetSource()->GetPinType() == 5)){
	    pre_slack_hold += WireMap[Wire2]->GetSource()->GetEarlyAT();}
	  if(CommonPointMark == 0){
	    if ((CLKPath[Wire2] == 2) || (CLKPath[Wire2] == 3)){
	      double t1 = WireMap[Wire2]->GetSink()->GetLateAT();
	      double t2 = WireMap[Wire2]->GetSink()->GetEarlyAT();
	      credit_hold = WireMap[Wire2]->GetSink()->GetLateAT() - WireMap[Wire2]->GetSink()->GetEarlyAT() ; 
	      CommonPointMark = 1;}}   

	  pre_slack_hold += WireMap[Wire2]->GetEarlyDelay();
	  token2 = strtok_r(NULL,delim,&posptr);}

	
	pre_slack_hold = pre_slack_hold - l0_late - t_hold;
	post_slack_hold = pre_slack_hold + credit_hold;
	if (pre_slack_hold<0){
	  Path* pPath = new Path(DLine, pre_slack_hold);
	  pPath->SetPostSlackHold(post_slack_hold);
	  pthread_mutex_lock (&PathVMutex);
	  PathV.push_back(pPath);
	  pthread_mutex_unlock (&PathVMutex);}
      }
      if (PathV.size()>0){
	Path* PreMin = *min_element(PathV.begin(),PathV.end(),ComparePreHoldPath);
	sort(PathV.begin(),PathV.end(),ComparePostHoldPath);
	pthread_mutex_lock (&TestNumMutex);
	HoldTestNum[outfile] = PathV[0]->GetPostSlackHold();
	pthread_mutex_unlock (&TestNumMutex);
	  
	  
	int HowManyPath;
	int PathVSize = (int) PathV.size();
	HowManyPath = min(NUM_PATH , PathVSize);

	output<<"hold "<<PreMin->GetPreSlackHold()<<" "<<PathV[0]->GetPostSlackHold()<<" "<<HowManyPath<<endl;
	for (unsigned j=0;j<HowManyPath;j++){
	  string CPath = PathV[j]->GetPathName();
	  int NumOft = count(CPath.begin() , CPath.end() , '\t');
	  int NumofItem = NumOft/2 + 2;
	  output<<PathV[j]->GetPreSlackHold()<<" "<<PathV[j]->GetPostSlackHold()<<" "<<NumofItem<<endl;
	  char* token_t;
	  char* PathLine;
	  char delim_t[]="\t ";
	  
	  PathLine = new char [CPath.length()+1];
	  strcpy(PathLine,CPath.c_str());
	  token_t = strtok_r(PathLine,delim_t,&posptr);
	  string Buffer(token_t);
	  token_t = strtok_r(NULL,delim_t,&posptr);
	  output<<token_t<<endl;
	  token_t = strtok_r(NULL,delim_t,&posptr);
	  string SourceNode;
	  while (token_t!=NULL){
	    token_t = strtok_r(NULL,delim_t,&posptr);
	    output<<token_t<<endl;
	    token_t = strtok_r(NULL,delim_t,&posptr);
	    if (token_t!=NULL){
	      SourceNode = string(token_t);}
	  }
	  //SourceNode.erase(SourceNode.begin(),SourceNode.begin()+8);
	  size_t vsourcepos = SourceNode.find("vsource:");
	  if (vsourcepos != string::npos){
	    SourceNode.erase(SourceNode.begin() , SourceNode.begin()+8);}
	  output<<SourceNode<<endl;  
	  delete PathLine;
	}
      }
      

      for (unsigned i=0;i<PathV.size();i++){
	delete PathV[i];}
      PathV.clear();
      CLKPath.clear();
      delete DLine_b;
      delete CLKLine_b;
      DFile.close();
      CLKFile.close();   
    }
  }
  pthread_exit((void*) t);
}



void Netlist::GenerateSetupReport()
{
  
    fstream SetupResult;
    string SetupFile;
    int min;
    double s;
    int tests=1;
    SetupFile = DelayFileName;
    SetupFile.erase(SetupFile.end()-5,SetupFile.end());
    SetupFile.append("setup");
    SetupResult.open(SetupFile.c_str() , ios::out|ios::app);
    if (NUM_TEST!=0){
      while ((!LatchV.empty()) && (tests<=NUM_TEST)) {
	s=SetTestNum[LatchV[0]];
	min = 0;
	for (unsigned k = 0; k<LatchV.size();k++){
	  string TempFile = LatchV[k];
	  if (SetTestNum[TempFile] < s){
	    s = SetTestNum[TempFile];
	    min = k;}
	}
	fstream Temp;
	string File = LatchV[min];
	Temp.open(File.c_str(),ios::in);
	SetupResult<<Temp.rdbuf();
	Temp.close();
	LatchV.erase(LatchV.begin()+min);
	tests++;
      }
    }
    else{
      while ((!LatchV.empty())) {
	s=SetTestNum[LatchV[0]];
	min = 0;
	for (unsigned k = 0; k<LatchV.size();k++){
	  string TempFile = LatchV[k];
	  if (SetTestNum[TempFile] < s){
	    s = SetTestNum[TempFile];
	    min = k;}
	}
	fstream Temp;
	string File = LatchV[min];
	Temp.open(File.c_str(),ios::in);
	SetupResult<<Temp.rdbuf();
	Temp.close();
	LatchV.erase(LatchV.begin()+min);
      }
    }
      
    SetupResult.close();
  

  
  for (unsigned i = 0; i<DPins.size() ; i++){
    string DPin = DPins[i]->GetPinName();
    string OUT = DPin;
    OUT.append("OUT");
    if (remove(OUT.c_str())!=0){
      cout<<"Error deleting "<<OUT<<endl;}
  }
}


void Netlist::GenerateHoldReport()
{
  
    fstream HoldResult;
    string HoldFile;
    int min;
    double s;
    int tests = 1;
    HoldFile = DelayFileName;
    HoldFile.erase(HoldFile.end()-5,HoldFile.end());
    HoldFile.append("hold");
    HoldResult.open(HoldFile.c_str() , ios::out|ios::app);
    if (NUM_TEST != 0){
      while ((!LatchV.empty()) && (tests<=NUM_TEST)){
	  s=HoldTestNum[LatchV[0]];
	  min = 0;
	  for (unsigned k = 0; k<LatchV.size();k++){
	    string TempFile = LatchV[k];
	    if (HoldTestNum[TempFile] < s){
	      s = HoldTestNum[TempFile];
	      min = k;}
	  }
	  fstream Temp;
	  string File = LatchV[min];
	  Temp.open(File.c_str(),ios::in);
	  HoldResult<<Temp.rdbuf();
	  Temp.close();
	  LatchV.erase(LatchV.begin()+min);
	  tests++;}
	
    }
    else{
      while (!LatchV.empty()){
	  min=0;
	  s = HoldTestNum[LatchV[0]];
	  for (unsigned k = 0; k<LatchV.size();k++){
	    string TempFile = LatchV[k];
	    if (HoldTestNum[TempFile] < s){
	      s = HoldTestNum[TempFile];
	      min = k;}
	  }
	  fstream Temp;
	  string File = LatchV[min];
	  Temp.open(File.c_str(),ios::in);
	  HoldResult<<Temp.rdbuf();
	  Temp.close();
	  LatchV.erase(LatchV.begin()+min);}
    }
      
    HoldResult.close();
  

  
  for (unsigned i = 0; i<DPins.size() ; i++){
    string DPin = DPins[i]->GetPinName();
    string OUT = DPin;
    /*
      if (DPin == "DFFR_X2_1:D"){
      cout<<"CLK check!"<<endl;}
    
      string CLKPin = DPin;
      CLKPin.erase(CLKPin.end()-1);
      CLKPin.append("CK");
    */
    //OUT.erase(OUT.end()-1);
    OUT.append("OUT");
    if (remove(OUT.c_str())!=0){
      cout<<"Error deleting "<<OUT<<endl;}
  }
}


void Netlist::RenameBoth()
{
  fstream HoldResult;
  fstream SetupResult;
  fstream WholeFile;
  string HoldFile;
  string SetupFile;
  HoldFile = DelayFileName;
  SetupFile = DelayFileName;
  HoldFile.erase(HoldFile.end()-5,HoldFile.end());
  SetupFile.erase(SetupFile.end()-5,SetupFile.end());
  HoldFile.append("hold");
  SetupFile.append("setup");
  HoldResult.open(HoldFile.c_str() , ios::in);
  SetupResult.open(SetupFile.c_str(),ios::in);
  WholeFile.open(OutputFileName.c_str(),ios::app|ios::out);
  WholeFile<<SetupResult.rdbuf();
  WholeFile<<HoldResult.rdbuf();
  SetupResult.close();
  HoldResult.close();
  WholeFile.close();
  if ((remove(HoldFile.c_str())!=0) || (remove(SetupFile.c_str())!=0)){
    cout<<"Rename Error"<<endl;}
}


void Netlist::RenameSetup()
{
  string SetupFile;
  SetupFile = DelayFileName;
  SetupFile.erase(SetupFile.end()-5,SetupFile.end());
  SetupFile.append("setup");
  if (rename(SetupFile.c_str(),OutputFileName.c_str())!=0){
    cout<<"Rename Error!"<<endl;}
}

void Netlist::RenameHold()
{
  string HoldFile;
  HoldFile = DelayFileName;
  HoldFile.erase(HoldFile.end()-5,HoldFile.end());
  HoldFile.append("hold");
  if (rename(HoldFile.c_str(),OutputFileName.c_str())!=0){
    cout<<"Rename Error!"<<endl;}
}




void Netlist::CleaningProcess()
{
  for (unsigned i = 0; i<DPins.size() ; i++){
    string DPin = DPins[i]->GetPinName();
    
    string CLKPin = DPin;
    size_t npos;
    npos = CLKPin.find(':');
    CLKPin.erase(CLKPin.begin()+npos+1 , CLKPin.end());
    CLKPin.append("CK");
    if ((remove(DPin.c_str())!=0) || (remove(CLKPin.c_str())!=0)){
      //cout<<"Error deleting "<<DPin<<" "<<CLKPin<<endl;}
    }
  }
}



void Netlist::TestHashTable(string n,string m)
{
  cout<<"The Timing Information of "<<PinMap[n]->GetPreviousWire()->GetWireName()<<":"<<endl;
  cout<<"Early Delay: "<<PinMap[n]->GetPreviousWire()->GetEarlyDelay()<<endl;
  cout<<"Late Delaty: "<<PinMap[n]->GetPreviousWire()->GetLateDelay()<<endl;
  Wire* temp = PinMap[n]->GetPreviousWire();
  temp = temp->GetINextWire();
  while(temp!=NULL){
    cout<<"\n\n\n\n\n"<<endl;
    cout<<"The Next Wire is "<<temp->GetWireName()<<endl;
    cout<<"The timing information of which is "<<endl;
    cout<<"Early Delay: "<<temp->GetEarlyDelay()<<endl;
    cout<<"Late Delay: "<<temp->GetLateDelay()<<endl;
    temp = temp->GetINextWire();}

  cout<<"\n\n\n\n\n"<<PinMap[m]->GetEarlyAT()<<endl;
  cout<<"\n\n\n\n\n"<<PinMap[m]->GetLateAT()<<endl;
  /*
    for (unsigned i=0;i<PrimaryInput.size();i++){
    cout<<PrimaryInput[i]->GetPinName()<<endl;}
    cout<<"\n\n\n\n\n"<<endl;
    for (unsigned i=0;i<DPins.size();i++){
    cout<<DPins[i]->GetPinName()<<endl;}
    cout<<"\n\n\n\n\n"<<endl;
    for (unsigned i=0;i<CKPins.size();i++){
    cout<<CKPins[i]->GetPinName()<<endl;}
  */
  
}

