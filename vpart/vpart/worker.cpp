#include <QtCore>
#include "worker.h"

Worker::Worker(QObject *parent) : QObject(parent)
{
}

void Worker::secondary_constructor(Dialog *mainDialog)
{
    mainGui=mainDialog;
}

void Worker::on_start_button_clicked()
{
    par pr;

    if((mainGui->wettability==1) or (mainGui->wettability==2))
    {
        if(mainGui->runType=="basic") run_part_mc_base_y12(pr);
        //place for choosing different run modes (not available in this version)
    }
    else
    {
        if(mainGui->runType=="basic") run_part_mc_base(pr);
        //place for choosing different run modes (not available in this version)
    }
}

void Worker::run_part_mc_base(par pr)
{
    int localDataPackSize;
    Part_mc_base mc_engine;
    mc_engine.initialize(mainGui,pr);
    emit systemInitialized();

    QHash<int,QHash<int, double> > dataPack;

    forever
    {
        if(mainGui->stopprocess)
        {
            break;
        }

        if(pr.b!=mainGui->global_beta)
        {
            mc_engine.updateBeta(mainGui->global_beta,pr);
        }

        if(pr.mu!=mainGui->global_mu)
        {
            mc_engine.updateMu(mainGui->global_mu,pr);
        }

        localDataPackSize=mainGui->dataPackSize;
        for(int i=0;i<localDataPackSize;i++)
        {
            dataPack[pr.zkdens[0]][(int)(pr.E+0.5)]+=mc_engine.getCurrentTimeStep();  //dataPack is the data structure storing the short term visit histogram: time[rho][interaction Energy]
            mc_engine.makeMCstep(pr);
        }

        emit newDataPack(dataPack);
        emit sendMuZkDensData(pr.mu,pr.zkdens[0],pr.zkdens[1],pr.zkdens[2],pr.zkdens[3]);

        dataPack.clear();

    }
}

void Worker::run_part_mc_base_y12(par pr)
{
    int localDataPackSize;
    Part_mc_base_y12 mc_engine;
    mc_engine.initialize(mainGui,pr);
    emit systemInitialized();

    QHash<int,QHash<int, double> > dataPack;

    forever
    {
        if(mainGui->stopprocess)
        {
            break;
        }

        if(pr.b!=mainGui->global_beta)
        {
            mc_engine.updateBeta(mainGui->global_beta,pr);
        }

        if(pr.mu!=mainGui->global_mu)
        {
            mc_engine.updateMu(mainGui->global_mu,pr);
        }

        localDataPackSize=mainGui->dataPackSize;
        for(int i=0;i<localDataPackSize;i++)
        {
            dataPack[pr.zkdens[0]][pr.E]+=mc_engine.getCurrentTimeStep();
            mc_engine.makeMCstep(pr);
        }

        emit newDataPack(dataPack);
        emit sendMuZkDensData(pr.mu,pr.zkdens[0],pr.zkdens[1],pr.zkdens[2],pr.zkdens[3]);

        dataPack.clear();

    }
}
