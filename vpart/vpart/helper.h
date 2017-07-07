#ifndef HELPER_H
#define HELPER_H

#include <QObject>
#include "dialog.h"
#include "qcustomplot.h"

class Dialog;

class Helper : public QObject
{
    Q_OBJECT
public:
    explicit Helper(QObject *parent = 0);

signals:
    void muRecalculated(double newMu);
    void focusRecalculated();
    void freeEnergyRecalculated(QVector<double> newfreeEnergyIndex,QVector<double> newfreeEnergy);

public slots:
    void secondary_constructor(Dialog *mainDialog);
    void initializeRawData();
    void receivingDataPack(QHash<int, QHash<int, double> > receivedDataPack);
    void rawDataClearing();
    void muChanged(double newMu);
    void focusChanged(QCPRange newKeyRange,QCPRange newValueRange);
    void insertDataPoint(double key, double value, double dt, QCPColorMapData *dat);
    void sample3dUpdate();
    void freeEnergyNeeded();

private:
    Dialog *mainGui;
    QVector< QHash <int,double> > rawData;
    QVector<double> rawFreeEnData;
    double rawTimeSum;
};

#endif // HELPER_H
