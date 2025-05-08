#!/bin/bash

STUDIES=("all_fixed" "all_on" "oct_fix_mo_fix" "oct_fix_no_only")

# First single study comps [I CBA TO BE CLEVER]
for k in "${!STUDIES[@]}"; do
    STU=${STUDIES[$k]}

    nova_adapt_file=Adapt/NOvA/Cut/mcmc_Adapt_NOvA_${STU} 
    nova_no_adapt_file=NoAdapt/NOvA/Cut/mcmc_NoAdapt_NOvA_${STU}
    t2k_adapt_file=Adapt/T2K/Cut/mcmc_Adapt_T2K_${STU} 
    t2k_no_adapt_file=NoAdapt/T2K/Cut/mcmc_NoAdapt_T2K_${STU}

    # Posteriors
    root -l -q -b 'scripts/compare_posteriors.C("'${nova_adapt_file}'.root", "NOvA Adapt", true, "'${nova_no_adapt_file}'.root", "NOvA No Adapt", true, "plots/nova_adapt_no_adapt_comp_'${STU}'.pdf")'
    root -l -q -b 'scripts/compare_posteriors.C("'${t2k_adapt_file}'.root", "T2K Adapt", false, "'${t2k_no_adapt_file}'.root", "T2K No Adapt", false, "plots/t2k_adapt_no_adapt_comp_'${STU}'.pdf")'
    root -l -q -b 'scripts/compare_posteriors.C("'${nova_adapt_file}'.root", "NOvA Adapt", true, "'${t2k_adapt_file}'.root", "T2K Adapt", false, "plots/nova_t2k_adapt_comp_'${STU}'.pdf")'
    root -l -q -b 'scripts/compare_posteriors.C("'${nova_no_adapt_file}'.root", "NOvA No Adapt", true, "'${t2k_no_adapt_file}'.root", "T2K No Adapt", false, "plots/nova_t2k_no_adapt_comp_'${STU}'.pdf")'

    # Diagnostics comp
    diag_suffix="_MCMC_Diag_no_adapt_step.root"
    root -l -b -q 'scripts/plot_diag_comp.C("'${nova_adapt_file}${diag_suffix}'", "NOvA Adapt", "'${nova_no_adapt_file}${diag_suffix}'", "NOvA No Adapt", "plots/nova_adapt_no_adapt_diag_comp_'${STU}'.pdf")'
    root -l -b -q 'scripts/plot_diag_comp.C("'${t2k_adapt_file}${diag_suffix}'", "T2K Adapt", "'${t2k_no_adapt_file}${diag_suffix}'", "T2K No Adapt", "plots/t2k_adapt_no_adapt_diag_comp_'${STU}'.pdf")'
    root -l -b -q 'scripts/plot_diag_comp.C("'${nova_adapt_file}${diag_suffix}'", "NOvA Adapt", "'${t2k_adapt_file}${diag_suffix}'", "T2K Adapt", "plots/nova_t2k_adapt_diag_comp_'${STU}'.pdf")'
    root -l -b -q 'scripts/plot_diag_comp.C("'${nova_no_adapt_file}${diag_suffix}'", "NOvA No Adapt", "'${t2k_no_adapt_file}${diag_suffix}'", "T2K No Adapt", "plots/nova_t2k_no_adapt_diag_comp_'${STU}'.pdf")'

    # Autocorrelation COMP
    root -l -b -q 'scripts/plot_average_ac_folder.C("'${nova_no_adapt_file}${diag_suffix}'", "NOvA No Adapt", "'${t2k_no_adapt_file}${diag_suffix}'", "T2K No Adapt", "plots/t2k_nova_no_adapt_ac_comp'${STU}'.pdf", true, false, true)'
    root -l -b -q 'scripts/plot_average_ac_folder.C("'${nova_adapt_file}${diag_suffix}'", "NOvA Adapt", "'${t2k_adapt_file}${diag_suffix}'", "T2K Adapt", "plots/t2k_nova_adapt_ac_comp'${STU}'.pdf", true, false, true)'
done

# Now general comps
root -l -b -q 'scripts/plot_average_ac_folder.C("'Adapt/NOvA/Cut/*Diag*'", "NOvA Adapt", "Adapt/T2K/Cut/*Diag*", "T2K Adapt", "plots/t2k_nova_adapt_ac_comp.pdf", true, false, true)'
root -l -b -q 'scripts/plot_average_ac_folder.C("'NoAdapt/NOvA/Cut/*Diag*'", "NOvA No Adapt", "NoAdapt/T2K/Cut/*Diag*", "T2K No Adapt", "plots/t2k_nova_no_adapt_ac_comp.pdf", true, false, true)'
root -l -b -q 'scripts/plot_average_ac_folder.C("'Adapt/*/Cut/*Diag*'", "Adapt", "NoAdapt/*/Cut/*Diag*", "No Adapt", "plots/no_adapt_adapt_ac_comp.pdf", true, false, true)'

