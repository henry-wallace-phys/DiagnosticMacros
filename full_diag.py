import uproot
import pandas as pd
import numpy as np
from typing import Sequence
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
import tqdm

class RootTreeHandler:
    def __init__(self, file_path: str, label: str, step_cut: int, tree_name: str="posteriors")->None:
        '''
        Constructor for RootFileHandler class.
        :param file_path: Path to the ROOT file.
        '''
        print(f"Loading {file_path}...")
        ttree_ = uproot.open(file_path+f":{tree_name}")
        self._label = label
        
        self.stored_tree = ttree_.arrays(library="pd", cut=f"step>{step_cut}")
    
    @property
    def label(self):
        return self._label    
    
    def get_param_vals(self, param: str):
        '''
        Get the values of a specific parameter at a given step.
        :param param: The parameter to retrieve.
        :param step: The step number.
        :return: A list of values for the specified parameter at the given step.
        '''
        return self.stored_tree[param].values.tolist()
    
    def get_autocorr_func(self, param: str):
        '''
        Get the autocorrelation of a specific parameter.
        :param param: The parameter to retrieve.
        :return: A list of autocorrelation values for the specified parameter.
        '''
        param_arr = np.array(self.get_param_vals(param))
        
        div = np.std(param_arr) * np.sqrt(param_arr)
        
        if div.any() == 0 or np.isnan(div).any():
            div = np.ones(len(param_arr))
            
        sig_norm = (param_arr - np.mean(param_arr)) / div
        fft_param = np.fft.fft(sig_norm, len(sig_norm))
        psd = np.abs(fft_param) ** 2
        autocorr = np.fft.ifft(psd).real
        autocorr /= np.max(autocorr)
        return autocorr

    
    def get_params(self):
        '''
        Get the list of parameters in the tree.
        :return: A list of parameter names.
        '''
        return self.stored_tree.columns.tolist()
    
    def get_min_max(self, param: str):

        return self.stored_tree[param].min(), self.stored_tree[param].max()

    def get_posterior_hist(self, param: str, bins: int | Sequence = 100):
        '''
        Get the histogram of a specific parameter.
        :param param: The parameter to retrieve.
        :param bins: The number of bins for the histogram.
        :return: A list of histogram values for the specified parameter.
        '''
        hist, bin_edges = np.histogram(self.stored_tree[param], bins=bins)
        return hist, bin_edges
    
class TreeComparitor:
    def __init__(self, file_path_1, label_1, file_path_2, label_2, step_cut=0, tree_name="posteriors"):
        self.tree_1 = RootTreeHandler(file_path_1, label_1, step_cut, tree_name)
        self.tree_2 = RootTreeHandler(file_path_2, label_2, step_cut, tree_name)
    
    def get_min_max(self, param: str):
        min_1, max_1 = self.tree_1.get_min_max(param)
        min_2, max_2 = self.tree_2.get_min_max(param)
        return min(min_1, min_2), max(max_1, max_2)
    
    def get_posterior_hists(self, param: str, bins: int = 25):

        # Get the min and max values for the parameter from both trees
        min_, max_ = self.get_min_max(param)
        
        # Get bins
        bins_arr = np.linspace(min_, max_, bins)
        
        hist_1, bin_edges_1 = self.tree_1.get_posterior_hist(param, bins_arr)
        hist_2, bin_edges_2 = self.tree_2.get_posterior_hist(param, bins_arr)
        return hist_1, bin_edges_1, hist_2, bin_edges_2
    
    def get_params(self):
        '''
        Get the list of parameters in the tree.
        :return: A list of parameter names.
        '''
        return self.tree_1.get_params()
    
    def get_autocorr_func(self, param: str):
        '''
        Get the autocorrelation of a specific parameter.
        :param param: The parameter to retrieve.
        :return: A list of autocorrelation values for the specified parameter.
        '''
        return self.tree_1.get_autocorr_func(param), self.tree_2.get_autocorr_func(param)
    
    def get_param_vals(self, param: str):
        '''
        Get the values of a specific parameter at a given step.
        :param param: The parameter to retrieve.
        :return: A list of values for the specified parameter at the given step.
        '''
        return self.tree_1.get_param_vals(param), self.tree_2.get_param_vals(param)
    
