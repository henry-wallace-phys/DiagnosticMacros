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


TH1D* get_hist_from_ttree(TTree* tree, TString branch_name, TString tree_label, int nbins, double min_val, double max_val){
  
  TH1D* h = new TH1D(tree_label, "trace_comp:step"+branch_name, nbins, min_val, max_val);
  tree->Draw(branch_name+">>"+tree_label, "step>100000");
  //  h->SetFillColorAlpha(kWhite, 0.0);
  h->Scale(1/static_cast<double>(tree->GetEntries()));
  return h;
}
// Simple script to compare traces
void ComparePosteriors(TString file_1_name, TString file_1_lab, TString file_2_name, TString file_2_lab, TString output="posteriors_comp.pdf"){
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
    
    auto file_1_hist = get_hist_from_ttree(file_1_posteriors, branch_name, file_1_lab, nbins, min_val, max_val);
    auto file_2_hist = get_hist_from_ttree(file_2_posteriors, branch_name, file_2_lab, nbins, min_val, max_val);

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
