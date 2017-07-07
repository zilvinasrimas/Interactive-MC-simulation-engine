#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTime>
#include "worker.h"
#include "helper.h"
#include "qcustomplot.h"
#include <vector>
#include <mgl2/qmathgl.h>

class Worker;
class Helper;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    explicit Dialog(Worker *mainworker,Helper *muhelper, QWidget *parent = 0);
    ~Dialog();


signals:
    void muChangeNeeded(double newMu);
    void focusChangeNeeded(QCPRange newKeyRange,QCPRange newValueRange);
    void clearRawData();
    void startWorker();
    void requestFreeEnergy();
    void muDensConfigureAerogelAxes();

public slots:
    void on_beta_spinbox_valueChanged(double arg1);
    void on_mu_spinbox_valueChanged(double arg1);
    void on_clear_button_clicked();
    void printMainScreen(int counter);

private slots:
    void on_mu_slider_valueChanged(int value);
    void on_beta_slider_valueChanged(int value);
    void on_stop_button_clicked();
    void on_start_button_clicked();
    void on_sample_lineEdit_textEdited(const QString &arg1);
    void prepareCustomPlot();
    void frameReplot();
    void selectionChanged();
    void keyPressEvent(QKeyEvent *keypressed);
    void on_focus_button_clicked();
    void on_fullView_button_clicked();
    void insertDataPoint(double key, double value, double dt, QCPColorMapData *dat);
    void findLimits();
    void addLimits();
    void addFreeEnergy();
    void updateLimits();
    void configureAerogelAxes();

    void receivingDataPack(QHash<int, QHash<int, double> > receivedDataPack);
    void initializeColorMap();
    void muChangeDone(double newMu);
    void focusChangeDone();
    void freeEnergyCalculated(QVector<double> freeEnDataIndex, QVector<double> freeEnData);

    void on_maxCells_lineEdit_editingFinished();
    void on_dataPackSize_lineEdit_editingFinished();
    void on_print_button_clicked();


private:
    Ui::Dialog *ui;
    Worker *guiWorker;
    Helper *guimuHelper;
    QTimer replotTimer;
    QCPColorMap *densEmap;
    QCPAxisRect *freeEnAxisRect;
    QCPGraph *freeEnGraph;
    QTime fps_time;
    int fps_frameCount;
    QTime pps_time;
    int pps_count;
    int print_count;

public:
    double global_beta,global_beta_past;
    double global_mu;
    double global_N,wettability;
    int dim_x,dim_y,dim_z;
    int maxCells;
    int dataPackSize,replotInterval;
    bool started,stopprocess,printAndClean,stopMuRecalclulation,stopFocusRecalclulation,aerogelOrIsing,drawLimits,loadLimits,clearRawDataNow,saveBoundary;

    QString sample_name,runType;
    QCPColorMapData *mainColorMapData;
    QVector<double> freeEn,freeEnIndex;
    std::vector<double> highTLimit,lowTLimit;
    std::vector<long long> entrpUpLim;
    std::vector<long long> entrpLowLim;
    std::vector< std::vector<long long> > global_cells;
    mglData matrixdat,transdat,mx,my,mz;


};

#endif // DIALOG_H