class TreePlotter:
    def __init__(self, file_path_1, label_1, file_path_2, label_2, step_cut=0, tree_name="posteriors"):
        self.tree_comparitor = TreeComparitor(file_path_1, label_1, file_path_2, label_2, step_cut, tree_name)
    

    def plot_parameter(self, param: str):
        '''
        Plot trace, posterior, and autocorrelation for a given parameter on the same figure.
        :param param: The parameter to plot.
        '''
        
        trace_1, trace_2 = self.tree_comparitor.get_param_vals(param)
        hist_1, bin_edges_1, hist_2, bin_edges_2 = self.tree_comparitor.get_posterior_hists(param)
        autocorr_1, autocorr_2 = self.tree_comparitor.get_autocorr_func(param)
    
        # Create a new figure
# Create figure with custom layout
        fig = plt.figure(figsize=(10, 8))
        gs = fig.add_gridspec(2, 2, width_ratios=[3, 1], height_ratios=[3, 1])

        fig.suptitle(f"Parameter: {param}", fontsize=16)
        # Plot trace
        
        ax_trace = fig.add_subplot(gs[0, 0])
        
        ax_trace.plot(trace_1, label=self.tree_comparitor.tree_1.label)
        ax_trace.plot(trace_2, label=self.tree_comparitor.tree_2.label)
        ax_trace.set_title("Trace")
        ax_trace.set_xlabel("Step")
        ax_trace.set_ylabel(param)
        ax_trace.legend()
        ax_trace.grid()
        # Plot posterior
        ax_post = fig.add_subplot(gs[0, 1])
        ax_post.bar(hist_1, height=np.diff(bin_edges_1), label=self.tree_comparitor.tree_1.label)
        ax_post.bar(hist_2, height=np.diff(bin_edges_2), label=self.tree_comparitor.tree_2.label)
        ax_post.set_title("Posterior")
        ax_post.set_xlabel(param)
        ax_post.set_ylabel("Posterior Density")
        ax_post.legend()
        ax_post.grid()
        # Plot autocorrelation
        ax_acf = fig.add_subplot(gs[1, 0])

        ax_acf.plot(autocorr_1, label=self.tree_comparitor.tree_1.label)
        ax_acf.plot(autocorr_2, label=self.tree_comparitor.tree_2.label)
        ax_acf.set_title("Autocorrelation")
        ax_acf.set_xlabel("Lag [step]")
        ax_acf.set_ylabel("Autocorrelation")
        ax_acf.legend()
        ax_acf.grid()
        # Adjust layout
        plt.tight_layout(rect=[0, 0.03, 1, 0.95])
        
        return fig, gs
    
    def __call__(self, output: str = "trace_comp.pdf"):
        with PdfPages(output) as pdf:
            for param in tqdm.tqdm(self.tree_comparitor.get_params()):
                fig, _ = self.plot_parameter(param)
                pdf.savefig(fig)
                plt.close(fig)
            
if __name__ == "__main__":
    import sys
    if len(sys.argv) < 5:
        print("Usage: python compare_chains.py file1 label1 file2 label2 [output]")
        sys.exit(1)
    
    
    output = sys.argv[5] if len(sys.argv) > 5 else "trace_comp.pdf"

    file_path_1 = sys.argv[1]
    label_1 = sys.argv[2]
    file_path_2 = sys.argv[3]
    label_2 = sys.argv[4]
    step_cut = 100000
    
    tree_plotter = TreePlotter(file_path_1, label_1, file_path_2, label_2, step_cut)
    tree_plotter(output)
    print(f"Comparison saved to {output}")