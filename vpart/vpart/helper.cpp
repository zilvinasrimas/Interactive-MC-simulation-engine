#include "helper.h"

Helper::Helper(QObject *parent) : QObject(parent)
{

}

void Helper::secondary_constructor(Dialog *mainDialog)
{
    mainGui=mainDialog;
}

void Helper::initializeRawData()
{
    rawData.resize(mainGui->global_N+1);
    rawFreeEnData.resize(mainGui->global_N+1);
    rawTimeSum=0;
}

void Helper::receivingDataPack(QHash<int, QHash<int, double> > receivedDataPack)
{
    QHashIterator<int, QHash<int, double> > iDPoverDens(receivedDataPack);
    while (iDPoverDens.hasNext())
    {
        iDPoverDens.next();
        QHashIterator<int, double> iDPoverE(iDPoverDens.value());
        while (iDPoverE.hasNext())
        {
            iDPoverE.next();

            rawData[iDPoverDens.key()][iDPoverE.key()]+=iDPoverE.value();
            rawFreeEnData[iDPoverDens.key()]+=iDPoverE.value();
            rawTimeSum+=iDPoverE.value();
        }      
    }
}


void Helper::freeEnergyNeeded()
{
    QVector<double> newFreeEnergy,newFreeEnergyIndex;
    for(int i=0;i<rawFreeEnData.size();i++)
    {
        double tempFE=rawFreeEnData[i]/rawTimeSum;
        if(tempFE>0.000001)
        {
            newFreeEnergy.push_back(-log(rawFreeEnData[i]));
            newFreeEnergyIndex.push_back(i);
        }
    }
    mainGui->freeEn=newFreeEnergy;
    mainGui->freeEnIndex=newFreeEnergyIndex;
    emit freeEnergyRecalculated(newFreeEnergyIndex,newFreeEnergy);

}

void Helper::muChanged(double newMu)
{
    mainGui->stopMuRecalclulation=false;
    mainGui->mainColorMapData->fill(0.0);

    for(int i=((int)floor(mainGui->mainColorMapData->keyRange().lower)>0?((int)floor(mainGui->mainColorMapData->keyRange().lower)):(0));
        i<((int)ceil(mainGui->mainColorMapData->keyRange().upper)<mainGui->global_N+1?((int)floor(mainGui->mainColorMapData->keyRange().upper)):(mainGui->global_N+1));
        i++)
    {
        if(mainGui->stopMuRecalclulation) return;

        QHashIterator<int, double> iRDoverE(rawData[i]);
        while (iRDoverE.hasNext())
        {
            iRDoverE.next();
            insertDataPoint(i, -iRDoverE.key()-newMu*i, iRDoverE.value(), mainGui->mainColorMapData);
        }
    }

    if(mainGui->stopMuRecalclulation) return;
    emit muRecalculated(newMu);
}

void Helper::focusChanged(QCPRange newKeyRange,QCPRange newValueRange)
{
    mainGui->stopFocusRecalclulation=false;
    mainGui->mainColorMapData->fill(0.0);
    mainGui->mainColorMapData->setRange(newKeyRange,newValueRange);

    if(newKeyRange.size()>(int)(sqrt(mainGui->maxCells*newKeyRange.size()/newValueRange.size())))
    {
        mainGui->mainColorMapData->setSize((int)(sqrt(mainGui->maxCells*newKeyRange.size()/newValueRange.size())),(int)(sqrt(mainGui->maxCells*newValueRange.size()/newKeyRange.size())));
    }
    else
    {
        mainGui->mainColorMapData->setSize((int)(newKeyRange.size()),(int)(newValueRange.size()));
    }

    for(int i=((int)floor(newKeyRange.lower)>0?((int)floor(newKeyRange.lower)):(0));
        i<((int)ceil(newKeyRange.upper)<mainGui->global_N+1?((int)floor(newKeyRange.upper)):(mainGui->global_N+1));
        i++)
    {
        if(mainGui->stopFocusRecalclulation) return;

        QHashIterator<int, double> iRDoverE(rawData[i]);
        while (iRDoverE.hasNext())
        {
            iRDoverE.next();
            insertDataPoint(i, -iRDoverE.key()-mainGui->global_mu*i, iRDoverE.value(), mainGui->mainColorMapData);
        }
    }
    emit focusRecalculated();
}

