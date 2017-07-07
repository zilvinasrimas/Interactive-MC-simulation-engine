#ifndef PART_MC_H
#define PART_MC_H

#include<vector>
#include<map>
#include<QString>
#include "dialog.h"
#include<random>
#include <QBitArray>

class Dialog;

class cell
{
    public:
    bool state;
    double mf;
    int envmt,envloc,x,y,z,zkind;
    std::vector<long long> nn;
    cell() {nn.clear();mf=0;state=0;envmt=0;envloc=0;x=0;y=0;z=0;zkind=-1;}
};

class par
{
    public:
    int printMeanEnergy;
    QBitArray cellState;
    std::vector<int> cellEnv,cellEnvloc;
    long long r3,iteration_count,total_iteration_count,Nenvs;
    long long N,degree,dens, x,y,rdCellSize,NsavedStates;
    std::vector<int> zkdens,zkl;
    double E,b,b_past,r1,mu,EquilibrationFactor,timeInStage;
    QString fname;
    std::mt19937 mt_engine;
    std::uniform_int_distribution<int> distSavedStates;
    std::uniform_real_distribution<double> distJumpProb,distJumpAtStep;
    std::vector<double> jumpAtStep,distWeights;
    par() {mu=0;r1=0;r3=0;b=0;b_past=0;N=0;degree=0;Nenvs=0;dens=0;E=0;fname.clear();zkdens.clear();x=0;y=0;zkl.clear();rdCellSize=0;cellState.clear();cellEnv.clear();cellEnvloc.clear();}
};

class sysState
{
    public:
    QBitArray cellState;
    std::vector<int> cellEnv,cellEnvloc;
    std::vector<int> zkdens;
    std::vector<std::vector<std::vector< int> > > s_envs;
    int E;
    sysState() {cellState.clear();cellEnv.clear();cellEnvloc.clear();zkdens.clear();s_envs.clear();E=0;}
};

class Part_mc
{
public:
    Part_mc();
};

#endif // PART_MC_H
