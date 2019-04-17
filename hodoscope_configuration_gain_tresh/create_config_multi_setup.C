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


void create_config_multi_setup(){

int charge_value;
cout << "Quelle est la charge utilisee pour la configuration ? (pC)" <<endl;
cin >> charge_value;

  TString path1 = Form("/media/oreste/DATA/S_CURVE/%ipC/",charge_value);
  TString path2 = Form("/media/oreste/DATA/S_CURVE/%ipC/",charge_value);
  TString path_config = Form("/media/oreste/DATA/S_CURVE/%ipC/",charge_value);
  ofstream file_config1(path_config + Form("conf_%ipC_max.txt",charge_value));
  ofstream file_config2(path_config + Form("conf_%ipC_midmax.txt",charge_value));
  ofstream file_config3(path_config + Form("conf_%ipC_midmin.txt",charge_value));

  int ch = 0;
  double gain1 = 0., gain2 = 0., gain3=0.;
  int thr_low = 0;
  int thr_up = 0;
  int thr_low_over = 0;
  int thr_up_over = 0;
  int over_end = 0;
  string temp1 = " ", temp2= " ", temp3= " ";
  ifstream file_in1(path1 + "Setup_gain-thr_ch_asic1.txt"); // change name of files if needed

  while(file_in1>>ch>>gain1>>gain2>>gain3){
    temp1 = Form("CH(%02i),Gain(%.2f),MASK(0),CSA(0)",ch , gain1);
    temp2 = Form("CH(%02i),Gain(%.2f),MASK(0),CSA(0)",ch , gain2);
    temp3 = Form("CH(%02i),Gain(%.2f),MASK(0),CSA(0)",ch , gain3);
    file_config1<<temp1<<endl;
    file_config2<<temp2<<endl;
    file_config3<<temp3<<endl;
    temp1 = " ", temp2= " ", temp3= " ";
  }
file_in1.close();

  ifstream file_in2(path2 + "Setup_gain-thr_ch_asic2.txt"); // change name of files if needed
  while(file_in2>>ch>>gain1>>gain2>>gain3){
    temp1 = Form("CH(%02i),Gain(%.2f),MASK(0),CSA(0)",ch , gain1);
    temp2 = Form("CH(%02i),Gain(%.2f),MASK(0),CSA(0)",ch , gain2);
    temp3 = Form("CH(%02i),Gain(%.2f),MASK(0),CSA(0)",ch , gain3);
    file_config1<<temp1<<endl;
    file_config2<<temp2<<endl;
    file_config3<<temp3<<endl;
temp1 = " ", temp2= " ", temp3= " ";
  }
file_in2.close();

  file_config1<<"Tresh_ASIC1(800)"<<endl;
  file_config1<<"Tresh_ASIC2(2200)"<<endl;
  file_config1.close();
  file_config2<<"Tresh_ASIC1(700)"<<endl;
  file_config2<<"Tresh_ASIC2(1900)"<<endl;
  file_config2.close();
  file_config3<<"Tresh_ASIC1(500)"<<endl;
  file_config3<<"Tresh_ASIC2(1000)"<<endl;
  file_config3.close();
}
