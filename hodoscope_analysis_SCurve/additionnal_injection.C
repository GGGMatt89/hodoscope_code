#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <TGraph.h>
#include <TString.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TLegend.h>
#include <TFile.h>
#include <TMath.h>
#include <TSystem.h>
#include <TPad.h>

using namespace std;

// Reading files and saving data in vectors function
void txt_to_vec_max(ifstream& file, vector <int> &enable_vec, vector <int> &short_vec, vector <int> &threshold_vec){
  double enable_sig=0., short_sig=0., thr=0., first=0., last=0.;
  while(file>>thr>>first>>enable_sig>>short_sig>>last){
    threshold_vec.push_back(thr);
    enable_vec.push_back(enable_sig);
    short_vec.push_back(short_sig);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
void additionnal_injection (){
//Variables declaration
TString path, path_injection, path_noise;
TString filename_input;
TString filename_injection;
int nASIC, charge_value,charge_old,nRUN,nRun_mes,nChannel;
float gain,gain_add;
bool add_noise;
ifstream input_file;
TFile *input_file2;

vector <int> enable_vec_injected, short_vec_injected, threshold_vec;			//built vectors from S_Curve obtained by current injection

cout << "Entrez le numero ASIC" <<endl;
cin >> nASIC;
cout << "Entrez le numero du RUN  de la mesure expérimentale : format 00XXX"<<endl;
cin >> nRun_mes;
cout << "Entrez le numero de channel etudie"<<endl;
cin >> nChannel;
cout << "Entrez la charge injectee lors de la 1ere analyse (pC)"<<endl;
cin >> charge_old;
cout << "Entrez le gain lors de la mesure (V)"<<endl;
cin >> gain;
cout << "Entrez la charge pour la nouvelle injection(pC)"<<endl;
cin >> charge_value;
cout << "Entrez la valeur du gain additionnel (V)"<<endl;
cin >> gain_add;

path=Form("/media/oreste/DATA/S_CURVE/NICE/RUN%05i/",nRun_mes);
path_injection=Form("/media/oreste/DATA/S_CURVE/%ipC/",charge_value);
path_noise=Form("/media/oreste/DATA/S_CURVE/Noise_With_PM_Plugged_ASIC%i/",nASIC);

TFile *additionnal_gain = new TFile(path+Form("additional_gain%.02f_ASIC%i_%ipC.root",gain_add,nASIC,charge_value),"RECREATE");
//path_injection=Form("/Users/fontana/Dropbox/ASIC_Characterization_Hodoscope/RUN_S_CURVE/%ipC/",charge_value);
TGraph *enable_injection; //additionnal injected charge
TGraph *short_injection;
TGraph *enable_injection_read; //injected charge at same gain than the measurement
TGraph *short_injection_read;
TGraph *enable_measurement_read; // measurement
TGraph *short_measurement_read;
TGraph *enable_noise;
TGraph *short_noise;
TH1F *frame_enable;
TH1F *frame_short;

//Graphical settings of additional graphs
    enable_injection = new TGraph();
    enable_injection->SetMarkerSize(1);
    enable_injection->SetMarkerStyle(22);
    enable_injection->SetMarkerColor(kOrange-5);
    enable_injection->SetLineColor(kOrange-5);


    short_injection = new TGraph();
    short_injection->SetMarkerSize(1);
    short_injection->SetMarkerStyle(22);
    short_injection->SetMarkerColor(kOrange-5);
    short_injection->SetLineColor(kOrange-5);

    enable_noise = new TGraph();
    enable_noise->SetMarkerSize(1);
    enable_noise->SetMarkerStyle(22);
    enable_noise->SetMarkerColor(kCyan+1);
    enable_noise->SetLineColor(kCyan+1);

    short_noise = new TGraph();
    short_noise->SetMarkerSize(1);
    short_noise->SetMarkerStyle(22);
    short_noise->SetMarkerColor(kCyan+1);
    short_noise->SetLineColor(kCyan+1);


    Double_t Max_ordonnee_enable = 0.;
    Double_t Max_ordonnee_short = 0.;
    Double_t Max_abscisse = 0.;

//Reading injection file for additionnal plot
filename_injection = Form(path_injection + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,nChannel, gain_add);
    input_file.open(filename_injection);
			if (input_file.is_open()) {
			cout << filename_injection<<endl;
			txt_to_vec_max(input_file, enable_vec_injected, short_vec_injected, threshold_vec);
				for(int i = 0; i<enable_vec_injected.size(); i++){
				enable_injection->SetPoint(i, threshold_vec.at(i), enable_vec_injected.at(i));
				short_injection->SetPoint(i, threshold_vec.at(i), short_vec_injected.at(i));
				}

      Max_abscisse = *max_element(threshold_vec.begin(), threshold_vec.end());
      Max_ordonnee_enable = *max_element(enable_vec_injected.begin(), enable_vec_injected.end());
      Max_ordonnee_short = *max_element(short_vec_injected.begin(), short_vec_injected.end());
      enable_vec_injected.clear();
      short_vec_injected.clear();
      threshold_vec.clear();
      input_file.close();
                        }

//Reading noise file for additionnal plot
filename_injection = Form(path_noise + "Asic%i_CH(%02i)_Gain(%.2f).txt",nASIC ,nChannel, gain_add);
    input_file.open(filename_injection);
			if (input_file.is_open()) {
			cout << filename_injection<<endl;
			txt_to_vec_max(input_file, enable_vec_injected, short_vec_injected, threshold_vec);
				for(int i = 0; i<enable_vec_injected.size(); i++){
				enable_noise->SetPoint(i, threshold_vec.at(i), enable_vec_injected.at(i));
				short_noise->SetPoint(i, threshold_vec.at(i), short_vec_injected.at(i));
				}
      enable_vec_injected.clear();
      short_vec_injected.clear();
      threshold_vec.clear();
      input_file.close();

//Openning analysis_output file
TCanvas *c = new TCanvas (Form("Canvas_analysis_ch%i_add_gain_%.02f_V_&_add_charge_%i_PC",nChannel,gain_add,charge_value), Form("Canvas_analysis_ch%i_add_gain_%.02f_V_&_add_charge_%i_PC",nChannel,gain_add,charge_value),1200,1000);
c->Divide(1,2);
TLegend *leg_enable = new TLegend (.7,.4,.9,.6);
TLegend *leg_short = new TLegend (.7,.4,.9,.6);
filename_input=Form(path+"analysis_ASIC%i_%ipC.root",nASIC,charge_old);
input_file2 = new TFile(filename_input, "READ");
//input_file2.open(filename_input);
Int_t x_max_enable = 0.;
Int_t x_max_short = 0.;
Double_t y_max_enable = 0.;
Double_t y_max_short = 0.;

Double_t tempX = 0, tempY = 0;
int ll = 0;
cout << filename_input<<endl;
//Additionnal charge injection curves
		if (!input_file2->IsZombie()) {
			cout << "File is open"<<endl;
      input_file2->GetObject(Form("en_inj_graph_ch%i_gain%.02f",nChannel, gain), enable_injection_read);
      for(ll = 0; ll< enable_injection_read->GetN(); ll++){
        enable_injection_read->GetPoint(ll, tempX, tempY);
        if(y_max_enable<tempY){
          y_max_enable = tempY;
        }
      }
      cout<<"y_max_enable "<<y_max_enable<<endl;
      enable_injection_read->GetPoint(0, tempX, tempY);
      cout<<"temp x "<<tempX<<endl;
      x_max_enable = tempX;
      cout<<" x_max_enable "<<x_max_enable<<endl;
      input_file2->GetObject(Form("short_inj_graph_ch%i_gain%.02f",nChannel, gain), short_injection_read);

      for(ll = 0; ll< short_injection_read->GetN(); ll++){
        short_injection_read->GetPoint(ll, tempX, tempY);
        if(y_max_short<tempY){
          y_max_short = tempY;
        }
      }
      cout<<"y_max_short "<<y_max_short<<endl;
      short_injection_read->GetPoint(0, tempX, tempY);
        cout<<"temp x "<<tempX<<endl;
      x_max_short = tempX;
      cout<<" x_max_short "<<x_max_short<<endl;
      input_file2->GetObject(Form("en_meas_graph_ch%i_gain%.02f",nChannel, gain), enable_measurement_read);
            cout << Max_abscisse << "  "<<Max_ordonnee_short<<" "<<endl;
      for(ll = 0; ll< enable_measurement_read->GetN(); ll++){
        enable_measurement_read->GetPoint(ll, tempX, tempY);
        if(y_max_enable<tempY){
          y_max_enable = tempY;
        }
      }
      enable_measurement_read->GetPoint(0, tempX, tempY);
      if(x_max_enable < tempX){
        x_max_enable = tempX;
      }
      input_file2->GetObject(Form("short_meas_graph_ch%i_gain%.02f",nChannel, gain), short_measurement_read);
      for(ll = 0; ll< short_measurement_read->GetN(); ll++){
        short_measurement_read->GetPoint(ll, tempX, tempY);
        if(y_max_short<tempY){
          y_max_short = tempY;
        }
      }
      short_measurement_read->GetPoint(0, tempX, tempY);
      if(x_max_short < tempX){
        x_max_short = tempX;
      }

      if(x_max_enable>Max_abscisse){
        Max_abscisse = x_max_enable;
      }
      if(x_max_short>Max_abscisse){
        Max_abscisse = x_max_short;
      }
      if(y_max_enable>Max_ordonnee_enable){
        Max_ordonnee_enable = y_max_enable;
      }
      if(y_max_short>Max_ordonnee_short){
        Max_ordonnee_short = y_max_short;
      }
      cout << "Voulez vous ajouter les courbes de bruit sous conditions qu elles soient disponibles ?  Oui = 1 // Non = 0"<<endl;
      cin>>add_noise;
      c->cd(1);
      frame_enable = gPad->DrawFrame(0,0,Max_abscisse+200,750/*Max_ordonnee_enable+0.1*Max_ordonnee_enable*/);
      frame_enable->GetXaxis()->SetTitle("Threshold");
      frame_enable->GetYaxis()->SetTitle("Entries");
      enable_injection_read->Draw("PL");
      enable_measurement_read->Draw("PL same");
      enable_injection->Draw("PL same");
      if (add_noise) {
      enable_noise->Draw("PL same");
      leg_enable->AddEntry(enable_noise, Form("Enable - Noise %.02f V",gain), "pl");
      }
      leg_enable->AddEntry(enable_injection_read, Form("Enable - injection de charge - %ipC - %.02f gain",charge_old,gain), "pl");
      leg_enable->AddEntry(enable_measurement_read, "Enable - beam measurement", "pl");
      leg_enable->AddEntry(enable_injection, Form("Enable - injection de charge - %ipC - %.02f gain",charge_value,gain_add), "pl");
      leg_enable->Draw();

      c-> Modified();
      c-> Update();
      
      c->cd(2);
      frame_short = gPad->DrawFrame(0,0,Max_abscisse+200,750/*Max_ordonnee_short+0.1*Max_ordonnee_short*/);
      frame_short->GetXaxis()->SetTitle("Threshold");
      frame_short->GetYaxis()->SetTitle("Entries");
      short_injection_read->Draw("PL");
      cout << gPad->GetUymax() <<"  "<<short_measurement_read->GetMaximum()<<endl; //
      //for (int i=0;i<short_measurement_read->GetN();i++) { short_measurement_read->GetY()[i] *=  gPad->GetUymax()/short_measurement_read->GetMaximum(); } // fonctionne mais GetMaximum negatif
      short_measurement_read->Draw("PL same");
      short_injection->Draw("PL same");
      if (add_noise) {
      short_noise->Draw("PL same");
      leg_short->AddEntry(short_noise, Form("Short - Noise %.02f V",gain), "pl");
      }
      leg_short->AddEntry(short_injection_read, Form("Short - injection de charge - %ipC - %.02f V",charge_old,gain), "pl");
      leg_short->AddEntry(short_measurement_read, "Short - beam measurement", "pl");
      leg_short->AddEntry(short_injection, Form("Short - injection de charge - %ipC - %.02f V",charge_value,gain_add), "pl");
      leg_short->Draw();
      //TGaxis *measurement_axis = new TGaxis(Max_abscisse+200,0,Max_abscisse+200,gPad->GetUymax(),1,Max_ordonnee_short+0.1*Max_ordonnee_short,510,"+G"); // fonctionne mais à adapter
      measurement_axis->SetLineColor(kRed);
      measurement_axis->SetLabelColor(kRed);
      measurement_axis->Draw();


      c-> Modified();
      c-> Update();
      
      
      additionnal_gain-> cd();
      c->Write(Form("canvas_short_with_add_gain_%0.2f",gain_add));
      additionnal_gain-> Close();
    }


}
return;
}
