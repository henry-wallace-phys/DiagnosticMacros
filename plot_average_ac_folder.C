#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include <TFile.h>
#include <TKey.h>
#include <TDirectoryFile.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <fnmatch.h>
#include <TGraph.h>
#include <TGraphAsymmErrors.h>

namespace AutoCorrelationPlotter
{

    // Utility functions
    bool IsHistogramAllOnes(TH1D *hist, double tolerance = 0.001, int max_failures = 100)
    {
        int failure_count = 0;

        for (int bin = 2; bin <= hist->GetNbinsX(); ++bin)
        {
            if (fabs(hist->GetBinContent(bin) - 1.0) > tolerance)
            {
                if (++failure_count > max_failures)
                {
                    return false;
                }
            }
        }
        return true;
    }

    std::pair<TGraph *, TGraph *> CreateMinMaxBand(TH1D *hist, int color)
    {
        int nBins = hist->GetNbinsX();
        std::vector<double> x(nBins), ymin(nBins), ymax(nBins);

        for (int i = 0; i < nBins; ++i)
        {
            x[i] = hist->GetBinCenter(i + 1);
            ymin[i] = hist->GetBinContent(i + 1);
            ymax[i] = hist->GetBinContent(i + 1);
        }

        TGraph *minGraph = new TGraph(nBins, x.data(), ymin.data());
        TGraph *maxGraph = new TGraph(nBins, x.data(), ymax.data());

        minGraph->SetLineColor(color);
        maxGraph->SetLineColor(color);
        minGraph->SetFillColorAlpha(color, 0.3);
        maxGraph->SetFillColorAlpha(color, 0.3);

        return {minGraph, maxGraph};
    }

    std::pair<TGraph *, TGraph *> CalculateMinMaxBand(const std::vector<TH1D *> &histograms, int color)
    {
        if (histograms.empty())
        {
            throw std::invalid_argument("Empty histogram vector provided");
        }

        int nBins = histograms[0]->GetNbinsX();
        std::vector<double> x(nBins), ymin(nBins, 1e10), ymax(nBins, -1e10);

        for (int bin = 0; bin < nBins; bin++)
        {
            x[bin] = histograms[0]->GetBinCenter(bin + 1);

            for (const auto &hist : histograms)
            {
                double content = hist->GetBinContent(bin + 1);
                ymin[bin] = std::min(ymin[bin], content);
                ymax[bin] = std::max(ymax[bin], content);
            }
        }

        TGraph *minGraph = new TGraph(nBins, x.data(), ymin.data());
        TGraph *maxGraph = new TGraph(nBins, x.data(), ymax.data());

        // Create a band using TGraphAsymmErrors for the shaded region
        TGraphAsymmErrors *band = new TGraphAsymmErrors(nBins);
        for (int i = 0; i < nBins; ++i)
        {
            band->SetPoint(i, x[i], (ymax[i] + ymin[i]) / 2.0);
            band->SetPointError(i, 0, 0, (ymax[i] - ymin[i]) / 2.0, (ymax[i] - ymin[i]) / 2.0);
        }

        band->SetFillColorAlpha(color, 0.2);
        band->SetLineColor(color);
        band->SetLineWidth(1);

        return {band, minGraph}; // Return band and min graph (min graph can be used for legend)
    }

    std::pair<TH1D *, TH1D *> CalculateMinMaxHistograms(const std::vector<TH1D *> &histograms)
    {
        if (histograms.empty())
        {
            throw std::invalid_argument("Empty histogram vector provided");
        }

        TH1D *min_hist = static_cast<TH1D *>(histograms[0]->Clone());
        TH1D *max_hist = static_cast<TH1D *>(histograms[0]->Clone());

        for (const auto &hist : histograms)
        {
            for (int bin = 1; bin <= min_hist->GetNbinsX(); ++bin)
            {
                double current_min = min_hist->GetBinContent(bin);
                double current_max = max_hist->GetBinContent(bin);
                double bin_content = hist->GetBinContent(bin);

                min_hist->SetBinContent(bin, std::min(current_min, bin_content));
                max_hist->SetBinContent(bin, std::max(current_max, bin_content));
            }
        }

        return {min_hist, max_hist};
    }

    // File processing functions
    void ProcessAutoCorrelationDirectory(TDirectoryFile *autocor_dir,
                                         TH1D *&average_hist,
                                         int &parameter_count,
                                         std::vector<TH1D *> &histograms)
    {
        TIter next(autocor_dir->GetListOfKeys());
        TKey *key;

        while ((key = dynamic_cast<TKey *>(next())))
        {
            TH1D *current_hist = nullptr;
            autocor_dir->GetObject(key->GetName(), current_hist);

            if (!current_hist ||
                current_hist->GetMaximum() <= 0 ||
                IsHistogramAllOnes(current_hist))
            {
                continue;
            }

            current_hist->SetDirectory(nullptr); // Detach from file
            histograms.push_back(current_hist);

            if (!average_hist)
            {
                average_hist = static_cast<TH1D *>(current_hist->Clone());
                average_hist->SetDirectory(nullptr);
            }
            else
            {
                average_hist->Add(current_hist);
            }
            parameter_count++;
        }
    }

