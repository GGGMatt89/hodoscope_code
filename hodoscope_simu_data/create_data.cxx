//c++ classes
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#pragma pack(1)
#include <stdint.h>
#include <unistd.h>
#include <ctime>
//custom classes
#include "functions.h"

using namespace std;
//This macro mimic the hodoscope data acquisition system
//It produces random numbers and saves them to binary files according to the hodoscope data structure
//The data files here are saved in the folder ./data/ with the naming structure runN-data_n.dat with N number of run, n number of file, which increases during the acquisition
//The number of events stored in each file is fixed to 10000 for this simulation

int main(int argc,char ** argv){

  cout<<"CREATING DATA FILES ..."<<endl;
  char test_fileHead[10];//Buffer for file header
  char test_eventHead[11];//Buffer for evet header
  char test_hitHead[7];//Buffer for data main part
  char test_data[5];//Buffer for data
  char test_fileEnd[10];
  //FIX HERE SOME PARAMETERS
  uint32_t runN = 50;
  uint16_t beg_file = 0xF0F0;
  uint16_t end_file = 0xF1F1;//FILE HEADER
  int file_counter = 0;
  int inv_modules = 0;//N of modules touched in the event
  int fiber_N = 0;//N of fiber for the data
  int fiber_time = 0;//Detected time for the fiber
  int N_files = 50;//Number of files to be created
  int ev_in_file = 10000;//Number of event in each file
  int oct_counter = 0;
  srand((int)time(0));
  int ll = 0;//loop variables
  int loo = 0;

  //Packing file header parameters
  pack_header((unsigned char *)test_fileHead, beg_file, runN, ev_in_file);
  //Creating data file
  fstream output_file;
  char buffer[50]; // make sure it's big enough -> it's for the file name
  snprintf(buffer, sizeof(buffer), "../data/%d-%d.dat", runN, file_counter);
  output_file.open(buffer, std::fstream::out | std::fstream::binary);
  if(!output_file){
    cout<<"Error opening file"<<endl;
  }
  else{
    //if the file is open, write the file header
    output_file.write((char *)test_fileHead, sizeof(test_fileHead));
    oct_counter+=sizeof(test_fileHead);
    output_file.flush();
  }
  for(int files = 0; files<N_files; files++){
    for(ll = 0; ll<ev_in_file; ll++){
      usleep(500);//adapt this delay to the expected acquisition rate to test the monitoring
      inv_modules = rand() % 20;//Extract a number of fibers involved in the event
      //Pack up event header and data main section
      pack_eventHeader((unsigned char *)test_eventHead, (uint16_t)0xABCD, (uint32_t)ll, (uint32_t)ll, (uint16_t)1);
      pack_dataMain((unsigned char *)test_hitHead, (uint8_t)0xEB, (uint8_t)25, (uint32_t)ll,  (uint8_t)7, (uint8_t)inv_modules);
      //Write event on file
      output_file.write((char *) test_eventHead, sizeof(test_eventHead));
      oct_counter+=sizeof(test_eventHead);
      output_file.flush();
      output_file.write((char *) test_hitHead, sizeof(test_hitHead));
      oct_counter+=sizeof(test_hitHead);
      output_file.flush();
      //Creating the data for each involved fiber with random extraction (or fixing the parameters for test)
      for(loo = 0; loo<inv_modules; loo++){
        fiber_N = rand() % 64;
        fiber_time = rand() % 1028;
        //Pack up the data structure for each involved fiber
        pack_data((unsigned char *) test_data, (uint8_t)fiber_N, (uint32_t) fiber_time);
        output_file.write((char *) test_data, sizeof(test_data));
        oct_counter+=sizeof(test_data);
        output_file.flush();
        bzero(test_data, 5);
      }
    bzero(test_eventHead, 11);
    bzero(test_hitHead, 7);
  }
  pack_fileEnd((unsigned char *) test_fileEnd, (uint16_t) end_file, (uint32_t) ev_in_file, (uint32_t)oct_counter);
  output_file.write((char *) test_fileEnd, sizeof(test_fileEnd));
  oct_counter+=sizeof(test_fileEnd);
  output_file.close();
  if(files!=(N_files-1)){cout<<"File "<<file_counter<<" written -> Opening new file!"<<endl;}
  else{cout<<"File "<<file_counter<<" written -> Finish!"<<endl; break;}
  file_counter++;
  snprintf(buffer, sizeof(buffer), "../data/%d-%d.dat", runN, file_counter);
  output_file.open(buffer, std::fstream::out | std::fstream::binary);
  if(!output_file){
    cout<<"Error opening file"<<endl;
  }
  else{
    output_file.write((char *)test_fileHead, sizeof(test_fileHead));
    oct_counter = sizeof(test_fileHead);
    output_file.flush();
  }
}

  return 0;

}
