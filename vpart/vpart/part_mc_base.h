#ifndef PART_MC_BASE_H
#define PART_MC_BASE_H

#include<vector>
#include<map>
#include<QString>
#include "dialog.h"
#include "part_mc.h"

class Dialog;
class cell;
class par;

class Part_mc_base
{
public:
    Part_mc_base();
    void makeMCstep(par &pr);
    double getCurrentTimeStep();
    void initialize(Dialog *mainDialog, par &pr);
    void updateBeta(double &new_b, par &pr);
    void updateMu(double &new_mu, par &pr);

private:
    void read_lattice_data(par &pr);
    void initializing_envs(par &pr);
    void initializing_envmt_envloc(par &pr);
    int find_changing_env(par &pr);
    void make_change(par &pr, int k);
    void update_envs(par &pr);

    std::vector<cell> cells;
    std::vector<double> s_envs_p,s_envs_e;
    std::vector<std::vector< long long> > s_envs;
    bool filled=0;
    int flip;
    double w_sum,emf,eff;


    Dialog *mainGui;


};

#endif // PART_MC_BASE_H
