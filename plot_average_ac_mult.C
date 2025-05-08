
TH1D* get_average_ac_for_file(TString diagfile){
    /*
    Get average autocorrelation histogram from a file
    */

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
    std::cout<<"Beginning loop!"<<std::endl;


    TH1D* auto_hist=nullptr;

    int npars = 0;
    while( (key = (TKey*) next()))
    {
        std::string auto_syst(key->GetName());
        TH1D* plotting_hist = dynamic_cast<TH1D*>(autocor->Get(auto_syst.c_str())->Clone());

        if(!auto_hist){
            auto_hist= dynamic_cast<TH1D*>(plotting_hist->Clone());
        }
        else{
            auto_hist->Add(plotting_hist);
        }
        delete plotting_hist;
        npars++;

    }

    auto_hist->Scale(1.0/float(npars));
//auto_hist->SetFillColorAlpha(kOrange+7, 0.9);
    return auto_hist;
}

void plot_average_ac_mult(TString file_1, TString label_1, TString file_2, TString label_2, TString output_name){
    TH1D* plot_1 = get_average_ac_for_file(file_1);
    TH1D* plot_1_errors = (TH1D*)plot_1->Clone();
    TH1D* plot_2 = get_average_ac_for_file(file_2);
    TH1D* plot_2_errors = (TH1D*)plot_2->Clone();

    TCanvas* c = new TCanvas("c", "c", 1200, 600);
    c->Draw();
    c->cd();
    c->SetGrid();

    plot_1->SetLineColor(kAzure-2);
    plot_1_errors->SetTitle("Average Autocorrelation Comparison");
    plot_1_errors->GetXaxis()->SetTitle("Lag");
    plot_1_errors->GetYaxis()->SetTitle("Autocorrelation");

    plot_1_errors->SetFillColorAlpha(kAzure-2, 0.4);

    plot_2->SetLineColor(kOrange+7);
    plot_2_errors->SetFillColorAlpha(kOrange+7, 0.4);

    TLegend* leg = new TLegend(0.6, 0.6, 0.9, 0.9);
    leg->AddEntry(plot_1, label_1+" Average AC");
    leg->AddEntry(plot_1_errors, label_1+" 1#sigma Error Band");

    leg->AddEntry(plot_2, label_2+" Average AC");
    leg->AddEntry(plot_2_errors, label_2+" 1#sigma Error Band");

    plot_1_errors->Draw("E4");

    plot_2_errors->Draw("E4 same");

    plot_1->Draw("l hist same");
    plot_2->Draw("l hist same");
    leg->Draw();

    c->Print(output_name);
}
