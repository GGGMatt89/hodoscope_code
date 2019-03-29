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
#include <stdio.h>
#include <mutex>
//ROOT classes
#include "TLatex.h"
#include "TROOT.h"
#include "TObject.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TFile.h"
#include "TApplication.h"
#include "TRint.h"
#include "TAxis.h"
#include "TTimer.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TGWindow.h"
#include "TGClient.h"
#include "TPaveText.h"
//custom classes
#include "functions.h"

extern int alphasort();

int file_select(const struct dirent *entry)//selection of files to be included in the return of the folder scan
{//ignore all files with the following features
  if((strcmp(entry->d_name, ".directory") == 0) ||(strcmp(entry->d_name, ".DS_Store") == 0) || (strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0) || strstr(entry->d_name,"tmp")!=NULL||strstr(entry->d_name,"pulse")!=NULL||strstr(entry->d_name,"T")!=NULL){
    return 0;
  }  else {
    return 1;
  }
}

int main (int argc,char ** argv)
{
  //Variables to be modified run by run and for changing file name structure
  //HERE YOU CAN SET THE VARIABLES TO ADAPT THIS APPLICATION TO YOUR SYSTEM
  //YOU JUST NEED TO FIX THE PATH TO THE FOLDER WHERE THE DATA WILL BE SAVED (variable path), AND THE FILENAME WHO COMES BEFORE THE FILE NUMBER (variable filemain)
  //I SUPPOSED A FILENAME LIKE file_name_N.dat WITH INCREASING N STARTING FROM 1, but this can be easily changed
  int runN = 0;//to be inserted by the user
  int update = 0;//to be inserted by the user
  std::cout<<"Type the run number : (return to confirm)"<<std::endl;
  std::cin>>runN;
  std::cout<<"Choose the frequency for updating plots : (number of events to analyse before updating)"<<std::endl;
  std::cin>>update;
  char path[100] = "../data/";//"/home/daq/gamhadron/daqGh/data/";//folder where the data are saved
  char filemain[100];//main part of the file name - only numbering stuff after that
  int xx = sprintf(filemain, "%d-", runN);
  char format[30] =".dat";//data format -> the acquisition software creates .dat files
  //-----------------------------------------------------------------------------------------------------------------------------//
  //---------------------------------------------DON'T MODIFY SINCE NOW ON (if not needed :-) )----------------------------------//
  //-----------------------------------------------------------------------------------------------------------------------------//

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
  std::cout<<"STARTING monitor for run "<<runN<<" ... "<<std::endl;
  //Creating data structure to read the data from file
  FILEHEADER file_beg;//file header - it contains 0xF0F0 (16 bit), the run number (32 bit), the total number of events recorder in the file (32 bit)
  FILEEND file_end;//file footer - it contains 0xF1F1 (16 bit), the total number of events recorder in the file (32 bit), the total number of octets recorded in the file (32 bit)
  EVENTHEADER event_id;//event header (one per event) - it contains 0xABCD (16 bit), the event number (32 bit), the trigger number (24 bit), the number of hits recorded in this trigger (16 bit)
  DATAMAIN data_struct;//main structure of data (one per event) - it contains 0x00EB (16 bit), the front end number (8 bit), the trigger number (24 bit), the mode number (8 bit), the number of involved detector modules (8 bit - example number of touched fibers for the hodoscope)
  hit_fiber_receive data_fiber;//data for each hit mode 0 - optimal (one per involved detector module) - it contains the number of the hit module (8 bit) and the recorded time for the interaction (32 bit)
  hit_fiber_receive1 data_fiber1;//data for each hit mode 1 - test (one per involved detector module) - it contains the number of the hit module (8 bit), the recorded time for the interaction (32 bit) and the recorded charge (16 bit)
  //Create ID to check what I read
  uint16_t beginning_file_id = 0xF0F0;
  uint16_t end_file_id = 0xF1F1;
  uint16_t beginning_event = 0xABCD;
  int N_ev_toRead = 0;//Set the number of events you want to read from each file for monitoring purpose - depending on the speed and accuracy you need
  int length;
  int N_event=0;
  int count = 0;
  int count1 = 0;
  int files_read = 0;
  int events_read = 0;
  int wait_loop = 0;//counter for waiting loops
  int time_to_wait = 50;//how many waiting loops you accept before stopping the program
  //Initializing char buffers for reading from file
  char init_file[10];//file header
  char end_file[8];//file end - for debugging
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
  //Creating the desired plots
  TH1F *cluster = new TH1F("cluster size", "cluster size", 20, 0.5, 20.5);//raw cluster size - total number of involved fibers per event
  TH1F *position_X = new TH1F("fiber X", "fiber X", 32, 0.5, 32.5);//number of fibers touched in the X plane
  TH1F *position_Y = new TH1F("fiber Y", "fiber Y", 32, 0.5, 32.5);//number of fibers touched in the Y plane
  cluster->SetLineColor(kBlue);
  cluster->GetXaxis()->SetTitle("N involved fibers");
  cluster->GetYaxis()->SetTitle("Entries");
  cluster->SetMinimum(0);
  position_X->SetLineColor(kRed);
  position_X->GetXaxis()->SetTitle("Fiber N X plane");
  position_X->GetYaxis()->SetTitle("Entries");
  position_X->SetMinimum(0);
  position_Y->SetLineColor(kGreen);
  position_Y->GetXaxis()->SetTitle("Fiber N Y plane");
  position_Y->GetYaxis()->SetTitle("Entries");
  position_Y->SetMinimum(0);

  TH1F *cluster_X = new TH1F("cluster size X", "cluster size X", 21, -0.5, 20.5);//cluster size for the X plane
  TH1F *cluster_Y = new TH1F("cluster size Y", "cluster size Y", 21, -0.5, 20.5);//cluster sieze for the Y plane
  TH1F *cog_X = new TH1F("position X", "position X", 96, 0.5, 32.5);//center of gravity of interaction in the X plane
  TH1F *cog_Y = new TH1F("position Y", "position Y", 96, 0.5, 32.5);//center of gravity of interaction in the Y plane
  cluster_X->SetLineColor(kRed);
  cluster_X->GetXaxis()->SetTitle("N involved fibers X");
  cluster_X->GetYaxis()->SetTitle("Entries");
  cluster_X->SetMinimum(0);
  cluster_Y->SetLineColor(kGreen);
  cluster_Y->GetXaxis()->SetTitle("N involved fibers Y");
  cluster_Y->GetYaxis()->SetTitle("Entries");
  cluster_Y->SetMinimum(0);
  cog_X->SetLineColor(kRed);
  cog_X->GetXaxis()->SetTitle("Position X plane");
  cog_X->GetYaxis()->SetTitle("Entries");
  cog_X->SetMinimum(0);
  cog_Y->SetLineColor(kGreen);
  cog_Y->GetXaxis()->SetTitle("Position Y plane");
  cog_Y->GetYaxis()->SetTitle("Entries");
  cog_Y->SetMinimum(0);

  TH2F *pos_2D = new TH2F("2d pos", "2D pos", 96, 0.5, 32.5, 96, 0.5, 32.5);//2D map of coincidences
  pos_2D->GetXaxis()->SetTitle("Position X");
  pos_2D->GetYaxis()->SetTitle("Position Y");
  pos_2D->SetMinimum(0);

  //Preparing stats to be shown
  TPaveText *pt;
  //Preparing canvas
  TCanvas *complex_canv = new TCanvas("all data", "all data", 1200, 1000);
  complex_canv->SetFillColor(0); //
  complex_canv->SetBorderMode(0);	//
  complex_canv->SetLeftMargin(0.1409396); //
  complex_canv->SetRightMargin(0.14865772); //
  gStyle->SetOptStat(000); //
  //Divide canvas to fill with all the desired plots
  TPad *raw_dt_pad = new TPad("raw_dt_pad", "raw_dt_pad", 0.03, 0.69, 0.48, 0.98, 0);
  TPad *treat_cluster_pad = new TPad("treat_cluster_pad", "treat_cluster_pad", 0.03, 0.36, 0.48, 0.67, 0);
  TPad *treat_positions_pad = new TPad("treat_positions_pad", "treat_positions_pad", 0.03, 0.03, 0.48, 0.34, 0);
  TPad *map_pad = new TPad("map_pad", "map_pad", 0.5, 0.36, 0.98, 0.98, 0);
  TPad *stats_pad = new TPad("stats_pad", "stats_pad", 0.5, 0.03, 0.98, 0.31, 0);
  raw_dt_pad->Draw();
  treat_cluster_pad->Draw();
  treat_positions_pad->Draw();
  map_pad->Draw();
  stats_pad->Draw();

  raw_dt_pad->Divide(3, 1);
  raw_dt_pad->cd(1);
  cluster->Draw();
  raw_dt_pad->cd(2);
  position_X->Draw();
  raw_dt_pad->cd(3);
  position_Y->Draw();
  treat_cluster_pad->Divide(2, 1);
  treat_cluster_pad->cd(1);
  cluster_X->Draw();
  treat_cluster_pad->cd(2);
  cluster_Y->Draw();
  treat_positions_pad->Divide(2,1);
  treat_positions_pad->cd(1);
  cog_X->Draw();
  treat_positions_pad->cd(2);
  cog_Y->Draw();
  map_pad->cd();
  pos_2D->Draw("colz");
  stats_pad->cd();
  pt = new TPaveText(.05,.1,.95,.9);
  pt->AddText(Form("#bf{RUN %d - monitoring stats}", runN));
  pt->Draw();
  //----------------------------------------------------END creation of desired plots----------------------------------------------------//

  //Variables for preliminary analysis
  int coinc_counter = 0;
  int molt_X = 0, molt_Y = 0;
  bool flag_X = false, flag_Y = false;
  double temp_pos_X = 0., temp_pos_Y = 0.;
  int cnt = 0;
  int cnt1 = 0;

  //Creating string to check that the selected files refer to the present run - all other data files are ignored
  std::stringstream  s;
  s<<runN;
  s<<"-";
  //Creating structures and variables to search in the data folder for files
  struct dirent **files;
  std::stringstream  ss;
  struct dirent **files2;
  std::stringstream  sss;
  std::fstream fp;
  bool cond = true;
  bool flag = 0;

  while(cond){//while to wait the presence of at least one complete file to start the monitoring
    std::cout<<"Waiting for data files to begin analysis ..."<<std::endl;
    count = scandir(path, &files, file_select, alphasort);//how many files in the folder?
    if(count>0 && flag == 0){
      std::cout<<"Number of data file found "<<count<<std::endl;
      std::cout<<"Recovered file list"<<std::endl;
      for(int x = 0; x<count; x++){
        if(strstr(files[x]->d_name, s.str().c_str())){
	        printf("-> %s", files[x]->d_name);
	        std::cout<<std::endl;
	        cnt++;
	      }else{
	        continue;
	      }
      }
      if(cnt>0)flag = 1;
    }
    //if(count>1){
    if(cnt>1){
      sss.str("");
      sss<<path;
      sss<<filemain;
      sss<<0;
      sss<<format;
      fp.open(sss.str().c_str(), std::fstream::in | std::fstream::binary);
      if(fp){
        std::cout<<"FILE open"<<std::endl;
        std::cout<<"Analysing file "<<sss.str().c_str()<<" ..... "<<std::endl;
        //std::cout<<"Checking if the file is well closed .... ";
        /*fp.seekg(8, std::ios::end);
        fp.read((char *)end_file, sizeof(end_file));
        unpack_fileend((unsigned char *)end_file, file_end);
        std::cout<<std::hex<<file_end.file_end_id<<std::endl;
        if(file_end.file_end_id == 0xF1F1){
          std::cout<<"   -> OK! "<<std::endl;
          //std::cout<<" The file contains "<<file_end.evnt_number<<" events "<<std::endl;
        }else{std::cout<<"The file is probably not yet closed! - wait! "<<std::endl; usleep(1000000); continue;}
        */
        fp.seekg(0, std::ios::end);
        length = fp.tellg();
        std::cout<<"The file size is "<<length<<std::endl;
        fp.seekg(0, std::ios::beg);//put the read pointer at the beginning of the file
        //Reading the file header
        fp.read((char *)init_file, sizeof(init_file));//reading the file header
        unpack_fileHead((unsigned char *)init_file, file_beg);
        if(file_beg.file_beg_id==beginning_file_id){
          std::cout<<"Beginning of new file read well "<<std::endl;
          std::cout<<"This is run "<<file_beg.run_number<<" and this file contains "<<file_beg.Ntot_events<<std::endl;
          N_ev_toRead = (int)file_beg.Ntot_events/5;
        }
      }else{std::cout<<"Problem opening the data file "<<std::endl;}
      cond = !cond;
      std::cout<<"First loop over"<<std::endl;
      std::cout<<"Reading pointer in position "<<fp.tellg()<<std::endl;
      break;//go out from this first while loop and start loopin on events
    }else{
      std::cout<<"Still waiting "<<std::endl;
      flag = 0;
      usleep(1000000);
    }
  }

  while(true){//Main while on events and files after first while used for inizialization
    std::cout<<N_event<<"\t"<<N_ev_toRead<<std::endl;
    std::cout<<"Reading pointer in position "<<fp.tellg()<<std::endl;
    if(N_event>=N_ev_toRead || fp.eof()){//if we are reading more events than requested or we are at the end of the file
      //we close the old file and search for new ones
      sss.str("");
      sss << path;
      count1 = scandir(path, &files, file_select, alphasort);//how many files in the folder?
      cnt1=0;
      for(int w = 0; w<count1; w++){
        if(strstr(files[w]->d_name, s.str().c_str())){
	  cnt1++;
	}else{
	  continue;
	}
      }
      if(cnt1>=cnt && cnt>(files_read+1)){//we found new files that have been closed for sure
        std::cout<<"There are "<< cnt1 <<" files, and still "<<cnt1-files_read<<" files to be analyzed! GO! "<<std::endl;
        N_event=0;
        fp.close();//closing the old file
        files_read++;//increment the number of files already analysed
        std::cout<<"End of analysis for file "<<files_read<<std::endl;
        sss<<filemain;
        sss<<files_read;
        sss<<format;
        fp.open(sss.str().c_str(), std::fstream::in | std::fstream::binary);//opening the new file
        if(fp){
          std::cout<<"FILE open"<<std::endl;
          std::cout<<"Analysing file "<<sss.str().c_str()<<" ..... "<<std::endl;
          fp.seekg(0, std::ios::end);
          length = fp.tellg();
          std::cout<<"The file size is "<<length<<std::endl;
          fp.seekg(0, std::ios::beg);//put the read pointer at the beginning of the file
          //Reading the file header
          fp.read((char *)init_file, sizeof(init_file));//reading the file header
          unpack_fileHead((unsigned char *)init_file, file_beg);
          if(file_beg.file_beg_id==beginning_file_id){
            std::cout<<"Beginning of new file read well "<<std::endl;
            std::cout<<"This is run "<<file_beg.run_number<<" and this file contains "<<file_beg.Ntot_events<<std::endl;
            N_ev_toRead = (int)file_beg.Ntot_events/5;

	        }
        }
      }
      //else if(count1>(files_read+2) && count<=(files_read+1)){
      else if(cnt1>(files_read+2) && cnt<=(files_read+1)){
        std::cout<<"There are "<< cnt1<<" files "<<std::endl;
        N_event=0;
        fp.close();//closing the old file
        files_read++;//increment the number of files already analysed
        std::cout<<"End of analysis for file "<<files_read<<std::endl;
	cnt = cnt1-1;
        sss<<filemain;
        sss<<files_read;
        sss<<format;
        fp.open(sss.str().c_str(), std::fstream::in | std::fstream::binary);//opening the new file
        if(fp){
          std::cout<<"FILE open"<<std::endl;
          std::cout<<"Analysing file "<<sss.str().c_str()<<" ..... "<<std::endl;
          fp.seekg(0, std::ios::end);
          length = fp.tellg();
          std::cout<<"The file size is "<<length<<std::endl;
          fp.seekg(0, std::ios::beg);//put the read pointer at the beginning of the file
          //Reading the file header
          fp.read((char *)init_file, sizeof(init_file));//reading the file header
          unpack_fileHead((unsigned char *)init_file, file_beg);
          if(file_beg.file_beg_id==beginning_file_id){
            std::cout<<"Beginning of new file read well "<<std::endl;
            std::cout<<"This is run "<<file_beg.run_number<<" and this file contains "<<file_beg.Ntot_events<<std::endl;
            N_ev_toRead = (int)file_beg.Ntot_events/5;
          }
        }

      }else if(wait_loop > time_to_wait){std::cout<<"The acquisition probably stopped! EXIT! "<<std::endl; complex_canv->SaveAs(Form("monitoring_run%d.png", runN)); return 0;
      }else{std::cout<<"Waiting for data files to analyse ..."<<std::endl; usleep(1000000); wait_loop++; continue;}
    }
    if(fp){
      fp.read((char *)event_header, sizeof(event_header));//reading the event header
      unpack_eventHead((unsigned char *)event_header, event_id);
      if(event_id.event_beg_id != beginning_event){
        std::cout<<"PROBLEM IN READING DATA -> CHECK THE DATA STRUCTURE! "<<std::endl;
	std::cout<<sizeof(event_header)<<std::endl;
        std::cout<<event_id.event_beg_id<<std::endl;
	std::cout<<event_id.event_number<<std::endl;
	std::cout<<event_id.trigger_number<<std::endl;
	std::cout<<event_id.hit_in_trig<<std::endl;
	return 0;
      }
      fp.read((char *)data_main_structure, sizeof(data_main_structure));//reading the data main part
      unpack_dataMain((unsigned char *)data_main_structure, data_struct);
      if(data_struct.modules_num>0){

	      cluster->Fill(data_struct.modules_num);//filling cluster plot
	      if(data_struct.mode_num==7){
	        for(int w = 0; w<data_struct.modules_num; w++){//reading the data from each involved fiber
	          fp.read((char *)hit_structure_mode0, sizeof(hit_structure_mode0));
	          unpack_data((unsigned char *)hit_structure_mode0, data_fiber);
	          if(data_fiber.N_fiber_rec<32){
		    position_X->Fill(data_fiber.N_fiber_rec + 1);
	            molt_X++;
	            flag_X = true;
		    temp_pos_X += (data_fiber.N_fiber_rec + 1);
	          }
	          else if(data_fiber.N_fiber_rec>=32){
		    position_Y->Fill(data_fiber.N_fiber_rec - 31 );
	            molt_Y++;
	            flag_Y = true;
		    temp_pos_Y += (data_fiber.N_fiber_rec - 31);
	          }
	          bzero(hit_structure_mode0, 5);
	        }
	        temp_pos_X /= molt_X;
	        temp_pos_Y /= molt_Y;
	        cluster_X->Fill(molt_X);
	        cluster_Y->Fill(molt_Y);
	        cog_X->Fill(temp_pos_X);
	        cog_Y->Fill(temp_pos_Y);
	        if(flag_X && flag_Y){
	          pos_2D->Fill(temp_pos_X, temp_pos_Y);
	          coinc_counter++;
	        }
	        temp_pos_X = 0.; temp_pos_Y = 0.; molt_X = 0; molt_Y = 0; flag_X = false; flag_Y = false;
	      }else if(data_struct.mode_num==8){
	        for(int w = 0; w<data_struct.modules_num; w++){//reading the data from each involved fiber
	          fp.read((char *)hit_structure_mode1, sizeof(hit_structure_mode1));
	          unpack_data1((unsigned char *)hit_structure_mode1, data_fiber1);
	          bzero(hit_structure_mode1, 7);
	        }
	      }else{std::cout<<"Unknown data format - EXIT"<<std::endl; return 0;}

	      N_event++;
	      events_read++;
	      gSystem->ProcessEvents();
	      gSystem->Sleep(5);
	      if(events_read%update == 0){
	        for(int pd2 = 1; pd2 < 4; pd2++){
	          complex_canv->cd();
	          raw_dt_pad->cd(pd2);
	          gPad->Modified();
	          gPad->Update();
	          if(pd2<3){
	            treat_cluster_pad->cd(pd2);
	            gPad->Modified();
	            gPad->Update();
	            treat_positions_pad->cd(pd2);
	            gPad->Modified();
	            gPad->Update();
	          }
	        }
	        complex_canv->cd();
	        map_pad->Modified();
	        map_pad->Update();
	        stats_pad->cd();
	        pt->Clear();
	        pt->AddText(Form("#bf{RUN %d - monitoring stats}", runN)); ((TText*)pt->GetListOfLines()->Last())->SetTextColor(kBlue); ((TText*)pt->GetListOfLines()->Last())->SetTextSize(0.18);
	        pt->AddLine(.0,.8,1.,.8);
	        pt->AddText(Form("#bf{-> Analysed events : %d }", events_read));
	        pt->AddText(Form("#bf{-> Detected coincidences : %d -> %.2f per cent of events }", coinc_counter, ((float)(coinc_counter)*100)/(float)(events_read)));
	        pt->AddText(Form("#bf{->  Average beam position : %.2f mm - %.2f mm }", cog_X->GetMean(), cog_Y->GetMean()));
	        pt->Draw();
	        stats_pad->Update();
	        std::cout<<"Event analyzed "<<events_read<<std::endl;
	      }
      }else{continue;}
    }
  }//end of while loop for reading events and files

  complex_canv->Connect("Closed()", "TApplication", gApplication, "Terminate()");
  theApp.Run();

  return 0;
}
