#include "TH1D.h"
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TStyle.h"
#include "TAxis.h"
#include "TGaxis.h"
#include "TGraph.h"
#include "TPad.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TMath.h"
#include <iostream>
#include <string>
//Plot autocorrelations and traces and save to png

void plot_average_ac(TString diagfile, TString output)
{
  TFile *fin = new TFile(diagfile, "open");

  if(fin->IsZombie()){
   std::cerr<<"Error couldnt open "<<diagfile<<std::endl;
   throw;
  }

  TDirectoryFile* autocor = (TDirectoryFile*)fin->Get("Auto_corr");

  if(!autocor){
    std::cerr<<"Error couldn't find Auto_Corr in "<<diagfile<<std::endl;
    throw;
  }


  TKey* key;
  TIter next(autocor->GetListOfKeys());
  
  std::cout<<"loaded in files"<<std::endl;
  
  TCanvas* c = new TCanvas("c", "c", 1200, 600);
  c->Draw();
  c->cd();
  c->SetGrid();

  std::cout<<"Beginning loop!"<<std::endl;

  gStyle->SetPalette(kRainbow);

  TH1D* auto_hist=nullptr;

  int npars = 0;

  THStack* s = new THStack();

  TLegend* leg = new TLegend(0.7, 0.7, 0.9, 0.9);
  while( (key = (TKey*) next()))
  { 
      std::string auto_syst(key->GetName());
      std::cout<<"Getting AC info"<<auto_syst<<std::endl;

      TH1D* plotting_hist = dynamic_cast<TH1D*>(autocor->Get(auto_syst.c_str())->Clone());

      if(!auto_hist){
        leg->AddEntry(plotting_hist, "Individual Auto-correlations");
        auto_hist= dynamic_cast<TH1D*>(plotting_hist->Clone());
      }
      else{
        auto_hist->Add(plotting_hist);
      }

      plotting_hist->SetLineColorAlpha(kAzure-2, 0.1);
      c->cd();
      s->Add(plotting_hist);

      npars++;
  }

  auto_hist->Scale(1.0/float(npars));
  //auto_hist->SetFillColorAlpha(kOrange+7, 0.9);
  auto_hist->SetLineColor(kBlack);
  leg->AddEntry(auto_hist, "Average Autocorrelation");
  s->Draw("nostack");
  TH1D* auto_hist_error = dynamic_cast<TH1D*>(auto_hist->Clone());
  auto_hist_error->SetFillColorAlpha(kOrange+7, 0.5);
  //auto_hist->Draw("E4 same");
  auto_hist_error->Draw("E4 same");
  auto_hist->Draw("l hist same");
  leg->AddEntry(auto_hist_error, "1#sigma Error");
  s->SetTitle("Average Autocorrelation Across All Parameters");
  s->GetXaxis()->SetTitle("Lag");
  s->GetYaxis()->SetTitle("Autocorrelation");
  leg->Draw();
  c->Print(output+".png");
}

