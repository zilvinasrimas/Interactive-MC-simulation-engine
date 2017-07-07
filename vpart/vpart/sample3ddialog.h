#ifndef SAMPLE3DDIALOG_H
#define SAMPLE3DDIALOG_H

#include <QDialog>
#include "worker.h"
#include "dialog.h"
#include <mgl2/qmathgl.h>
#include "mymgldraw.h"
#include "helper.h"

class Dialog;
class Worker;
class Helper;

namespace Ui {
class Sample3DDialog;
}

class Sample3DDialog : public QDialog
{
    Q_OBJECT

public:
    explicit Sample3DDialog(Dialog *mainDialog, Worker *mainworker, Helper *mainHelper, QWidget *parent = 0);
    ~Sample3DDialog();

signals:
    void sample3dUpdateNeeded();

private slots:
    void on_ShowButton_clicked();
    void initializeSample3DDialog();
    void frameReplot();

    void on_up_button_clicked();
    void on_down_button_clicked();
    void on_right_button_clicked();
    void on_left_button_clicked();

    void on_checkBox_clicked(bool checked);

    void on_print_Button_clicked();

private:
    myMGLDraw *matrix_dist;
    Ui::Sample3DDialog *ui;
    QMathGL *mainMGL;
    Dialog *mainGui;
    Worker *guiWorker;
    QTimer replotTimer;
    bool replot=0;
    int print_count;

};

#endif // SAMPLE3DDIALOG_H
