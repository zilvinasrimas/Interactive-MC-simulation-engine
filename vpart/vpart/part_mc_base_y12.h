#ifndef PART_MC_BASE_Y12_H
#define PART_MC_BASE_Y12_H


#include<vector>
#include<map>
#include<QString>
#include "dialog.h"
#include "part_mc.h"

class Dialog;
class cell;
class par;

class Part_mc_base_y12
{
public:
    Part_mc_base_y12();
    void makeMCstep(par &pr);
    double getCurrentTimeStep();
    void initialize(Dialog *mainDialog, par &pr);
    void updateBeta(double &new_b, par &pr);
    void updateMu(double &new_mu, par &pr);

private:
    void read_lattice_data(par &pr);
    void initializing_envs(par &pr);
    void initializing_envmt_envloc_scan(par &pr);
    void initializing_sums_degree_8_scan();
    int find_changing_env_degree_8_scan(par &pr,double &r1, bool &filled);
    void make_change_scan(par &pr, bool &filled, int k);
    void update_envs(par &pr);
    void update_sums_degree_8();

    std::vector<cell> cells;
    std::vector<std::vector<double> > s_envs_p;
    std::vector<std::vector<std::vector< long long> > > s_envs;
    std::vector<std::vector<std::vector<double> > > s_sums;
    long emf,eff;
    double s_sum;
    bool filled=0;

    Dialog *mainGui;


};

#endif // PART_MC_BASE_Y12_H
