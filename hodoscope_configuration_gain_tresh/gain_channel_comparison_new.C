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

// Paramètres d'automatisation du nom des fichiers
void gain_channel_comparison(int N_files = 15, bool PM_an = 0){//15 values of gain
int choix;
TString path;
int nASIC;
int nChannel;
double thr = 0.;
double enable_sig = 0.;
double short_sig = 0.;
double first = 0., last = 0.;
double Max_enable=0., Max_short=0., Thr_enable_max=0.,Thr_enable_min=0., Thr_short_min=0.,Thr_short_temp=0.,Thr_enable_temp=0.,Thr_short_max=0., Enable_comp=0.,Max_short_temp=0.,Max_thr_temp=0., Thr_min_ref=0., Thr_max_ref=0.;
int i = 0,j=0, k=0, w=0,x=0, y=0,temp=0,channel_ref=0;
float gain[N_files];
for(int s = 0; s<N_files; s++){
  gain[s] = 0.25 + 0.25*s;
}
cout<<"Tapez le chemin vers le fichier data "<<endl;
cin>>path;
cout<<"Entrez le numéro ASIC "<<endl;
cin>>nASIC;
cout << "Voulez vous etudier une voie en particulier (1) //// toutes les voies (2)"<<endl;
cin >> choix;

int channels = 0;
if(choix == 1){channels = 1;}//analysis of one single channel - choice 1
if(choix == 2){channels = 32;}//analysis of all the channels in one ASIC (32) - choice 2

int TOT_ARRAYS = N_files * channels;
TGraph *enable_plot[TOT_ARRAYS];
TGraph *short_plot[TOT_ARRAYS];
TCanvas *c1[TOT_ARRAYS];
TH1F *frame[TOT_ARRAYS];
TLegend *leg[TOT_ARRAYS];

if (choix ==1){
  cout<<"THIS PART OF THE PROGRAM IS NOT AVAILABLE "<<endl;
  
 
}
/*****************************Program ALL channels ALL gains for 1 ASIC***********************************************/
else if (choix == 2) {
TString filename;
vector <int> thr_vec, enable_vec, short_vec, threshold_vec;

int overlap_temp, ovl,final_thr,final_channel;
ifstream input_file;
  double int_min = Thr_min_ref;
  double int_max = Thr_max_ref;
  int thr_low[32];
  int thr_up[32];
  int thr_low_over[32];
  int thr_up_over[32];
  double gain_set[32];
  int overlap[32];
for(int s = 0; s<32; s++){
    thr_low[s]=0;
    thr_up[s]=0;
    gain_set[s]=0.;
    overlap[s]=0;
  }
bool flag = 0;
double thr = 0.;
double enable_sig = 0.;
double short_sig = 0.;
double first = 0., last = 0.;
double max_abs = 0.;
double max_thr = 0.;
double Max_enable=0., Max_short=0., Thr_enable_max=0.,Thr_enable_min=0., Thr_short_min=0.,Thr_short_temp=0.,Thr_enable_temp=0.,Thr_short_max=0., Short_sig_temp=0.,Max_short_temp=0.,Max_thr_temp=0.;
int i = 0,j=0, k=0, w=0,x=0, y=0,temp=0;
TFile *output = new TFile(path + Form("output_ASIC%i.root", nASIC), "RECREATE");

int ii = 0;
/*Lecture fichier*/
for (int nChannel = 1;nChannel<=32;nChannel++) {
  ii=0;
  for(int l = 0; l<N_files; l++){
    c1[(nChannel-1)*N_files + l] = new TCanvas(Form("canvas overlap ch%d gain%d", nChannel, l), Form("canvas overlap ch%d gain%d", nChannel, l), 600, 500);
    c1[(nChannel-1)*N_files + l]->SetTitle(Form("Reponse ASIC%i Hodoscope ch %02i gain %f - overlap enable/short", nASIC, nChannel, gain[l]));
    //Paramètres graphiques
    enable_plot[(nChannel-1)*N_files + l] = new TGraph();
    enable_plot[(nChannel-1)*N_files + l]->SetMarkerSize(2);
    enable_plot[(nChannel-1)*N_files + l]->SetMarkerStyle(31);
    enable_plot[(nChannel-1)*N_files + l]->SetMarkerColor(8);
    enable_plot[(nChannel-1)*N_files + l]->SetLineColor(8);
    enable_plot[(nChannel-1)*N_files + l]->SetTitle("Enable;Threshold;Counts");
    enable_plot[(nChannel-1)*N_files + l]->GetXaxis()->CenterTitle();
    enable_plot[(nChannel-1)*N_files + l]->GetYaxis()->CenterTitle();
    short_plot[(nChannel-1)*N_files + l]=new TGraph();
    short_plot[(nChannel-1)*N_files + l]->SetMarkerSize(2);
    short_plot[(nChannel-1)*N_files + l]->SetMarkerStyle(31);
    short_plot[(nChannel-1)*N_files + l]->SetMarkerColor(2);
    short_plot[(nChannel-1)*N_files + l]->SetLineColor(2);
    short_plot[(nChannel-1)*N_files + l]->SetTitle("Short pulse;Threshold;Counts");
    short_plot[(nChannel-1)*N_files + l]->GetXaxis()->CenterTitle();
    short_plot[(nChannel-1)*N_files + l]->GetYaxis()->CenterTitle();
    temp =0;
    flag = 0;
    Thr_enable_max=0.,Thr_enable_min=0., Thr_short_temp=0., Thr_short_min=0.,Thr_short_max=0.;
    Max_enable=0.,Max_short=0.;
    max_abs = 0.;
    max_thr = 0.;
    filename = " ";
    filename = Form(path + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,nChannel, gain[l]);
    input_file.open(filename);

    if(input_file.is_open()){
// Remplissage de la courbe
while(input_file>>thr>>first>>enable_sig>>short_sig>>last){
  thr_vec.push_back(thr);
  enable_vec.push_back(enable_sig);
  short_vec.push_back(short_sig);
  enable_plot[(nChannel-1)*N_files + l]->SetPoint(ii, thr, enable_sig);
  short_plot[(nChannel-1)*N_files + l]->SetPoint(ii, thr, short_sig);
  ii++;
  // Recherche de la valeur maximale
  if (enable_sig>Max_enable) {
    Max_enable = enable_sig;
    Thr_enable_temp = thr;
  }
  if (short_sig>Max_short) {
    Max_short = short_sig;
    Thr_short_temp = thr;
  }
}// ferme le while
if(Max_enable > Max_short){max_abs = Max_enable;}else{max_abs = Max_short;}
max_thr = *max_element(thr_vec.begin(), thr_vec.end());
input_file.close();
// Recherche du seuil minimum
k=1;
Max_short_temp=0;
Short_sig_temp=0;


for(UInt_t h = 0; h < short_vec.size();h++){
      if(short_vec.at(h)>Max_short_temp){
        Max_short_temp = short_vec.at(h);
		    Short_sig_temp=thr_vec.at(h);
        
      }
      else if(Max_short_temp>10 && short_vec.at(h)<=Max_short_temp/20 && flag == 0){
        Max_thr_temp = thr_vec.at(h);
        flag = 1;
      }
      else if(Max_short_temp>10 && short_vec.at(h)>=Max_short_temp/20 && flag == 1){
        Thr_short_max = thr_vec.at(h)+100;
        break;
      }
}
/**************************Reading and comparison with key values for each channel and gain********************************/


// Recherche du seuil 10% de la valeur max pour Enable et Short_pulse
i=0;
j=0;
k=0;
w=0;
x=0;
y=0;

for(i = 0; i<enable_vec.size(); i++){
  if (enable_vec.at(i) >= 0.1*Max_enable && thr_vec.at(i)>=Thr_enable_temp && w==0) {
    Thr_enable_max=thr_vec.at(i);
	   w=1;
    break;
  }
}

for(k = 0; k<short_vec.size(); k++){
  if (short_vec.at(k) < Max_short && thr_vec.at(k)<=Thr_short_temp && y==0) {
    Thr_short_min=thr_vec.at(k);
	y=1;
    break;
  }
}

// Affichage seuil optimal



leg[(nChannel-1)*N_files + l] = new TLegend (.7,.4,.9,.6);
leg[(nChannel-1)*N_files + l]->AddEntry(enable_plot[(nChannel-1)*N_files + l],"Enable", "l");
leg[(nChannel-1)*N_files + l]->AddEntry(short_plot[(nChannel-1)*N_files + l],"Short pulse", "l");
c1[(nChannel-1)*N_files + l]->cd();
frame[(nChannel-1)*N_files + l] = c1[(nChannel-1)*N_files + l]->DrawFrame(0,0,max_thr+200,max_abs+0.1*max_abs);
frame[(nChannel-1)*N_files + l]->GetXaxis()->SetTitle("THR");
frame[(nChannel-1)*N_files + l]->GetYaxis()->SetTitle("Entries");
c1[(nChannel-1)*N_files + l]->Update();
enable_plot[(nChannel-1)*N_files + l]->Draw("PL"); //Plot Line
short_plot[(nChannel-1)*N_files + l]->Draw("PL same");
leg[(nChannel-1)*N_files + l]->Draw();


/************************************/

if (gain[l]==0.25 && Thr_short_max>Thr_min_ref){
  Thr_min_ref=Thr_short_max;
  Thr_max_ref=Max_thr_temp;
  channel_ref=nChannel;
  thr_low[channel_ref-1] = Thr_min_ref;
  thr_up[channel_ref-1] = Thr_max_ref;
  thr_low_over[channel_ref-1]=Thr_min_ref;
  thr_up_over[channel_ref-1]=Thr_max_ref;
  gain_set[channel_ref-1] = 0.25;
  overlap[channel_ref-1] = (Thr_max_ref-Thr_min_ref)/100 + 1;
}

}
else{
cout<<"Fichier pas trouvé -> verifier le nom du fichier"<<endl;
if(c1[(nChannel-1)*N_files + l]){c1[(nChannel-1)*N_files + l]->Close(); gSystem->ProcessEvents(); delete c1[(nChannel-1)*N_files + l]; c1[(nChannel-1)*N_files + l] = 0;}
continue;
}

thr_vec.clear();
enable_vec.clear();
short_vec.clear();


input_file.close();
output->cd();
c1[(nChannel-1)*N_files + l]->Write(Form("canvas channel %d gain %f - overlap", nChannel, gain[l]));
if(c1[(nChannel-1)*N_files + l]){c1[(nChannel-1)*N_files + l]->Close(); gSystem->ProcessEvents(); delete c1[(nChannel-1)*N_files + l]; c1[(nChannel-1)*N_files + l] = 0;}
} // ferme l++
} // ferme channel ++
int_min=Thr_min_ref;
int_max=Thr_max_ref;
nChannel=1;
for (int nChannel = 1;nChannel<=32;nChannel++) {
  if(!PM_an){
    if(nChannel != channel_ref){find_gain(path, nASIC, int_min, int_max, nChannel, thr_low[nChannel-1], thr_up[nChannel-1], thr_low_over[nChannel-1], thr_up_over[nChannel-1], gain_set[nChannel-1], overlap[nChannel-1]);
    cout<<"The identified gain for channel "<<nChannel<<" is "<<gain_set[nChannel-1]<<" with an overlap of "<<overlap[nChannel-1]<<" thr values!"<<endl;}}
  else{find_thr_PM(path, nASIC, nChannel);}
} // ferme channel ++

int min_overlap = 20;
int min_over_thrMin;
int min_over_thrMax;
int overlap_all[32];
int min_over_ch;
double gain_set_temp = 0.;

for(int x = 0; x < 32; x++){
  overlap_all[x] = 0;
  if(overlap[x]<=min_overlap){
    min_overlap = overlap[x];
    if(gain_set[x] > gain_set_temp){
      gain_set_temp = gain_set[x];
      min_over_thrMin = thr_low_over[x];
      min_over_thrMax = thr_up_over[x];
      min_over_ch = x + 1;
    }
  }
}

cout<<"Minimum overlap with reference values for channel "<<min_over_ch<<" with overlap of "<<min_overlap<<" in range "<<min_over_thrMin<<" - "<<min_over_thrMax<<endl;

int value_over[min_overlap];
for(int xc = 0; xc < min_overlap; xc++){
  value_over[xc] = 0;
}

for(int z = 0; z < 32; z++){
  for(int ss = 0; ss < min_overlap; ss++){
    for(int xs = 0; xs < overlap[z]; xs++){
      if((min_over_thrMin + (100*ss)) == (thr_low_over[z] + (xs*100))){
        overlap_all[z]++;
        value_over[ss]++;
        break;
      }
    }
  }
}
cout<< channel_ref << "  seuil bas : "<< Thr_min_ref<<"  seuil haut : "<<Thr_max_ref<<endl;
cout<<"Verified overlap for all channels "<<endl;
for(int za = 0; za <32; za++){
  cout<<"CH "<<za + 1<<" found overlap "<<overlap_all[za]<<endl;
}

int thr_to_find = 0;
int max_vote_temp = 0;
for(int fgh = 0; fgh<min_overlap; fgh++){
  if(max_vote_temp<value_over[fgh]){
    thr_to_find = min_over_thrMin + (100*fgh);
  }
}


for(int h = 0; h < min_overlap; h++){
  cout<<value_over[h]<<endl;
  if(value_over[h]==32){
    cout<<"Possible thr value = "<<min_over_thrMin + 100*h<<endl;
  }
}

bool OK = 0;
int count = 0;
for (int jh = 0; jh< 32; jh++){
  OK = 0;
  count = 0;
  if(overlap_all[jh] == 0){
    cout<<"Checking channel "<<jh<<endl;
    while((!OK)&&(count<14)){
      OK = check_gain(path, nASIC, int_min, int_max, jh+1, thr_to_find, thr_low[jh], thr_up[jh], thr_low_over[jh], thr_up_over[jh], gain_set[jh], overlap[jh]);
      if(!OK){gain_set[jh] = gain_set[jh]+0.25;}
      count++;
    }
  }
}

ofstream out_txt(path + Form("results_gain-thr_ch_asic%i.txt", nASIC));
for(int loop = 0; loop<32; loop++){
	if(nASIC==2){out_txt<<loop+33<<"\t"<<gain_set[loop]<<"\t"<<thr_low[loop]<<"\t"<<thr_up[loop]<<"\t"<<thr_low_over[loop]<<"\t"<<thr_up_over[loop]<<"\t"<<overlap[loop]<<endl;}
	if(nASIC==1){out_txt<<loop+1<<"\t"<<gain_set[loop]<<"\t"<<thr_low[loop]<<"\t"<<thr_up[loop]<<"\t"<<thr_low_over[loop]<<"\t"<<thr_up_over[loop]<<"\t"<<overlap[loop]<<endl;}
}
int cnl=1;
ovl=overlap_temp;
for (cnl=1;cnl<=32;cnl++) {
    if (overlap[cnl-1]<ovl) {
        ovl=overlap[cnl-1];
        final_channel=cnl;
    }
}
cout<<thr_low[final_channel-1]<<endl;
i=0;

threshold_vec.clear();
output->Close();

}// ferme le if choix
return;

} // ferme fonction main
