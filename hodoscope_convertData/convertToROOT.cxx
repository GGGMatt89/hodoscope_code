//c++ classes
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include <cstring>
#include <dirent.h>//class used to manipulate and traverse directories
#include <sys/stat.h>
#include <stdio.h>
#include <mutex>
//ROOT classes
#include "TROOT.h"
#include "TFile.h"
#include "TApplication.h"
#include "TRint.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TTree.h"
//custom classes
#include "functions.h"

extern int alphasort();

int file_select(const struct dirent *entry)
{
  if((strcmp(entry->d_name, ".DS_Store") == 0) || (strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0) ||strstr(entry->d_name,"tmp")!=NULL||strstr(entry->d_name,"pulse")!=NULL||strstr(entry->d_name,"T")!=NULL)
    return 0;
  else
    return 1;
}

int main (int argc,char ** argv)
{
  //Variables to be modified run by run and for changing file name structure
  //HERE YOU CAN SET THE VARIABLES TO ADAPT THIS APPLICATION TO YOUR SYSTEM
  //YOU JUST NEED TO FIX THE PATH TO THE FOLDER WHERE THE DATA WILL BE SAVED, AND THE FILENAME WHO COMES BEFORE THE FILE NUMBER
  //I SUPPOSED A FILENAME LIKE file_name_N.dat WITH INCREASING N STARTING FROM 1, but this can be easily changed
  int runN = 0;//to be inserted by the user
  std::cout<<"Type the run number : (return to confirm)"<<std::endl;
  std::cin>>runN;
  char path[100] = "../data/";//"/home/daq/gamhadron/daqGh/data/";//folder where the data are saved
  char path_out[100] = "../data_ROOT/";
  char filemain[100];//main part of the file name - only numbering stuff after that
  int xx = sprintf(filemain, "%d-", runN);
  char format[30] =".dat";//data format -> the acquisition software creates .dat files
  //---------------------------------------------------DON'T MODIFY FROM NOW ON----------------------------------//

  //ROOT STYLING
  gROOT->SetStyle("Plain");
  gROOT->ForceStyle();
  gStyle -> SetStatW(0.28);
  gStyle -> SetStatH(0.13);
  gStyle -> SetStatColor(0);
  gStyle -> SetStatX(0.87);
  gStyle -> SetStatY(0.85);
  gStyle -> SetStatFont(0);
  gStyle -> SetOptStat(111);
  gStyle->SetPalette(1);
  //-------------------------------//

  std::cout<<"CONVERTING DATA TO ROOT FORMAT ..."<<std::endl;
  //Creating data structure to read the data from file
  FILEHEADER file_beg;
  FILEEND file_end;
  EVENTHEADER event_id;
  DATAMAIN data_struct;
  hit_fiber_receive data_fiber;
  hit_fiber_receive1 data_fiber1;
  std::vector <Int_t> N_fiber;
  std::vector <Int_t> time_data;

  //Create ID to check what I read
  uint16_t beginning_file_id = 0xF0F0;
  uint16_t end_file_id = 0xF1F1;
  uint16_t beginning_event = 0xABCD;
  int N_ev_toRead = 0;//Set the number of events you want to read from each file for monitoring purpose
  int length;
  int N_event=0;
  int count = 0;
  int files_read = 0;
  int cnt = 0;
  //Initializing char buffers for reading from file
  char init_file[8];//file header
  char end_file[8];
  uint8_t event_header[11];//event header
  uint8_t data_main_structure[7];//data main structure
  uint8_t hit_structure_mode0[5];//just time data - optimal mode
  uint8_t hit_structure_mode1[7];//time and charge data - test mode
  int position = 0; //to check the pointer position in the file and avoid misreading
  //Starting ROOT in the c++ program
  TApplication theApp("App", &argc, argv);
    if (gROOT->IsBatch()) {
      fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);
      return 1;
    }

  std::stringstream fold;
  fold<<path_out;
  fold<<"run";
  fold<<runN;
  fold<<"/";
  //creating folder for converted data
  const int dir_err = mkdir(fold.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (-1 == dir_err)
  {
     printf("Error creating directory!n");
     exit(1);
   }
  std::stringstream  s;
  s<<runN;
  s<<"-";

  //Creating structures and variables to search in the data folder for files
  struct dirent **files;
  std::stringstream  ss;
  std::fstream fp;

  count = scandir(path, &files, file_select, alphasort);//how many files in the folder?
  if(count>0){
    std::cout<<"Number of data file found "<<count<<std::endl;
    std::cout<<"Recovered file list"<<std::endl;
    for(int x = 0; x<count; x++){
      if(strstr(files[x]->d_name, s.str().c_str())){
	      printf("-> %s\n", files[x]->d_name);
	      cnt++;
	    }else{
	      continue;
	    }
    }
  }
  TFile *outroot;
  outroot = new TFile(Form("%soutput_run%d.root", fold.str().c_str(), runN), "RECREATE");
  outroot->cd();
  //Preparing storage objects and files
  TTree *tree = new TTree(Form("hodoscope data run %d", runN), Form("hodoscope data run %d", runN));
  tree->Branch("fe_number",&data_struct.fe_number,"fe_number/I");
  tree->Branch("trigger_number",&data_struct.trigger_number,"trigger_number/I");
  tree->Branch("modules_num",&data_struct.modules_num, "modules_num/I");
  tree->Branch("N_fiber", &N_fiber);
  tree->Branch("time_data", &time_data);
  tree->SetMaxTreeSize(1000000000LL);//max file size set to 1Gb -> change here to modify
  for(int fls = 0; fls<cnt; fls++){
    if(fp.is_open()){fp.close();files_read++;}
    ss.str("");
    ss<<path;
    ss<<filemain;
    ss<<fls;
    ss<<format;
    fp.open(ss.str().c_str(), std::fstream::in | std::fstream::binary);
    //std::cout<<"Converting file "<<ss.str().c_str()<<" ..... "<<std::endl;
    if(fp){
      std::cout<<"FILE open"<<std::endl;
      std::cout<<"Converting file "<<ss.str().c_str()<<" ..... "<<std::endl;
      fp.seekg(0, std::ios::end);
      length = fp.tellg();
      std::cout<<"The file size is "<<length<<std::endl;
      fp.seekg(0, std::ios::beg);//put the read pointer at the beginning of the file
      //Reading the file header
      fp.read((char *)init_file, sizeof(init_file));//reading the file header
      unpack_fileHead((unsigned char *)init_file, file_beg);
      if(file_beg.file_beg_id==beginning_file_id){
        std::cout<<"Beginning of new file -> OK! "<<std::endl;
        std::cout<<"This is run "<<file_beg.run_number<<" and this file contains "<<file_beg.Ntot_events<<" events"<<std::endl;
      }
      for(int evt = 0; evt<file_beg.Ntot_events; evt++){
      //for(int evt = 0; evt<20; evt++){
        fp.read((char *)event_header, sizeof(event_header));//reading the event header
        unpack_eventHead((unsigned char *)event_header, event_id);
        if(event_id.event_beg_id != beginning_event){
          std::cout<<"PROBLEM IN READING DATA -> CHECK THE DATA STRUCTURE!"<<std::endl;
          return 0;
        }else{
          fp.read((char *)data_main_structure, sizeof(data_main_structure));//reading the data main part
          unpack_dataMain((unsigned char *)data_main_structure, data_struct);
          if(data_struct.mode_num==7){
            for(int w = 0; w<data_struct.modules_num; w++){//reading the data from each involved fiber
              fp.read((char *)hit_structure_mode0, sizeof(hit_structure_mode0));
              unpack_data((unsigned char *)hit_structure_mode0, data_fiber);
              N_fiber.push_back((Int_t)data_fiber.N_fiber_rec);
              time_data.push_back((Int_t)data_fiber.hit_time_rec);
              bzero(hit_structure_mode0, 5);
            }
          }else if(data_struct.mode_num==8){
            for(int w = 0; w<data_struct.modules_num; w++){//reading the data from each involved fiber
              fp.read((char *)hit_structure_mode1, sizeof(hit_structure_mode1));
              unpack_data1((unsigned char *)hit_structure_mode1, data_fiber1);
              bzero(hit_structure_mode1, 7);
            }
          }else{std::cout<<"Unknown data format: check the run details! Not able to decode data "<<std::endl; return 0;}
        }
        tree->Fill();
        N_fiber.clear();
        time_data.clear();
        N_event++;
        if(N_event % 1000 == 0){std::cout<<"Event "<<N_event<<" read! Still working!"<<std::endl;}
      }
    }
    else{
      std::cout<<"Problem opening the data file "<<std::endl; continue;
    }
      fp.close();
  }
  tree->Write();
  outroot = tree->GetCurrentFile();
  outroot->Close();
  std::cout<<"End of conversion for run "<<runN<<std::endl;
  std::cout<<"N of converted events "<<N_event<<std::endl;
  std::cout<<"EXIT! Bye! "<<std::endl;

  return 0;
}
