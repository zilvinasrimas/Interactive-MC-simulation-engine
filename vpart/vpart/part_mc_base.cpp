#include "part_mc_base.h"
#include<cmath>
#include<cstdlib>
#include<QFile>
#include<QTextStream>
#include<QDebug>
#include<algorithm>

using namespace std;

Part_mc_base::Part_mc_base()
{

}

void Part_mc_base::makeMCstep(par &pr)
{
    make_change(pr,find_changing_env(pr));
    update_envs(pr);
}

double Part_mc_base::getCurrentTimeStep()
{
    return 1.0/w_sum;
}

void Part_mc_base::initialize(Dialog *mainDialog, par &pr)
{
    mainGui=mainDialog;

    random_device rd;
    mt19937 temp_mt_engine(rd());
    pr.mt_engine=temp_mt_engine;

    pr.b=mainGui->global_beta;
    pr.mu=mainGui->global_mu;
    pr.fname=mainGui->sample_name;
    pr.E=0;
    pr.zkdens.assign(4,0);
    pr.zkl.assign(4,0);
    eff=1;
    emf=mainGui->wettability*eff;

    read_lattice_data(pr);
    initializing_envs(pr);
    initializing_envmt_envloc(pr);
    update_envs(pr);

    mainGui->global_N=pr.rdCellSize;
    mainGui->global_cells.clear();
    for(unsigned int i=0;i<pr.rdCellSize;i++)
    {
        vector<long long> temp_cell_data(4,0);
        temp_cell_data[0]=cells[i].x;
        temp_cell_data[1]=cells[i].y;
        temp_cell_data[2]=cells[i].z;
        mainGui->global_cells.push_back(temp_cell_data);
    }
}

void Part_mc_base::read_lattice_data(par &pr)
{
    unsigned long long nnsize=0, tmp=0;
    cell ctmp;
    QFile infile("../in/" + pr.fname + ".dat");
    infile.open(QIODevice::ReadWrite);
    QTextStream in(&infile);
    in>>pr.N>>pr.x>>pr.y>>pr.zkl[1]>>pr.zkl[2]>>pr.zkl[3]>>pr.degree;
    pr.Nenvs=(pr.degree+1)*(pr.degree+1);
    pr.zkl[0]=pr.zkl[1]+2*pr.zkl[2]+2*pr.zkl[3];
    mainGui->dim_x=pr.x;mainGui->dim_y=pr.y;mainGui->dim_z=pr.zkl[0];
    if(pr.zkl[3]==0)    //this is to make data structures which need x.y.z but dont have forced empty b.c. in z direction (aerogel with suffix c)
    {
        pr.rdCellSize=pr.N;
    }
    else
    {
        pr.rdCellSize=pr.N-2*pr.x*pr.y;
    }
    for(long long i=0;i<pr.N;i++)
    {
        cells.push_back(ctmp);
        in>>tmp>>cells.back().x>>cells.back().y>>cells.back().z>>cells.back().zkind>>nnsize;
        for(unsigned long long j=0;j<nnsize;j++)
        {
            in>>tmp;
            cells.back().nn.push_back(tmp);
        }
        cells.back().mf=pr.degree-cells.back().nn.size();
    }
    infile.close();
}

void Part_mc_base::initializing_envs(par &pr)
{
    s_envs.clear();
    s_envs_p.clear();
    vector<long long> temp_envs_struct;
    s_envs.assign((pr.degree+1)*(pr.degree+1)*2,temp_envs_struct);
    s_envs_p.assign((pr.degree+1)*(pr.degree+1)*2,0);
    s_envs_e.assign((pr.degree+1)*(pr.degree+1)*2,0);

    for(unsigned int i=0;i<pr.degree+1;i++)
    for(unsigned int j=0;j<pr.degree+1;j++)
    if(i+j<=pr.degree)
    {
        s_envs_e[(pr.degree+1)*j+i]=i*eff+j*emf;
        s_envs_e[(pr.degree+1)*(pr.degree+1+j)+i]=i*eff+j*emf;
    }
}

