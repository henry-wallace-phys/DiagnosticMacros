#!/bin/bash

STUDIES=("all_fixed" "all_on" "oct_fix_mo_fix" "oct_fix_no_only")

# First single study comps [I CBA TO BE CLEVER]
for k in "${!STUDIES[@]}"; do
    STU=${STUDIES[$k]}

    nova_adapt_file=Adapt/NOvA/LongFit/NoCut/mcmc_Adapt_NOvA_${STU}_long 
    nova_no_adapt_file=NoAdapt/NOvA/LongFit/NoCut/mcmc_NoAdapt_NOvA_${STU}_long
    t2k_adapt_file=Adapt/T2K/LongFit/NoCut/mcmc_Adapt_T2K_${STU}_long
    t2k_no_adapt_file=NoAdapt/T2K/LongFit/NoCut/mcmc_NoAdapt_T2K_${STU}_long

    mkdir -p plots/${STU}

    # Posteriors
    root -l -q -b 'scripts/compare_posteriors.C("'${nova_adapt_file}'.root", "NOvA Adapt", true, "'${nova_no_adapt_file}'.root", "NOvA No Adapt", true, "plots/'${STU}'/nova_adapt_no_adapt_comp_'${STU}'.pdf")'
    root -l -q -b 'scripts/compare_posteriors.C("'${t2k_adapt_file}'.root", "T2K Adapt", false, "'${t2k_no_adapt_file}'.root", "T2K No Adapt", false, "plots/'${STU}'/t2k_adapt_no_adapt_comp_'${STU}'.pdf")'
    root -l -q -b 'scripts/compare_posteriors.C("'${nova_adapt_file}'.root", "NOvA Adapt", true, "'${t2k_adapt_file}'.root", "T2K Adapt", false, "plots/'${STU}'/nova_t2k_adapt_comp_'${STU}'.pdf")'
    root -l -q -b 'scripts/compare_posteriors.C("'${nova_no_adapt_file}'.root", "NOvA No Adapt", true, "'${t2k_no_adapt_file}'.root", "T2K No Adapt", false, "plots/'${STU}'/nova_t2k_no_adapt_comp_'${STU}'.pdf")'

    # Diagnostics comp
    diag_suffix="_MCMC_Diag_no_adapt_step.root"
    root -l -b -q 'scripts/plot_diag_comp.C("'${nova_adapt_file}${diag_suffix}'", "NOvA Adapt", "'${nova_no_adapt_file}${diag_suffix}'", "NOvA No Adapt", "plots/'${STU}'/nova_adapt_no_adapt_diag_comp_'${STU}'")'
    root -l -b -q 'scripts/plot_diag_comp.C("'${t2k_adapt_file}${diag_suffix}'", "T2K Adapt", "'${t2k_no_adapt_file}${diag_suffix}'", "T2K No Adapt", "plots/'${STU}'/t2k_adapt_no_adapt_diag_comp_'${STU}'")'
    root -l -b -q 'scripts/plot_diag_comp.C("'${nova_adapt_file}${diag_suffix}'", "NOvA Adapt", "'${t2k_adapt_file}${diag_suffix}'", "T2K Adapt", "plots/'${STU}'/nova_t2k_adapt_diag_comp_'${STU}'")'
    root -l -b -q 'scripts/plot_diag_comp.C("'${nova_no_adapt_file}${diag_suffix}'", "NOvA No Adapt", "'${t2k_no_adapt_file}${diag_suffix}'", "T2K No Adapt", "plots/'${STU}'/nova_t2k_no_adapt_diag_comp_'${STU}'")'

    # Autocorrelation COMP
    root -l -b -q 'scripts/plot_average_ac_folder.C("'${nova_no_adapt_file}${diag_suffix}'", "NOvA No Adapt", "'${t2k_no_adapt_file}${diag_suffix}'", "T2K No Adapt", "plots/'${STU}'/t2k_nova_no_adapt_ac_comp'${STU}'.png", true, false, true)'
    root -l -b -q 'scripts/plot_average_ac_folder.C("'${nova_adapt_file}${diag_suffix}'", "NOvA Adapt", "'${t2k_adapt_file}${diag_suffix}'", "T2K Adapt", "plots/'${STU}'/t2k_nova_adapt_ac_comp'${STU}'.png", true, false, true)'
done

# Now general comps
root -l -b -q 'scripts/plot_average_ac_folder.C("'Adapt/NOvA/LongFit/NoCut/*Diag*'", "NOvA Adapt", "Adapt/T2K/NoCut/*Diag*", "T2K Adapt", "plots/t2k_nova_adapt_ac_comp.png", true, false, true)'
root -l -b -q 'scripts/plot_average_ac_folder.C("'NoAdapt/NOvA/LongFit/NoCut/*Diag*'", "NOvA No Adapt", "NoAdapt/T2K/NoCut/*Diag*", "T2K No Adapt", "plots/LongFit/t2k_nova_no_adapt_ac_comp.png", true, false, true)'
root -l -b -q 'scripts/plot_average_ac_folder.C("'Adapt/*/LongFit/NoCut/*Diag*'", "Adapt", "NoAdapt/*/LongFit/NoCut/*Diag*", "No Adapt", "plots/no_adapt_adapt_ac_comp.png", true, false, true)'

