#include "part_mc_base_y12.h"
#include<cmath>
#include<cstdlib>
#include<QFile>
#include<QTextStream>
#include<QDebug>
#include<algorithm>

using namespace std;

Part_mc_base_y12::Part_mc_base_y12()
{

}

void Part_mc_base_y12::makeMCstep(par &pr)
{
    make_change_scan(pr,filled,find_changing_env_degree_8_scan(pr,pr.r1,filled));
    update_sums_degree_8();
}

double Part_mc_base_y12::getCurrentTimeStep()
{
    return 1.0/s_sum;
}

void Part_mc_base_y12::initialize(Dialog *mainDialog, par &pr)
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
    initializing_envmt_envloc_scan(pr);
    initializing_sums_degree_8_scan();

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

void Part_mc_base_y12::read_lattice_data(par &pr)
{
    unsigned long long nnsize=0, tmp=0;
    cell ctmp;
    QFile infile("../in/" + pr.fname + ".dat");
    infile.open(QIODevice::ReadWrite);
    QTextStream in(&infile);
    in>>pr.N>>pr.x>>pr.y>>pr.zkl[1]>>pr.zkl[2]>>pr.zkl[3]>>pr.degree;
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

void Part_mc_base_y12::initializing_envs(par &pr)
{
    s_envs.clear();
    vector<long long> temp_envs_list;
    vector<vector<long long> > temp_envs_struct(pr.degree*emf+1,temp_envs_list);
    s_envs.assign(2,temp_envs_struct);

    vector<double> temp_s_envs_p_struct;
    s_envs_p.assign(2,temp_s_envs_p_struct);
    for(unsigned int i=0;i<pr.degree*emf+1;i++)
    {
        if(i+pr.mu>0)
        {
            s_envs_p[0].push_back(1.0);
            s_envs_p[1].push_back(exp(-pr.b*(i+pr.mu)));
        }
        else
        {
            s_envs_p[0].push_back(exp(pr.b*(i+pr.mu)));
            s_envs_p[1].push_back(1.0);
        }
    }
}

void Part_mc_base_y12::initializing_envmt_envloc_scan(par &pr)
{
    for(long long i=0;i<pr.rdCellSize;i++)
    {
        long long ff_interaction=0;
        for(unsigned long long j=0;j<cells[i].nn.size();j++) ff_interaction=ff_interaction+cells[cells[i].nn[j]].state;
        cells[i].envmt=cells[i].mf*emf+ff_interaction*eff;
        cells[i].envloc=s_envs[cells[i].state][cells[i].envmt].size();
        s_envs[cells[i].state][cells[i].envmt].push_back(i);
    }
}

void Part_mc_base_y12::initializing_sums_degree_8_scan()
{
    vector<vector<double> >temp_sums;
    s_sums.assign(2,temp_sums);

    for(int fld=0;fld<2;fld++)
    {
        s_sums[fld].clear();
        vector<double> tempvec;

        if(emf==2)
        {
            tempvec.clear();
            for(unsigned int i=0;i<16;i++)
            {
                tempvec.push_back(s_envs[fld][i+1].size()*s_envs_p[fld][i+1]);
            }
            s_sums[fld].push_back(tempvec);

            tempvec.clear();
            for(unsigned int i=0;i<8;i++)
            {
                tempvec.push_back(s_sums[fld][0][2*i]+s_sums[fld][0][2*i+1]);
            }
            s_sums[fld].push_back(tempvec);

            tempvec.clear();
            for(unsigned int i=0;i<4;i++)
            {
            tempvec.push_back(s_sums[fld][1][2*i]+s_sums[fld][1][2*i+1]);
            }
            s_sums[fld].push_back(tempvec);

            tempvec.clear();
            for(unsigned int i=0;i<2;i++)
            {
                tempvec.push_back(s_sums[fld][2][2*i]+s_sums[fld][2][2*i+1]);
            }
            s_sums[fld].push_back(tempvec);

            tempvec.clear();
            tempvec.push_back(s_envs[fld][0].size()*s_envs_p[fld][0]);
            tempvec.push_back(s_sums[fld][3][0]+s_sums[fld][3][1]);
            s_sums[fld].push_back(tempvec);

            tempvec.clear();
            tempvec.push_back(s_sums[fld][4][0]+s_sums[fld][4][1]);
            s_sums[fld].push_back(tempvec);
        }
        else if(emf==1)
        {
            tempvec.clear();
            for(unsigned int i=0;i<8;i++)
            {
                tempvec.push_back(s_envs[fld][i+1].size()*s_envs_p[fld][i+1]);
            }
            s_sums[fld].push_back(tempvec);

            tempvec.clear();
            for(unsigned int i=0;i<4;i++)
            {
                tempvec.push_back(s_sums[fld][0][2*i]+s_sums[fld][0][2*i+1]);
            }
            s_sums[fld].push_back(tempvec);

            tempvec.clear();
            for(unsigned int i=0;i<2;i++)
            {
            tempvec.push_back(s_sums[fld][1][2*i]+s_sums[fld][1][2*i+1]);
            }
            s_sums[fld].push_back(tempvec);

            tempvec.clear();
            tempvec.push_back(s_envs[fld][0].size()*s_envs_p[fld][0]);
            tempvec.push_back(s_sums[fld][2][0]+s_sums[fld][2][1]);
            s_sums[fld].push_back(tempvec);

            tempvec.clear();
            tempvec.push_back(s_sums[fld][3][0]+s_sums[fld][3][1]);
            s_sums[fld].push_back(tempvec);
        }


    }
    s_sum=s_sums[0].back().back()+s_sums[1].back().back();
}

int Part_mc_base_y12::find_changing_env_degree_8_scan(par &pr, double &r1, bool &filled)
{
    uniform_real_distribution<double> dist(0,s_sum);
    r1=dist(pr.mt_engine);

    if(emf==2)
    {
        filled=(r1>=s_sums[0].back().back());
        if(filled) r1=r1-s_sums[0].back().back();
        if(s_sums[filled][4][0]>r1) {return 0;}
        else
        {
            r1=r1-s_sums[filled][4][0];
            if(s_sums[filled][3][0]>r1)
            {
                if(s_sums[filled][2][0]>r1)
                {
                    if(s_sums[filled][1][0]>r1)
                    {
                        if(s_sums[filled][0][0]>r1) {return 1;}
                        else {r1=r1-s_sums[filled][0][0]; return 2;}
                    }
                    else
                    {
                        r1=r1-s_sums[filled][1][0];
                        if(s_sums[filled][0][2]>r1) {return 3;}
                        else {r1=r1-s_sums[filled][0][2]; return 4;}
                    }
                }
                else
                {
                    r1=r1-s_sums[filled][2][0];
                    if(s_sums[filled][1][2]>r1)
                    {
                        if(s_sums[filled][0][4]>r1) {return 5;}
                        else {r1=r1-s_sums[filled][0][4]; return 6;}
                    }
                    else
                    {
                        r1=r1-s_sums[filled][1][2];
                        if(s_sums[filled][0][6]>r1) {return 7;}
                        else {r1=r1-s_sums[filled][0][6]; return 8;}
                    }
                }
            }
            else
            {
                r1=r1-s_sums[filled][3][0];
                if(s_sums[filled][2][2]>r1)
                {
                    if(s_sums[filled][1][4]>r1)
                    {
                        if(s_sums[filled][0][8]>r1) {return 9;}
                        else {r1=r1-s_sums[filled][0][8]; return 10;}
                    }
                    else
                    {
                        r1=r1-s_sums[filled][1][4];
                        if(s_sums[filled][0][10]>r1) {return 11;}
                        else {r1=r1-s_sums[filled][0][10]; return 12;}
                    }
                }
                else
                {
                    r1=r1-s_sums[filled][2][2];
                    if(s_sums[filled][1][6]>r1)
                    {
                        if(s_sums[filled][0][12]>r1) {return 13;}
                        else {r1=r1-s_sums[filled][0][12]; return 14;}
                    }
                    else
                    {
                        r1=r1-s_sums[filled][1][6];
                        if(s_sums[filled][0][14]>r1) {return 15;}
                        else {r1=r1-s_sums[filled][0][14]; return 16;}
                    }
                }
            }
        }
    }
    else if(emf==1)
    {
        filled=(r1>=s_sums[0].back().back());
        if(filled) r1=r1-s_sums[0].back().back();
        if(s_sums[filled][3][0]>r1) {return 0;}
        else
        {
            r1=r1-s_sums[filled][3][0];
            if(s_sums[filled][2][0]>r1)
            {
                if(s_sums[filled][1][0]>r1)
                {
                    if(s_sums[filled][0][0]>r1) {return 1;}
                    else {r1=r1-s_sums[filled][0][0]; return 2;}
                }
                else
                {
                    r1=r1-s_sums[filled][1][0];
                    if(s_sums[filled][0][2]>r1) {return 3;}
                    else {r1=r1-s_sums[filled][0][2]; return 4;}
                }
            }
            else
            {
                r1=r1-s_sums[filled][2][0];
                if(s_sums[filled][1][2]>r1)
                {
                    if(s_sums[filled][0][4]>r1) {return 5;}
                    else {r1=r1-s_sums[filled][0][4]; return 6;}
                }
                else
                {
                    r1=r1-s_sums[filled][1][2];
                    if(s_sums[filled][0][6]>r1) {return 7;}
                    else {r1=r1-s_sums[filled][0][6]; return 8;}
                }
            }
        }
    }
    else qDebug()<<"Chosen wettability (y) is not among the allowed values!";
}

void Part_mc_base_y12::make_change_scan(par &pr, bool &filled, int k)
{
    int flip=1-2*filled;
    long long ch_cln,ch_env,r3=pr.r1/s_envs_p[filled][k];

    long long changing_cell=s_envs[filled][k][r3];
    cells[changing_cell].state=!filled;
    mainGui->global_cells[changing_cell][3]=!filled;
    cells[s_envs[filled][k].back()].envloc=r3;
    s_envs[filled][k][r3]=s_envs[filled][k].back();
    s_envs[filled][k].pop_back();
    cells[changing_cell].envloc=s_envs[!filled][k].size();
    s_envs[!filled][k].push_back(changing_cell);
    pr.zkdens[cells[changing_cell].zkind]+=flip;
    pr.zkdens[0]+=flip;
    pr.E+=k*flip;

    for(unsigned long long j=0;j<cells[changing_cell].nn.size();j++)
    {
        if(cells[cells[changing_cell].nn[j]].zkind<4)
        {
            ch_cln=cells[changing_cell].nn[j];
            ch_env=cells[ch_cln].envmt;

            cells[ch_cln].envmt=cells[ch_cln].envmt+flip;
            cells[s_envs[cells[ch_cln].state][ch_env].back()].envloc=cells[ch_cln].envloc;
            s_envs[cells[ch_cln].state][ch_env][cells[ch_cln].envloc]=s_envs[cells[ch_cln].state][ch_env].back();
            s_envs[cells[ch_cln].state][ch_env].pop_back();
            cells[ch_cln].envloc=s_envs[cells[ch_cln].state][ch_env+flip].size();
            s_envs[cells[ch_cln].state][ch_env+flip].push_back(ch_cln);
        }
    }
}

void Part_mc_base_y12::updateBeta(double &new_b, par &pr)
{
    pr.b=new_b;
    update_envs(pr);
    update_sums_degree_8();
}

void Part_mc_base_y12::updateMu(double &new_mu, par &pr)
{
    pr.mu=new_mu;
    update_envs(pr);
    update_sums_degree_8();
}

void Part_mc_base_y12::update_envs(par &pr)
{

    for(unsigned int i=0;i<pr.degree*emf+1;i++)
    {
        if(i+pr.mu>0)
        {
            s_envs_p[0][i]=1.0;
            s_envs_p[1][i]=exp(-pr.b*(i+pr.mu));
        }
        else
        {
            s_envs_p[0][i]=exp(pr.b*(i+pr.mu));
            s_envs_p[1][i]=1.0;
        }
    }
}

void Part_mc_base_y12::update_sums_degree_8()
{
    if(emf==2)
    {
        for(int fld=0;fld<2;fld++)
        {
            for(int i=0;i<16;i++)
            {
                s_sums[fld][0][i]=s_envs[fld][i+1].size()*s_envs_p[fld][i+1];
            }

            for(unsigned int i=0;i<8;i++)
            {
                s_sums[fld][1][i]=s_sums[fld][0][2*i]+s_sums[fld][0][2*i+1];
            }

            for(unsigned int i=0;i<4;i++)
            {
                s_sums[fld][2][i]=s_sums[fld][1][2*i]+s_sums[fld][1][2*i+1];
            }

            for(unsigned int i=0;i<2;i++)
            {
                s_sums[fld][3][i]=s_sums[fld][2][2*i]+s_sums[fld][2][2*i+1];
            }

            s_sums[fld][4][0]=s_envs[fld][0].size()*s_envs_p[fld][0];
            s_sums[fld][4][1]=s_sums[fld][3][0]+s_sums[fld][3][1];
            s_sums[fld][5][0]=s_sums[fld][4][0]+s_sums[fld][4][1];
            s_sum=s_sums[0].back().back()+s_sums[1].back().back();
        }
    }
    else if(emf==1)
    {
        for(int fld=0;fld<2;fld++)
        {
            for(int i=0;i<8;i++)
            {
                s_sums[fld][0][i]=s_envs[fld][i+1].size()*s_envs_p[fld][i+1];
            }

            for(unsigned int i=0;i<4;i++)
            {
                s_sums[fld][1][i]=s_sums[fld][0][2*i]+s_sums[fld][0][2*i+1];
            }

            for(unsigned int i=0;i<2;i++)
            {
                s_sums[fld][2][i]=s_sums[fld][1][2*i]+s_sums[fld][1][2*i+1];
            }

            s_sums[fld][3][0]=s_envs[fld][0].size()*s_envs_p[fld][0];
            s_sums[fld][3][1]=s_sums[fld][2][0]+s_sums[fld][2][1];
            s_sums[fld][4][0]=s_sums[fld][3][0]+s_sums[fld][3][1];
            s_sum=s_sums[0].back().back()+s_sums[1].back().back();
        }
    }
}