void Part_mc_base::initializing_envmt_envloc(par &pr)
{
    for(long long i=0;i<pr.rdCellSize;i++)
    {
        long long ff_interaction=0;
        for(unsigned long long j=0;j<cells[i].nn.size();j++) ff_interaction=ff_interaction+cells[cells[i].nn[j]].state;
        cells[i].envmt=(pr.degree+1)*((pr.degree+1)*cells[i].state+cells[i].mf)+ff_interaction;
        cells[i].envloc=s_envs[(pr.degree+1)*((pr.degree+1)*cells[i].state+cells[i].mf)+ff_interaction].size();
        s_envs[(pr.degree+1)*((pr.degree+1)*cells[i].state+cells[i].mf)+ff_interaction].push_back(i);
    }
}

int Part_mc_base::find_changing_env(par &pr)
{
    discrete_distribution<int> dist(pr.distWeights.begin(),pr.distWeights.end());
    return dist(pr.mt_engine);
}

void Part_mc_base::make_change(par &pr, int k)
{
    filled=(pr.Nenvs<=k);
    flip=1-2*filled;
    uniform_int_distribution<int> d(0,s_envs[k].size()-1);
    long long ch_cln,ch_env,r3=d(pr.mt_engine);
    long long changing_cell=s_envs[k][r3];
    cells[changing_cell].state=!filled;
    cells[changing_cell].envmt+=flip*pr.Nenvs;
    mainGui->global_cells[changing_cell][3]=!filled;
    cells[s_envs[k].back()].envloc=r3;
    s_envs[k][r3]=s_envs[k].back();
    s_envs[k].pop_back();
    cells[changing_cell].envloc=s_envs[k+flip*pr.Nenvs].size();
    s_envs[k+flip*pr.Nenvs].push_back(changing_cell);
    pr.zkdens[cells[changing_cell].zkind]+=flip;
    pr.zkdens[0]+=flip;
    pr.E+=s_envs_e[k]*flip;
    for(unsigned long long j=0;j<cells[changing_cell].nn.size();j++)
    {
        if(cells[cells[changing_cell].nn[j]].zkind<4)
        {
          ch_cln=cells[changing_cell].nn[j];
          ch_env=cells[ch_cln].envmt;

          cells[ch_cln].envmt=ch_env+flip;
          cells[s_envs[ch_env].back()].envloc=cells[ch_cln].envloc;
          s_envs[ch_env][cells[ch_cln].envloc]=s_envs[ch_env].back();
          s_envs[ch_env].pop_back();
          cells[ch_cln].envloc=s_envs[ch_env+flip].size();
          s_envs[ch_env+flip].push_back(ch_cln);
        }
    }
}

void Part_mc_base::updateBeta(double &new_b, par &pr)
{
    pr.b=new_b;
    update_envs(pr);
}

void Part_mc_base::updateMu(double &new_mu, par &pr)
{
    pr.mu=new_mu;
    update_envs(pr);
}

void Part_mc_base::update_envs(par &pr)
{
    pr.distWeights.assign((pr.degree+1)*(pr.degree+1)*2,0);
    w_sum=0;
    for(unsigned int i=0;i<pr.degree+1;i++)
    for(unsigned int j=0;j<pr.degree+1;j++)
    if(i+j<=pr.degree)
    {
        if(i*eff+j*emf+pr.mu>0)
        {
            s_envs_p[(pr.degree+1)*j+i]=1;
            s_envs_p[(pr.degree+1)*(pr.degree+1+j)+i]=exp(-pr.b*(i*eff+j*emf+pr.mu));
        }
        else
        {
            s_envs_p[(pr.degree+1)*j+i]=exp(pr.b*(i*eff+j*emf+pr.mu));
            s_envs_p[(pr.degree+1)*(pr.degree+1+j)+i]=1;
        }
        w_sum+=s_envs[(pr.degree+1)*j+i].size()*s_envs_p[(pr.degree+1)*j+i];
        w_sum+=s_envs[(pr.degree+1)*(pr.degree+1+j)+i].size()*s_envs_p[(pr.degree+1)*(pr.degree+1+j)+i];
        pr.distWeights[(pr.degree+1)*j+i]=s_envs[(pr.degree+1)*j+i].size()*s_envs_p[(pr.degree+1)*j+i];
        pr.distWeights[(pr.degree+1)*(pr.degree+1+j)+i]=s_envs[(pr.degree+1)*(pr.degree+1+j)+i].size()*s_envs_p[(pr.degree+1)*(pr.degree+1+j)+i];
    }
}

