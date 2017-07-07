#include "sample3ddialog.h"
#include "ui_sample3ddialog.h"
#include "mymgldraw.h"
#include <mgl2/qt.h>

Sample3DDialog::Sample3DDialog(Dialog *mainDialog, Worker *mainworker, Helper *mainHelper, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Sample3DDialog)
{
    ui->setupUi(this);
    this->setGeometry(1200,0,this->width(),this->height());
    mainGui=mainDialog;
    guiWorker=mainworker;

    connect(mainworker, SIGNAL(systemInitialized()), this, SLOT(initializeSample3DDialog()));
    connect(this, SIGNAL(sample3dUpdateNeeded()), mainHelper, SLOT(sample3dUpdate()));
    connect(&replotTimer, SIGNAL(timeout()), this, SLOT(frameReplot()));

    mainMGL = new QMathGL(ui->MathGLwidget);


    QScrollArea *scrollArea = new QScrollArea(ui->MathGLwidget);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(0);
    scrollArea->setWidget(mainMGL);
    QVBoxLayout *layout = new QVBoxLayout(ui->MathGLwidget);
    layout->addWidget(scrollArea);
    ui->MathGLwidget->setLayout(layout);

    print_count=0;
}

Sample3DDialog::~Sample3DDialog()
{
    delete ui;
}

void Sample3DDialog::initializeSample3DDialog()
{
    matrix_dist=new myMGLDraw(mainGui);

    QThread *drawThread = new QThread;
    matrix_dist->moveToThread(drawThread);
    drawThread->start();

    mainMGL->setDraw(matrix_dist);
    mainMGL->setRotate(1);
    mainMGL->update();
    replotTimer.start(1000);
}

void Sample3DDialog::on_ShowButton_clicked()
{
    mainMGL->update();
}

void Sample3DDialog::frameReplot()
{
    if(replot) mainMGL->update();
    emit sample3dUpdateNeeded();
}

void Sample3DDialog::on_left_button_clicked()
{
    mainMGL->setPhi(mainMGL->getPhi()-1);
}
void Sample3DDialog::on_right_button_clicked()
{
    mainMGL->setPhi(mainMGL->getPhi()+1);
}
void Sample3DDialog::on_up_button_clicked()
{
    mainMGL->setTet(mainMGL->getTet()+1);
}
void Sample3DDialog::on_down_button_clicked()
{
    mainMGL->setTet(mainMGL->getTet()-1);
}
void Sample3DDialog::on_checkBox_clicked(bool checked)
{
    replot=checked;
}

void Sample3DDialog::on_print_Button_clicked()
{
     print_count++;
     QString fileName = "../out/print/plot_"+QString::number(print_count)+"_b_"+QString::number(mainGui->global_beta)+"_mu_"+QString::number(mainGui->global_mu)+".png";
     mainMGL->exportPNG(fileName);
     on_ShowButton_clicked();
}
