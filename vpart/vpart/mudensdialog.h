#ifndef MUDENSDIALOG_H
#define MUDENSDIALOG_H

#include <QDialog>
#include "worker.h"
#include "qcustomplot.h"
#include "dialog.h"
#include<QList>

class Dialog;
class Worker;

namespace Ui {
class MuDensDialog;
}

class MuDensDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MuDensDialog(Dialog *mainDialog, Worker *mainworker, Helper *muhelper, QWidget *parent = 0);
    ~MuDensDialog();


private slots:
    void prepareCustomPlot();
    void selectionChanged();
    void frameReplot();

    void receivingMuDensData(double newMu, double newDens);
    void receivingMuZkDensData(double newMu,double zkDens0,double zkDens1,double zkDens2,double zkDens3);
    void initializeMuDensGraph();
    void configureAerogelAxes();


    void on_clear_button_clicked();

    void on_rb_blue_clicked();
    void on_rb_red_clicked();
    void on_rb_green_clicked();
    void on_rb_black_clicked();


    void on_printButton_clicked();

private:
    Ui::MuDensDialog *ui;
    Dialog *mainGui;
    Worker *guiWorker;
    Helper *muHelper;
    QTimer replotTimer;
    QCPGraph *muDensGraph;
    QCPGraph *muDensGraph0;
    QCPGraph *muDensGraph1;
    QCPGraph *muDensGraph2;
    QCPGraph *muDensGraph3;
    QList<QCPGraph*> graphlist;
    int print_count;
};

#endif // MUDENSDIALOG_H
