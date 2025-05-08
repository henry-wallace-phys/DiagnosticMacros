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
//Plot autocorrelations and traces and save to pdf

void plot_diag(TString diagfile, TString output)
{
  TFile *fin = new TFile(diagfile, "open");
  TDirectoryFile* trace = (TDirectoryFile*)fin->Get("Trace");
  TDirectoryFile* autocor = (TDirectoryFile*)fin->Get("Auto_corr");

  if(trace->GetNkeys() != autocor->GetNkeys())
    {
      std::cerr<<"You haven't given me the same number of traces+autocorrelations I'm leaving!"<<std::endl;
      throw;
    }
  
  TKey* key;
  TIter next(trace->GetListOfKeys());
  
  std::cout<<"loaded in files"<<std::endl;
  
  TCanvas* c = new TCanvas("c", "c", 1200, 600);
  TPad* p1 = new TPad("p1", "p1", 0.0, 0.0, 0.5, 1.0);
  TPad* p2 = new TPad("p2", "p2", 0.5, 0.0, 1.0, 1.0);
  c->Draw();
  c->cd();
  c->SetGrid();
  p1->Draw();
  p2->Draw();
  c->Print(output+".pdf[");

  
  std::cout<<"Beginning loop!"<<std::endl;
  while( (key = (TKey*) next()))
  { 
      std::string trace_syst(key->GetName());
      std::string auto_syst=trace_syst.substr(0, trace_syst.length()-5)+"Lag";
      std::cout<<"Plotting traces and autocorrelations for "<<trace_syst<<std::endl;
      
      TH1D* tracehist=(TH1D*)trace->Get(trace_syst.c_str());
      TH1D* autohist=(TH1D*)autocor->Get(auto_syst.c_str());
      autohist->SetNdivisions(5);

      p1->cd();
      autohist->Draw();
      p2->cd();
      tracehist->Draw();
      c->Print(output+".pdf");
    }
  c->Print(output+".pdf]");
}

