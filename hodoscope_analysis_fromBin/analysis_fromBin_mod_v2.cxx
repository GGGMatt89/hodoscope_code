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
#include <vector>
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
  int runN = 0;//to be inserted by the user
  std::cout<<"Type the run number : (return to confirm)"<<std::endl;
  std::cin>>runN;
  char path[100] = "/media/oreste/DATA/HODOPIC_Acq/";//"/home/daq/gamhadron/daqGh/data/";//folder where the data are saved
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
  std::cout<<"STARTING analysis for run "<<runN<<" ... "<<std::endl;
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
  int file_to_an = 0;
  int N_event=0;
  int N_event_valid = 0;
  int N_event_valid_coinc = 0;
  int N_event_valid_X =0;
  int N_event_valid_Y =0;

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

  //Initializing vectors for saving time datas and variable to calculate the time difference
  std::vector <int>  hit_time_rec_vec;
  double mean_time=0., sum_time=0.,time_delay=0.;
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
  TH1F *cog_X = new TH1F("position X", "position X", 32, 0.5, 32.5);//center of gravity of interaction in the X plane
  TH1F *cog_Y = new TH1F("position Y", "position Y", 32, 0.5, 32.5);//center of gravity of interaction in the Y plane
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

  TH2F *pos_2D = new TH2F("2d pos", "2D pos", 32, 0.5, 32.5, 32, 0.5, 32.5);//2D map of coincidences
  pos_2D->GetXaxis()->SetTitle("Position X");
  pos_2D->GetYaxis()->SetTitle("Position Y");
  pos_2D->SetMinimum(0);

  TH1F *time = new TH1F("time info", "time info", 150, 40, 100);
  time->SetLineColor(kBlue);
  time->GetXaxis()->SetTitle("Time difference (ns)");
  time->SetMinimum(-200);
  time->GetYaxis()->SetTitle("Entries");
  time->SetMinimum(0);
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
  TPad *map_pad = new TPad("map_pad", "map_pad", 0.5, 0.46, 0.98, 0.98, 0);
  TPad *stats_pad = new TPad("stats_pad", "stats_pad", 0.5, 0.02, 0.98, 0.26, 0);
  TPad *time_pad = new TPad("time_pad", "time_pad", 0.48, 0.26, 0.73, 0.46, 0);
  raw_dt_pad->Draw();
  treat_cluster_pad->Draw();
  treat_positions_pad->Draw();
  map_pad->Draw();
  time_pad->Draw();
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
  time_pad->cd();
  time->Draw();
  

  //----------------------------------------------------END creation of desired plots----------------------------------------------------//

  //Variables for preliminary analysis
  int coinc_counter = 0;
  int molt_X = 0, molt_Y = 0;
  int conv_X=0., conv_Y=0.;
  bool flag_X = false, flag_Y = false;
  double temp_pos_X = 0., temp_pos_Y = 0.;
  int cnt = 0;

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

  count = scandir(path, &files, file_select, alphasort);//how many files in the folder?
  if(count>0){
  std::cout<<"Number of data file found "<<count<<std::endl;
  std::cout<<"Recovered file list : "<<std::endl;
    for(int x = 0; x<count; x++){
      if(strstr(files[x]->d_name, s.str().c_str())){
	      printf("-> %s", files[x]->d_name);
	      std::cout<<std::endl;
	      cnt++;
	    }else{
	      continue;
	    }
    }
  }
  if(cnt>0){
    std::cout<<"We collected "<<cnt<<" files for run "<<runN<<std::endl;
    std::cout<<"How many files you want to analyse ? (Type here and return (-1 for all))"<<std::endl;
    std::cin>>file_to_an;
    if(file_to_an==-1){file_to_an = cnt - 2;}
    for(int fileAn = 0; fileAn < file_to_an; fileAn++){
      if(fp.is_open()){fp.close();}
      sss.str("");
      sss<<path;
      sss<<filemain;
      sss<<fileAn;
      sss<<format;
      fp.open(sss.str().c_str(), std::fstream::in | std::fstream::binary);
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
        }else{
          std::cout<<"PROBLEM IN READING FILE HEADER -> CHECK THE FILE STRUCTURE! Moving to the next file"<<std::endl;
          continue;
        }
        for(int Nevnts = 0; Nevnts<file_beg.Ntot_events; Nevnts++){
        hit_time_rec_vec.clear();
          fp.read((char *)event_header, sizeof(event_header));//reading the event header
          unpack_eventHead((unsigned char *)event_header, event_id);
          //std::cout<<Nevnts<<std::endl;
          if(event_id.event_beg_id != beginning_event){
            std::cout<<"PROBLEM IN READING DATA -> CHECK THE DATA STRUCTURE! Moving to the next file"<<std::endl;
	          std::cout<<event_id.event_beg_id<<std::endl;
	          std::cout<<event_id.event_number<<std::endl;
	          std::cout<<event_id.trigger_number<<std::endl;
	          std::cout<<event_id.hit_in_trig<<std::endl;
	          break;
          }
          fp.read((char *)data_main_structure, sizeof(data_main_structure));//reading the data main part
          unpack_dataMain((unsigned char *)data_main_structure, data_struct);
          if(data_struct.modules_num>0){
            N_event_valid++;
            cluster->Fill(data_struct.modules_num);//filling cluster plot
	          if(data_struct.mode_num==7){
		          for(int w = 0; w<data_struct.modules_num; w++){//reading the data from each involved fiber
	               fp.read((char *)hit_structure_mode0, sizeof(hit_structure_mode0));
	               unpack_data((unsigned char *)hit_structure_mode0, data_fiber);
                 hit_time_rec_vec.push_back(data_fiber.hit_time_rec);
                 sum_time=0.;
                 for(int z=0;z<hit_time_rec_vec.size();z++){ sum_time=sum_time+hit_time_rec_vec.at(z);}
                 mean_time=sum_time/hit_time_rec_vec.size();
                 time_delay = std::abs((mean_time*0.3125) - (event_id.trigger_number*0.625)) ;
		             time->Fill(time_delay);
                 if(data_fiber.N_fiber_rec<32){
                   cog_X->Fill(data_fiber.N_fiber_rec);
    		           conv_X = convert_Xchannel_withCable(data_fiber.N_fiber_rec);
    		           if(conv_X > 0){
    		             position_X->Fill(conv_X);
    		             molt_X++;
    		             flag_X = true;
    		             temp_pos_X += (conv_X);
    		           }
    		           else{std::cout<<"Problem in fiber identification -> skip event "<<std::endl; continue;}
    	           }

    	           else if(data_fiber.N_fiber_rec>=32){
                   cog_Y->Fill(data_fiber.N_fiber_rec-32);
    	             conv_Y = convert_Ychannel_withCable(data_fiber.N_fiber_rec-32);
    		           if(conv_Y>0){
    		              position_Y->Fill(conv_Y);
    		              molt_Y++;
    		              flag_Y = true;
    		              temp_pos_Y += (conv_Y);
                  }
    		          else{std::cout<<"Problem in fiber identification -> skip event "<<std::endl; continue;}
    	           }
    	           bzero(hit_structure_mode0, 5);
    	        }
    	        temp_pos_X /= molt_X;
    	        temp_pos_Y /= molt_Y;
    	        cluster_X->Fill(molt_X);
    	        cluster_Y->Fill(molt_Y);
    	        //cog_X->Fill(temp_pos_X);
    	        //cog_Y->Fill(temp_pos_Y);
                if(flag_X){N_event_valid_X++;}
                if(flag_Y){N_event_valid_Y++;}
    	        if(flag_X && flag_Y){
                    N_event_valid_coinc++;
    	          pos_2D->Fill(temp_pos_X, temp_pos_Y);
    	          coinc_counter++;
    	        }
    	        temp_pos_X = 0.; temp_pos_Y = 0.; molt_X = 0; molt_Y = 0; flag_X = false; flag_Y = false; conv_X = 0; conv_Y = 0;
	          }else if(data_struct.mode_num==8){
	            for(int w = 0; w<data_struct.modules_num; w++){//reading the data from each involved fiber
	              fp.read((char *)hit_structure_mode1, sizeof(hit_structure_mode1));
	              unpack_data1((unsigned char *)hit_structure_mode1, data_fiber1);
	              bzero(hit_structure_mode1, 7);
	            }
	          }else{std::cout<<"Unknown data format - Moving to the next file"<<std::endl;
             break;}
            events_read++;
            N_event++;
          }
        }
        //time->Fit("gaus","V","E1",56,67);
        gSystem->ProcessEvents();
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
	      time_pad->Modified();
	      time_pad->Update();
	      stats_pad->cd();
	      pt->Clear();
	      pt->AddText(Form("#bf{RUN %d - monitoring stats}", runN)); ((TText*)pt->GetListOfLines()->Last())->SetTextColor(kBlue); ((TText*)pt->GetListOfLines()->Last())->SetTextSize(0.14);
	      pt->AddLine(.0,.8,1.,.8);
	      pt->AddText(Form("#bf{-> Analysed events : %d }", events_read));
	      pt->AddText(Form("#bf{-> Detected coincidences : %d -> %.2f per cent of events }", coinc_counter, ((float)(coinc_counter)*100)/(float)(events_read)));
	      pt->AddText(Form("#bf{->  Average beam position : %.2f mm - %.2f mm }", cog_X->GetMean(), cog_Y->GetMean()));
	      pt->Draw();
	      stats_pad->Update();
	      std::cout<<"Event analyzed (with at least one fiber hit) "<<events_read<<std::endl;
        std::cout<<"End of analysis for file "<<sss.str().c_str()<<std::endl;
      }else{
        std::cout<<"Problem opening the data file "<<sss.str().c_str()<<std::endl;
        continue;
      }
    }
  }else{std::cout<<"No files for the selected run - RETURN"<<std::endl;}

  // density of events under time difference peak computing
 
  int last_bin = time->FindLastBinAbove(0,1);
  int first_bin = time->FindFirstBinAbove(0,1);
  int bline_counter;
  double baseline_integral=0.;
  double peak_time_integral=0.;
  for (int bline=last_bin-26; bline<=last_bin-16;bline++){ //Change -26 and -16 value if bline value is under the peak. It can change if the time window is modified. It has been configured for a time window between 56 and 86 ns about.
    baseline_integral = baseline_integral + time->GetBinContent(bline);
    bline_counter=bline_counter+1;
  }
  double baseline = baseline_integral / bline_counter;
  std::cout<<baseline<<std::endl;

  for (int bin_scanner=first_bin; bin_scanner<=last_bin; bin_scanner++){
    if (time->GetBinContent(bin_scanner)>baseline){
    peak_time_integral = peak_time_integral + (time->GetBinContent(bin_scanner) - baseline);
    }
  }
  double density_under_peak_time = (peak_time_integral / time->Integral())*100;
  std::cout<<"Density of events under the time difference peak without baseline : "<<density_under_peak_time<<std::endl;
  
  

  time->SetMinimum(0.001);
  TCanvas *cTimeLog = new TCanvas("time spectrum log scale", "time spectrum log scale", 600, 500);
  cTimeLog->SetFillColor(0); //
  cTimeLog->SetBorderMode(0);	//
  cTimeLog->SetLeftMargin(0.1409396); //
  cTimeLog->SetRightMargin(0.14865772); //
  cTimeLog->SetLogy();
  gStyle->SetOptStat(000); //
  cTimeLog->cd();
  time->Draw("hist");


    std::cout<<"TOT EVENT ANALYZED "<<N_event<<std::endl;
    std::cout<<"Event analyzed (with at least one fiber hit) "<<N_event_valid<<" so "<<(double(N_event_valid)/double(N_event))*100<<std::endl;
    std::cout<<"Event analyzed (with at least one fiber hit in X) "<<N_event_valid_X<<" so "<<(double(N_event_valid_X)/double(N_event))*100<<std::endl;
    std::cout<<"Event analyzed (with at least one fiber hit in Y) "<<N_event_valid_Y<<" so "<<(double(N_event_valid_Y)/double(N_event))*100<<std::endl;
    std::cout<<"Event analyzed (with X/Y coinc) "<<N_event_valid_coinc<<" so "<<(double(N_event_valid_coinc)/double(N_event))*100<<std::endl;


  TFile outroot(Form("./analysis_output/run%d.root", runN), "RECREATE");
  complex_canv->Write("complete_canvas");
  cluster->Write("cluster_raw");
  position_X->Write("raw fibers X");
  position_Y->Write("raw fibers Y");
  cluster_X->Write("cluster_X");
  cluster_Y->Write("cluster_Y");
  cog_X->Write("position X");
  cog_Y->Write("position Y");
  pos_2D->Write("2D map");
  time ->Write("time difference");
  cTimeLog->Write("time diff log scale");

  //outroot.Close();

  return 0;
}