    void ProcessDiagnosticFile(const TString &file_path,
                               TH1D *&average_hist,
                               int &parameter_count,
                               std::vector<TH1D *> &histograms)
    {
        std::unique_ptr<TFile> input_file(TFile::Open(file_path));
        if (!input_file || input_file->IsZombie())
        {
            throw std::runtime_error("Could not open file: " + std::string(file_path.Data()));
        }

        TDirectoryFile *autocor_dir = nullptr;
        input_file->GetObject("Auto_corr", autocor_dir);

        if (!autocor_dir)
        {
            throw std::runtime_error("Auto_corr directory not found in file: " + std::string(file_path.Data()));
        }

        ProcessAutoCorrelationDirectory(autocor_dir, average_hist, parameter_count, histograms);
    }

    // File search functions
    void FindMatchingFilesRecursive(const TString &base_dir,
                                    const std::vector<TString> &pattern_parts,
                                    size_t current_part_index,
                                    std::vector<TString> &result_files)
    {
        if (current_part_index >= pattern_parts.size())
            return;

        TSystemDirectory dir(base_dir, base_dir);
        std::unique_ptr<TList> entries(dir.GetListOfFiles());
        if (!entries)
            return;

        TIter next(entries.get());
        TSystemFile *entry;
        const TString &current_pattern = pattern_parts[current_part_index];

        while ((entry = dynamic_cast<TSystemFile *>(next())))
        {
            TString name = entry->GetName();
            if (name == "." || name == "..")
                continue;

            if (fnmatch(current_pattern.Data(), name.Data(), 0) != 0)
                continue;

            TString full_path = base_dir + "/" + name;
            full_path.ReplaceAll("//", "/");

            if (current_part_index == pattern_parts.size() - 1)
            {
                if (!entry->IsDirectory())
                {
                    result_files.push_back(full_path);
                }
            }
            else
            {
                if (entry->IsDirectory())
                {
                    FindMatchingFilesRecursive(full_path, pattern_parts, current_part_index + 1, result_files);
                }
            }
        }
    }

    std::vector<TString> FindFilesWithWildcard(const TString &full_pattern)
    {
        std::vector<TString> pattern_parts;
        TString token;
        Ssiz_t from = 0;

        while (full_pattern.Tokenize(token, from, "/"))
        {
            if (!token.IsNull())
            {
                pattern_parts.push_back(token);
            }
        }

        if (pattern_parts.empty())
        {
            return {};
        }

        TString base_dir = ".";
        if (!pattern_parts.empty() && !pattern_parts[0].Contains("*"))
        {
            base_dir = pattern_parts[0];
            pattern_parts.erase(pattern_parts.begin());
        }

        std::vector<TString> matching_files;
        FindMatchingFilesRecursive(base_dir, pattern_parts, 0, matching_files);
        return matching_files;
    }

    // Main processing function
    TH1D *ProcessInputFolder(const TString &folder_path, std::vector<TH1D *> &histograms)
    {
        std::vector<TString> input_files = FindFilesWithWildcard(folder_path);

        if (input_files.empty())
        {
            std::cerr << "Warning: No matching files found in " << folder_path << std::endl;
            return nullptr;
        }

        TH1D *average_hist = nullptr;
        int parameter_count = 0;

        for (const auto &file_path : input_files)
        {
            try
            {
                ProcessDiagnosticFile(file_path, average_hist, parameter_count, histograms);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error processing file " << file_path << ": " << e.what() << std::endl;
            }
        }

        if (average_hist && parameter_count > 0)
        {
            std::cout << "Processed " << parameter_count << " parameters from "
                      << input_files.size() << " files." << std::endl;
            average_hist->Scale(1.0 / parameter_count);
        }

        return average_hist;
    }

    // Plotting functions
    std::pair<double, double> DrawMinMaxHistograms(const std::vector<TH1D *> &histograms,
                                                   const TString &label,
                                                   int color,
                                                   int line_style,
                                                   bool draw_on_canvas,
                                                   TLegend *legend = nullptr)
    {
        auto [min_hist, max_hist] = CalculateMinMaxHistograms(histograms);

        if (draw_on_canvas)
        {
            min_hist->SetLineColor(color);
            min_hist->SetLineWidth(1);
            min_hist->SetLineStyle(line_style);
            min_hist->Smooth(3);
            min_hist->Draw("same");

            max_hist->SetLineColor(color);
            max_hist->SetLineWidth(1);
            max_hist->SetLineStyle(line_style);
            max_hist->Smooth(1);
            max_hist->Draw("same");

            if (legend)
            {
                legend->AddEntry(min_hist, "Full range " + label, "l");
            }
        }

        return {min_hist->GetMinimum(), max_hist->GetMaximum()};
    }