void Helper::insertDataPoint(double key, double value, double dt, QCPColorMapData *dat)
{
    int klkey=((key-0.5-dat->keyRange().lower)/dat->keyRange().size()*(dat->keySize()-1)+0.5),
        klvalue=((value-0.5-dat->valueRange().lower)/dat->valueRange().size()*(dat->valueSize()-1)+0.5),
        kukey=((key+0.5-dat->keyRange().lower)/dat->keyRange().size()*(dat->keySize()-1)+0.5),
        kuvalue=((value+0.5-dat->valueRange().lower)/dat->valueRange().size()*(dat->valueSize()-1)+0.5);

    if(kukey==klkey)
    {
        if(kuvalue==klvalue)
        {
            dat->setCell(klkey,klvalue,dat->cell(klkey,klvalue)+dt);
        }
        else
        {
            double Puvalue=value+0.5-dat->valueRange().lower+(0.5-kuvalue)*dat->valueRange().size()/(dat->valueSize()-1);
            dat->setCell(klkey,klvalue,dat->cell(klkey,klvalue)+(1-Puvalue)*dt);
            dat->setCell(klkey,kuvalue,dat->cell(klkey,kuvalue)+Puvalue*dt);
        }
    }
    else
    {
        if(kuvalue==klvalue)
        {
            double Pukey=key+0.5-dat->keyRange().lower+(0.5-kukey)*dat->keyRange().size()/(dat->keySize()-1);
            dat->setCell(kukey,klvalue,dat->cell(kukey,klvalue)+Pukey*dt);
            dat->setCell(klkey,klvalue,dat->cell(klkey,klvalue)+(1-Pukey)*dt);
        }
        else
        {
            double Pukey=key+0.5-dat->keyRange().lower+(0.5-kukey)*dat->keyRange().size()/(dat->keySize()-1),
                   Puvalue=value+0.5-dat->valueRange().lower+(0.5-kuvalue)*dat->valueRange().size()/(dat->valueSize()-1);

            dat->setCell(klkey,klvalue,dat->cell(klkey,klvalue)+(1-Pukey)*(1-Puvalue)*dt);
            dat->setCell(klkey,kuvalue,dat->cell(klkey,kuvalue)+(1-Pukey)*Puvalue*dt);
            dat->setCell(kukey,klvalue,dat->cell(kukey,klvalue)+Pukey*(1-Puvalue)*dt);
            dat->setCell(kukey,kuvalue,dat->cell(kukey,kuvalue)+Pukey*Puvalue*dt);
        }
    }
}

void Helper::rawDataClearing()
{
    rawData.clear();
    rawData.resize(mainGui->global_N+1);
    rawFreeEnData.clear();
    rawFreeEnData.resize(mainGui->global_N+1);
    rawTimeSum=0;
}

void Helper::sample3dUpdate()
{
    for(unsigned int i=0;i<mainGui->global_cells.size();i++)
    {
        if(mainGui->global_cells[i][0]%2==0)
        {
            mainGui->matrixdat.a[mainGui->global_cells[i][0]/2+mainGui->matrixdat.nx*(mainGui->global_cells[i][1]/2+mainGui->matrixdat.ny*mainGui->global_cells[i][2]/2)] = 2*mainGui->global_cells[i][3];
            mainGui->transdat.a[mainGui->global_cells[i][0]/2+mainGui->matrixdat.nx*(mainGui->global_cells[i][1]/2+mainGui->matrixdat.ny*mainGui->global_cells[i][2]/2)] = 8.0/(mainGui->dim_z+1.0)*(double)(mainGui->global_cells[i][2])/2*mainGui->global_cells[i][3];

        }
    }
}

