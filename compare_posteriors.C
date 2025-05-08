#include <iostream>
#include <string>
#include <vector>

#include <TTree.h>
#include <TFile.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TGraph.h>
#include <THStack.h>
#include <TMath.h>


TTree* get_posterior_tree(TString file_name){
  TFile* file = TFile::Open(file_name);
  if(file->IsZombie()){
    std::cerr<<"ERROR::"<<file_name<<" not valid"<<std::endl;
    throw;
  }
  TTree* posteriors = (TTree*)file->Get("posteriors");
  if(!posteriors){
    std::cerr<<"ERROR::"<<file_name<<" does not contain posterior tree"<<std::endl;
  }
  return posteriors;
}


TH1D* fix_hist(TTree* tree, TString branch_name, TString tree_label, double min_val, double max_val, int nbins, int step_cut=100000){
  TH1D* h = new TH1D(tree_label, branch_name, nbins, min_val, max_val);

  double val;
  int step;
  tree->SetBranchAddress("step", &step);
  tree->SetBranchAddress(branch_name, &val);

  for(int i=0; i<tree->GetEntries(); i++){
    tree->GetEntry(i);

    if(step<step_cut){
      continue;
    }

    if(val<min_val){
      val = 2*max_val + val;
    }
    if(val>max_val){
      val = 2*min_val + val;
    }
    if(step>100000){
      h->Fill(val);
    }

  }
  tree->ResetBranchAddresses();
  return h;
}

TH1D* get_hist_from_ttree(TTree* tree, TString branch_name, TString tree_label, int nbins, double min_val, double max_val, bool nova=false){
  
  TH1D* h;

  TString cut = "(step>100000)";
  if(nova){
    // Apply nova cuts
    // cut += "*(TMath::Cos(delta_cp))";
  }

  
  if(nova && branch_name.Contains("delta_cp")){
    // Fix the nova hist
    h = fix_hist(tree, branch_name, tree_label, -1*TMath::Pi(), TMath::Pi(), nbins);
  }

  else{
    h = new TH1D(tree_label, "trace_comp:step"+branch_name, nbins, min_val, max_val);
    tree->Draw(branch_name+">>"+tree_label, cut, "goff");
  }

  //  h->SetFillColorAlpha(kWhite, 0.0);
  h->Scale(1/static_cast<double>(tree->GetEntries()));
  return h;
}

// Simple script to compare traces
void compare_posteriors(TString file_1_name, TString file_1_lab, bool file_1_nova, TString file_2_name, TString file_2_lab, bool file_2_nova, TString output="posteriors_comp.pdf"){
  auto file_1_posteriors = get_posterior_tree(file_1_name);
  auto file_2_posteriors = get_posterior_tree(file_2_name);

  TCanvas* c = new TCanvas("c", "c");
  c->Draw();

  c->cd();

  c->Print(output+"[");
  auto branches = file_1_posteriors->GetListOfBranches();

  for(int i=0; i<branches->GetEntries(); i++){
    TString branch_name = static_cast<TBranch*>(branches->At(i))->GetName();

    std::cout<<"Plotting "<<branch_name<<std::endl;

    
    auto max_val = std::max(file_1_posteriors->GetMaximum(branch_name),
			    file_2_posteriors->GetMaximum(branch_name));

    
    auto min_val = std::min(file_1_posteriors->GetMinimum(branch_name),
			    file_2_posteriors->GetMinimum(branch_name));

    int nbins = 50;
    
    auto file_1_hist = get_hist_from_ttree(file_1_posteriors, branch_name, file_1_lab, nbins, min_val, max_val, file_1_nova);
    auto file_2_hist = get_hist_from_ttree(file_2_posteriors, branch_name, file_2_lab, nbins, min_val, max_val, file_2_nova);

    TLegend* leg = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg->AddEntry(file_1_hist, file_1_lab);
    leg->AddEntry(file_2_hist, file_2_lab);
    
    file_1_hist->SetLineColor(kOrange+7);
    file_2_hist->SetLineColor(kAzure-3);

    THStack* s = new THStack("LogLComparison", "Trace Comparison "+branch_name);
    
    s->Add(file_1_hist);
    s->Add(file_2_hist);

    s->Draw("hist nostack");
    s->GetXaxis()->SetTitle("Step");
    s->GetYaxis()->SetTitle(branch_name);
    c->Update();
    leg->Draw();
    c->Print(output);

    c->Clear();
    delete file_1_hist;
    delete file_2_hist;
    delete s;
      
    
  }
  c->Print(output+"]");
}
