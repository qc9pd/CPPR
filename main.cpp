#include <iostream>
#include <pthread.h>
#include "Netlist.h"
#include "Pin.h"
#include "Netlist.h"
#include <cstring>
#include <exception>

#define NUMTHREADS1 3
#define THREADS 8

using namespace std;

void my_terminate()
{
  cout << "oops..." << endl;
}

static const bool SET_TERMINATE = std::set_terminate(my_terminate);

int main(int argc, char** argv)
{
  int NUM_TEST, NUM_PATH;
//xxx   try{
  //delayfile timingfile outputfile [-setup -NUMTEST 10 -NUMPATH 1]
  //delayfile timingfile outputfile [-setup -NUMTEST -NUMPATH 1]
  //delayfile timingfile outputfile [-setup -NUMTEST 10 -NUMPATH]
  if (argc==7){
    NUM_TEST = 0;
    NUM_PATH = numeric_limits<int>::max();}
  else if (argc==8){
    if (atoi(argv[6])!=0){
      NUM_TEST = atoi(argv[6]);
      NUM_PATH = numeric_limits<int>::max();}
    else if (atoi(argv[6])==0){
      if (atoi(argv[7])!=0){
	NUM_TEST = 0;
	NUM_PATH = atoi(argv[7]);}
      else{
	cout<<"command error: please reset option!"<<endl;}
    }}
  else if (argc == 9){
    NUM_TEST = atoi(argv[6]);
    NUM_PATH = atoi(argv[8]);}
  else{
    cout<<"The usage of the timer is:"<<endl;
    cout<<"./MST delayfile timingfile outputfile [-setup/-both/-hold -NUMTEST 10 -NUMPATH 10]"<<endl;
    return -1;}
    
   
  string df(argv[1]);
  string tf(argv[2]);
  string of(argv[3]);
  //string n="DFFR_X2_1:D";
  //string m="vsource:G2";
  Netlist* Netlist1;
  string Mode(argv[4]);
  Mode.erase(Mode.begin());
  cout<<Mode<<endl;
  
  
  Netlist1 = new Netlist(df,tf,of,Mode,NUM_TEST,NUM_PATH);
  
  Netlist1->ReadDelayFile();
  Netlist1->ReadTimingFile();
  /*int NUM;
  NUM = Netlist1->GetDPinsSize();
  pthread_mutex_t StreamMutex[NUM];
  pthread_mutex_init(&StreamMutex[NUM],NULL);*/
  
  //Netlist1->TestHashTable(n,m);
  Netlist1->VectorDuplicate();
  
  pthread_t thread1[2];
  pthread_t thread2[8];
  pthread_t thread3[8];
  pthread_attr_t attr;
  int rc1 , rc2, rc3[8], rc4[8];
  void* status;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
  
  MEMBER_FUN memfun_ptr1 = &Netlist::CLKPathEnumeration;
  NORMAL_FUN normal_ptr1 = *(NORMAL_FUN*)&memfun_ptr1;
  
  MEMBER_FUN memfun_ptr2 = &Netlist::SetATForAllPins;
  NORMAL_FUN normal_ptr2 = *(NORMAL_FUN*)&memfun_ptr2;

  MEMBER_FUN memfun_ptr3 = &Netlist::DataPathEnumeration;
  NORMAL_FUN normal_ptr3 = *(NORMAL_FUN*)&memfun_ptr3;

  MEMBER_FUN memfun_ptr4 = &Netlist::CalculateSlackSetup;
  NORMAL_FUN normal_ptr4 = *(NORMAL_FUN*)&memfun_ptr4;
  
  MEMBER_FUN memfun_ptr5 = &Netlist::CalculateSlackHold;
  NORMAL_FUN normal_ptr5 = *(NORMAL_FUN*)&memfun_ptr5;


  rc1 = pthread_create(&thread1[0],&attr,normal_ptr1,Netlist1);
  if (rc1){
    cout<<"ERROR Code for thread 1 "<<rc1<<endl;
    exit(-1);}
  
  rc2 = pthread_create(&thread1[1],&attr,normal_ptr2,Netlist1);
  if (rc2){
    cout<<"ERROR Code for thread 2 "<<rc2<<endl;
    exit(-1);}
  
  
  rc1 = pthread_join(thread1[0] , &status);
  if (rc1){
    cout<<"1"<<endl;
    exit(-2);}
  
  rc2 = pthread_join(thread1[1] , &status);
  if (rc2){
    cout<<"2"<<endl;
    exit(-2);}
  

  for (unsigned i = 0; i<THREADS;i++){
    rc3[i] = pthread_create(&thread2[i],&attr,normal_ptr3,Netlist1);
    if (rc3[i]){
      cout<<"ERROR Code for thread3["<<i<<"] "<<rc3[i]<<endl;
      exit(-1);}
  }

  for (unsigned i=0;i<THREADS;i++){
    rc3[i] = pthread_join(thread2[i], &status);
    if (rc3[i]){
      cout<<"ERROR Code for thread3["<<i<<"] "<<rc3[i]<<endl;
      exit(-1);}
  }
  
  /*
  Netlist1->CLKPathEnumeration();
  Netlist1->DataPathEnumeration();
  Netlist1->SetATForAllPins();
  */

  if(Mode == "-setup"){
    Netlist1->CopyDPins();
    for (unsigned i = 0; i<THREADS;i++){
      rc4[i] = pthread_create(&thread3[i],&attr,normal_ptr4,Netlist1);
      if (rc4[i]){
	cout<<"ERROR Code for thread3["<<i<<"] "<<rc4[i]<<endl;
	exit(-1);}}
    for (unsigned i=0;i<THREADS;i++){
      rc4[i] = pthread_join(thread3[i], &status);
      if (rc4[i]){
	cout<<"ERROR Code for thread3["<<i<<"] "<<rc4[i]<<endl;
	exit(-1);}}
    Netlist1->GenerateSetupReport();
    Netlist1->RenameSetup();}
  else if (Mode == "-hold"){
    Netlist1->CopyDPins();
    for (unsigned i = 0; i<THREADS;i++){
      rc4[i] = pthread_create(&thread3[i],&attr,normal_ptr5,Netlist1);
      if (rc4[i]){
	cout<<"ERROR Code for thread3["<<i<<"] "<<rc4[i]<<endl;
	exit(-1);}}
    for (unsigned i=0;i<THREADS;i++){
      rc4[i] = pthread_join(thread3[i], &status);
      if (rc4[i]){
	cout<<"ERROR Code for thread3["<<i<<"] "<<rc4[i]<<endl;
	exit(-1);}}
    Netlist1->GenerateHoldReport();
    Netlist1->RenameHold();}
  else{
    Netlist1->CopyDPins();
    for (unsigned i = 0; i<THREADS;i++){
      rc4[i] = pthread_create(&thread3[i],&attr,normal_ptr4,Netlist1);
      if (rc4[i]){
	cout<<"ERROR Code for thread3["<<i<<"] "<<rc4[i]<<endl;
	exit(-1);}}
    for (unsigned i=0;i<THREADS;i++){
      rc4[i] = pthread_join(thread3[i], &status);
      if (rc4[i]){
	cout<<"ERROR Code for thread3["<<i<<"] "<<rc4[i]<<endl;
	exit(-1);}}
    Netlist1->GenerateSetupReport();
    Netlist1->CopyDPins();
    for (unsigned i = 0; i<THREADS;i++){
      rc4[i] = pthread_create(&thread3[i],&attr,normal_ptr5,Netlist1);
      if (rc4[i]){
	cout<<"ERROR Code for thread3["<<i<<"] "<<rc4[i]<<endl;
	exit(-1);}}
    for (unsigned i=0;i<THREADS;i++){
      rc4[i] = pthread_join(thread3[i], &status);
      if (rc4[i]){
	cout<<"ERROR Code for thread3["<<i<<"] "<<rc4[i]<<endl;
	exit(-1);}}
    Netlist1->GenerateHoldReport();
    Netlist1->RenameBoth();}

  Netlist1->CleaningProcess();
  pthread_attr_destroy(&attr);
//xxx }
  
//xxx   catch(std::exception e){
//xxx     cout << e.what() << endl;
//xxx   }
//xxx   catch(boost::exception &e){
//xxx     cout << "boost exception" << endl;
//xxx   }
//xxx  
  //pthread_mutex_destroy(&StreamMutex[NUM]);
  pthread_exit(NULL);
  return 1;
}
