#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <TGraph.h>
#include <TH1F.h>
#include <TString.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TFile.h>
#include <TMath.h>
#include <TSystem.h>
#include "functions.h"

using namespace std;


void create_config(){
  TString path1 = "/media/oreste/DATA/S_CURVE/2pC/";
  TString path2 = "/media/oreste/DATA/S_CURVE/2pC/";
  TString path_config = "/media/oreste/DATA/S_CURVE/2pC/";
  ofstream file_config1(path_config + "conf_2pC_.txt");

  int ch = 0;
  double gain = 0.;
  int thr_low = 0;
  int thr_up = 0;
  int thr_low_over = 0;
  int thr_up_over = 0;
  int over_end = 0;
  string temp = " ";
  ifstream file_in1(path1 + "Setup_gain-thr_ch_asic1.txt");

  while(file_in1>>ch>>gain>>thr_low>>thr_up>>thr_low_over>>thr_up_over>>over_end){
    temp = Form("CH(%02i),Gain(%.2f),MASK(0),CSA(0)",ch , gain+0.25);
    file_config<<temp<<endl;
    temp = " ";
  }
file_in1.close();

  ifstream file_in2(path2 + "Setup_gain-thr_ch_asic2.txt");
  while(file_in2>>ch>>gain>>thr_low>>thr_up>>thr_low_over>>thr_up_over>>over_end){
    temp = Form("CH(%02i),Gain(%.2f),MASK(0),CSA(0)",ch , gain+0.25);
    file_config<<temp<<endl;
    temp = " ";
  }
file_in2.close();

  file_config<<"Tresh_ASIC1(400)"<<endl;
  file_config<<"Tresh_ASIC2(900)"<<endl;
  file_config.close();
}