    void CreateComparisonPlot(TH1D *hist1, TH1D *hist2,
                              const std::vector<TH1D *> &histograms1,
                              const std::vector<TH1D *> &histograms2,
                              const TString &label1, const TString &label2,
                              const TString &output_filename,
                              bool show_min_max = true,
                              bool show_all_histograms = false,
                              bool draw_error_bands = true)
    {
        if (!hist1 || !hist2)
        {
            throw std::invalid_argument("Invalid histograms provided for plotting");
        }

        std::unique_ptr<TCanvas> canvas(new TCanvas("ac_canvas", "Autocorrelation Comparison", 1200, 600));
        canvas->SetGrid();

        // Configure histograms
        const int color1 = kAzure - 2;
        const int color2 = kOrange + 7;

        hist1->SetLineColor(color1);
        hist1->SetLineWidth(1);
        hist1->SetTitle("Average Autocorrelation Comparison");
        hist1->GetXaxis()->SetTitle("Lag");
        hist1->GetYaxis()->SetTitle("Autocorrelation");

        std::unique_ptr<TH1D> error_band1(static_cast<TH1D *>(hist1->Clone()));
        error_band1->SetFillColorAlpha(color1, 0.3);

        hist2->SetLineColor(color2);
        hist2->SetLineWidth(2);
        std::unique_ptr<TH1D> error_band2(static_cast<TH1D *>(hist2->Clone()));
        error_band2->SetFillColorAlpha(color2, 0.3);

        // Create legend
        std::unique_ptr<TLegend> legend(new TLegend(0.6, 0.7, 0.9, 0.9));
        legend->SetHeader("Autocorrelation Comparison", "C");
        legend->AddEntry(hist1, label1 + " Average", "l");
        legend->AddEntry(error_band1.get(), label1 + " #pm1#sigma", "f");
        legend->AddEntry(hist2, label2 + " Average", "l");
        legend->AddEntry(error_band2.get(), label2 + " #pm1#sigma", "f");

        // Create min-max bands
        auto [band1, minGraph1] = CalculateMinMaxBand(histograms1, color1);
        auto [band2, minGraph2] = CalculateMinMaxBand(histograms2, color2);

        if (show_min_max)
        {
            legend->AddEntry(band1, label1 + " Full Range", "f");
            legend->AddEntry(band2, label2 + " Full Range", "f");
        }

        // Determine Y-axis range
        double y_min = std::min(band1->GetYaxis()->GetXmin(), band2->GetYaxis()->GetXmin());
        double y_max = std::max(band1->GetYaxis()->GetXmax(), band2->GetYaxis()->GetXmax());
        hist1->GetYaxis()->SetRangeUser(y_min * 0.95, y_max * 1.05);

        // Draw elements in proper order
        hist1->Draw("AXIS"); // Draw axis first

        if (show_min_max)
        {
            band1->Draw("3 same");
            band2->Draw("3 same");
        }

        if(draw_error_bands)
        { 
            error_band1->Draw("E3 same");
            error_band2->Draw("E3 same");
        }

        hist1->Draw("l hist same"); // Redraw to be on top
        hist2->Draw("l hist same");

        if (show_all_histograms)
        {
            for (auto hist : histograms1)
            {
                hist->SetLineColorAlpha(color1, 0.1);
                hist->SetLineWidth(1);
                hist->Draw("l same");
            }
            for (auto hist : histograms2)
            {
                hist->SetLineColorAlpha(color2, 0.1);
                hist->SetLineWidth(1);
                hist->Draw("l same");
            }
        }

        legend->Draw();
        canvas->Print(output_filename);

        // Cleanup
        delete band1;
        delete minGraph1;
        delete band2;
        delete minGraph2;

        std::cout << "Plot saved to " << output_filename << std::endl;
    }

} // namespace AutoCorrelationPlotter

// Main interface function
void plot_average_ac_folder(const TString &folder1, const TString &label1,
                            const TString &folder2, const TString &label2,
                            const TString &output_name,
                            bool draw_min_max = true,
                            bool draw_all = false,
                            bool draw_errors = true)
{
    std::cout << "Comparing autocorrelations between:\n"
              << "  - " << folder1 << " (" << label1 << ")\n"
              << "  - " << folder2 << " (" << label2 << ")\n";

    // Process first folder
    std::vector<TH1D *> histograms1;
    std::unique_ptr<TH1D> average1(AutoCorrelationPlotter::ProcessInputFolder(folder1, histograms1));
    if (!average1)
    {
        throw std::runtime_error("Failed to process first folder: " + std::string(folder1.Data()));
    }

    // Process second folder
    std::vector<TH1D *> histograms2;
    std::unique_ptr<TH1D> average2(AutoCorrelationPlotter::ProcessInputFolder(folder2, histograms2));
    if (!average2)
    {
        throw std::runtime_error("Failed to process second folder: " + std::string(folder2.Data()));
    }

    // Create comparison plot
    AutoCorrelationPlotter::CreateComparisonPlot(
        average1.get(), average2.get(),
        histograms1, histograms2,
        label1, label2,
        output_name,
        draw_min_max,
        draw_all,
        draw_errors
    );
}