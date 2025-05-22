void plot_diag_comp(TString input_file_1, TString file_1_label, TString input_file_2, TString file_2_label, TString output){

    TFile *fin_1 = new TFile(input_file_1, "open");
    TDirectoryFile* trace_1 = (TDirectoryFile*)fin_1->Get("Trace");
    TDirectoryFile* autor_1 = (TDirectoryFile*)fin_1->Get("Auto_corr");


    TFile *fin_2 = new TFile(input_file_2, "open");
    TDirectoryFile* trace_2 = (TDirectoryFile*)fin_2->Get("Trace");
    TDirectoryFile* autor_2 = (TDirectoryFile*)fin_2->Get("Auto_corr");


    if(trace_1->GetNkeys() != autor_1->GetNkeys())
    {
        std::cerr<<"You haven't given me the same number of trace_1s+autor_1relations I'm leaving!"<<std::endl;
        throw;
    }

    TKey* key;
    TIter next(trace_1->GetListOfKeys());

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
        std::string trace_1_syst(key->GetName());
        std::string auto_syst_1=trace_1_syst.substr(0, trace_1_syst.length()-5)+"Lag";
        std::cout<<"Plotting trace_1s and autor_1relations for "<<trace_1_syst<<std::endl;

        TH1D* trace_1hist=(TH1D*)trace_1->Get(trace_1_syst.c_str());
        TH1D* autohist_1=(TH1D*)autor_1->Get(auto_syst_1.c_str());

        TH1D* trace_2hist=(TH1D*)trace_2->Get(trace_1_syst.c_str());
        TH1D* autohist_2=(TH1D*)autor_2->Get(auto_syst_1.c_str());

        autohist_1->SetNdivisions(5);
        autohist_2->SetNdivisions(5);

        trace_2hist->SetNdivisions(5);

        int rebinFactor = 1000; // Adjust this value as needed
        trace_1hist->Rebin(rebinFactor);
        trace_1hist->Scale(1.0/float(rebinFactor));
        trace_2hist->Rebin(rebinFactor); 
        trace_2hist->Scale(1.0/float(rebinFactor));

        trace_1hist->SetLineColorAlpha(kAzure-2, 0.5);
        autohist_1->SetLineColor(kAzure-2);

        trace_2hist->SetLineColorAlpha(kOrange+7, 0.5);
        autohist_2->SetLineColor(kOrange+7);


        TLegend* leg = new TLegend(0.6, 0.6, 0.9, 0.9);
        leg->AddEntry(autohist_1, file_1_label);
        leg->AddEntry(autohist_2, file_2_label);

        p2->cd();
        THStack* s1 = new THStack();
        s1->Add(autohist_1);
        s1->Add(autohist_2);
        s1->Draw("nostack");

        s1->GetXaxis()->SetNdivisions(5);
        s1->GetXaxis()->SetTitle("Lag");
        s1->GetYaxis()->SetTitle("Autocorrelation");

        s1->SetTitle(trace_1_syst.c_str());
        p2->Update();
        leg->Draw();

        p1->cd();

        THStack* s = new THStack();

        // Get rid of annoying fit line!
        auto fit_1 = trace_1hist->GetFunction("Fitter");
        trace_1hist->GetListOfFunctions()->Remove(fit_1);

        auto fit_2 = trace_2hist->GetFunction("Fitter");
        trace_2hist->GetListOfFunctions()->Remove(fit_2);

        s->Add(trace_1hist);
        s->Add(trace_2hist);
        s->Draw("nostack");

        s->SetTitle(trace_1_syst.c_str());
        s->GetXaxis()->SetNdivisions(5);
        s->GetXaxis()->SetTitle("Step");
        s->GetYaxis()->SetTitle("Param Variation");
        s->SetTitle(trace_1_syst.c_str());
        p1->Update();

        c->Print(output+".pdf");

        delete s;
        delete s1;
    }
    c->Print(output+".pdf]");
}
