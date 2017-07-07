#include "mudensdialog.h"
#include "ui_mudensdialog.h"
#include "qcustomplot.h"

MuDensDialog::MuDensDialog(Dialog *mainDialog, Worker *mainworker,Helper *muhelper, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MuDensDialog)
{
    ui->setupUi(this);
    muHelper=muhelper;
    guiWorker=mainworker;
    mainGui=mainDialog;
    this->setGeometry(0,0,this->width(),this->height());
    ui->rb_blue->setStyleSheet("background-color: rgb(150, 150, 255)");
    ui->rb_red->setStyleSheet("background-color: rgb(255, 150, 150)");
    ui->rb_green->setStyleSheet("background-color: rgb(150, 255, 150)");
    ui->rb_black->setStyleSheet("background-color: rgb(150, 150, 150)");


    connect(mainworker, SIGNAL(sendMuDensData(double,double)), this, SLOT(receivingMuDensData(double,double)));
    connect(mainworker, SIGNAL(sendMuZkDensData(double,double,double,double,double)), this, SLOT(receivingMuZkDensData(double,double,double,double,double)));
    connect(mainworker, SIGNAL(systemInitialized()), this, SLOT(initializeMuDensGraph()));
    connect(&replotTimer, SIGNAL(timeout()), this, SLOT(frameReplot()));
    connect(mainGui, SIGNAL(muDensConfigureAerogelAxes()), this, SLOT(configureAerogelAxes()));

    prepareCustomPlot();
    print_count=0;
    replotTimer.start(mainGui->replotInterval);
}

MuDensDialog::~MuDensDialog()
{
    delete ui;
}

void MuDensDialog::prepareCustomPlot()
{
    ui->mainData->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |QCP::iSelectAxes);
    ui->mainData->xAxis->setSelectableParts(QCPAxis::spAxis);
    ui->mainData->yAxis->setSelectableParts(QCPAxis::spAxis);
    connect(ui->mainData, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

    if(mainGui->aerogelOrIsing)
    {
        ui->mainData->yAxis->setLabel("𝜚");
        ui->mainData->xAxis->setLabel("𝜇");
    }
    else
    {
        ui->mainData->yAxis->setLabel("M");
        ui->mainData->xAxis->setLabel("H");
    }
    QFont myFont("Helvetica", 22);
    myFont.setLetterSpacing(QFont::PercentageSpacing,120);
    ui->mainData->xAxis->setLabelFont(myFont);
    ui->mainData->yAxis->setLabelFont(myFont);
}

void MuDensDialog::selectionChanged()
{
    if (ui->mainData->selectedAxes().isEmpty())
    {
        ui->mainData->axisRect()->setRangeZoomFactor(0.75,0.75);
    }
    else if(ui->mainData->selectedAxes().first()==ui->mainData->xAxis)
    {
        ui->mainData->axisRect()->setRangeZoomFactor(0.75,0.0);
    }
    else if(ui->mainData->selectedAxes().first()==ui->mainData->yAxis)
    {
        ui->mainData->axisRect()->setRangeZoomFactor(0.0,0.75);
    }
}

void MuDensDialog::receivingMuDensData(double newMu, double newDens)
{
    muDensGraph->addData(newMu,newDens);
}

void MuDensDialog::receivingMuZkDensData(double newMu,double zkDens0,double zkDens1,double zkDens2,double zkDens3)
{
    muDensGraph0->addData(newMu,zkDens0);
    muDensGraph1->addData(newMu,zkDens1);
    muDensGraph2->addData(newMu,zkDens2);
    muDensGraph3->addData(newMu,zkDens3);
}

void MuDensDialog::initializeMuDensGraph()
{
    ui->mainData->yAxis->setRange((QCPRange(0, mainGui->global_N)));
    ui->mainData->xAxis->setRange((QCPRange(-10, 0)));

    ui->mainData->yAxis->setAutoTicks(0);
    ui->mainData->yAxis->setAutoTickLabels(0);
    if(mainGui->aerogelOrIsing)
    {
        QVector<double> YtickVector(6); for(int i=0;i<YtickVector.size();i++) YtickVector[i]=i*double(mainGui->global_N)/5;  //aerogel Y ticks
        QVector<QString> YtickVectorLabels(6);for(int i=0;i<YtickVectorLabels.size();i++) YtickVectorLabels[i]=QString::number(double(i)/5);
        ui->mainData->yAxis->setTickVector(YtickVector);
        ui->mainData->yAxis->setTickVectorLabels(YtickVectorLabels);
    }
    else
    {
        QVector<double> YtickVector(5); for(int i=0;i<YtickVector.size();i++) YtickVector[i]=i*double(mainGui->global_N)/4;     //ising Y ticks
        QVector<QString> YtickVectorLabels(5);for(int i=0;i<YtickVectorLabels.size();i++) YtickVectorLabels[i]=QString::number(double(i-2)/2);
        ui->mainData->yAxis->setTickVector(YtickVector);
        ui->mainData->yAxis->setTickVectorLabels(YtickVectorLabels);
    }

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph0=ui->mainData->graph(); graphlist.append(ui->mainData->graph());
    muDensGraph0->setPen(QPen(Qt::blue));
    muDensGraph0->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk0->isChecked()) muDensGraph0->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    else muDensGraph0->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 3));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph1=ui->mainData->graph(); graphlist.append(ui->mainData->graph());
    muDensGraph1->setPen(QPen(Qt::blue));
    muDensGraph1->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk1->isChecked()) muDensGraph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    else muDensGraph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph2=ui->mainData->graph(); graphlist.append(ui->mainData->graph());
    muDensGraph2->setPen(QPen(Qt::blue));
    muDensGraph2->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk2->isChecked()) muDensGraph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 5));
    else muDensGraph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph3=ui->mainData->graph(); graphlist.append(ui->mainData->graph());
    muDensGraph3->setPen(QPen(Qt::blue));
    muDensGraph3->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk3->isChecked()) muDensGraph3->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 5));
    else muDensGraph3->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));
}

void MuDensDialog::frameReplot()
{
    ui->mainData->replot();
}

void MuDensDialog::on_clear_button_clicked()
{
    if(true)
    {
        if(graphlist.size()>4)
            for(int i=0;i<4;i++)
            {
                ui->mainData->removeGraph(graphlist[0]);
                graphlist.removeFirst();
            }
    }
    else {ui->mainData->removeGraph(graphlist[0]);graphlist.removeFirst();} //legacy : no additional layer and whole zbl structure

}

void MuDensDialog::on_rb_blue_clicked()
{
    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph0=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph0->setPen(QPen(Qt::blue));
    muDensGraph0->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk0->isChecked()) muDensGraph0->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    else muDensGraph0->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 3));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph1=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph1->setPen(QPen(Qt::blue));
    muDensGraph1->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk1->isChecked()) muDensGraph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    else muDensGraph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph2=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph2->setPen(QPen(Qt::blue));
    muDensGraph2->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk2->isChecked()) muDensGraph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 5));
    else muDensGraph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph3=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph3->setPen(QPen(Qt::blue));
    muDensGraph3->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk3->isChecked()) muDensGraph3->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 5));
    else muDensGraph3->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));
}

void MuDensDialog::on_rb_red_clicked()
{
    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph0=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph0->setPen(QPen(Qt::red));
    muDensGraph0->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk0->isChecked()) muDensGraph0->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    else muDensGraph0->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 3));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph1=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph1->setPen(QPen(Qt::red));
    muDensGraph1->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk1->isChecked()) muDensGraph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    else muDensGraph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph2=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph2->setPen(QPen(Qt::red));
    muDensGraph2->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk2->isChecked()) muDensGraph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 5));
    else muDensGraph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph3=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph3->setPen(QPen(Qt::red));
    muDensGraph3->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk3->isChecked()) muDensGraph3->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 5));
    else muDensGraph3->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));
}

void MuDensDialog::on_rb_green_clicked()
{
    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph0=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph0->setPen(QPen(Qt::green));
    muDensGraph0->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk0->isChecked()) muDensGraph0->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    else muDensGraph0->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 3));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph1=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph1->setPen(QPen(Qt::green));
    muDensGraph1->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk1->isChecked()) muDensGraph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    else muDensGraph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph2=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph2->setPen(QPen(Qt::green));
    muDensGraph2->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk2->isChecked()) muDensGraph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 5));
    else muDensGraph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph3=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph3->setPen(QPen(Qt::green));
    muDensGraph3->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk3->isChecked()) muDensGraph3->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 5));
    else muDensGraph3->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));
}

void MuDensDialog::on_rb_black_clicked()
{
    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph0=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph0->setPen(QPen(Qt::black));
    muDensGraph0->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk0->isChecked()) muDensGraph0->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    else muDensGraph0->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 3));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph1=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph1->setPen(QPen(Qt::black));
    muDensGraph1->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk1->isChecked()) muDensGraph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    else muDensGraph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph2=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph2->setPen(QPen(Qt::black));
    muDensGraph2->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk2->isChecked()) muDensGraph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle, 5));
    else muDensGraph2->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));

    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    muDensGraph3=ui->mainData->graph();graphlist.append(ui->mainData->graph());
    muDensGraph3->setPen(QPen(Qt::black));
    muDensGraph3->setLineStyle(QCPGraph::lsNone);
    if(ui->checkBox_zk3->isChecked()) muDensGraph3->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 5));
    else muDensGraph3->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, 5));
}

void MuDensDialog::on_printButton_clicked()
{
    print_count++;
    QString fileName = "../out/print/mudensGraph_"+QString::number(print_count)+"_b_"+QString::number(mainGui->global_beta)+"_mu_"+QString::number(mainGui->global_mu)+".png";
    ui->mainData->savePng(fileName,0,0,1.0,-1);
}

void MuDensDialog::configureAerogelAxes()
{
    if(mainGui->aerogelOrIsing)
    {
        ui->mainData->yAxis->setLabel("𝜚");
        ui->mainData->xAxis->setLabel("𝜇");
    }
    else
    {
        ui->mainData->yAxis->setLabel("M");
        ui->mainData->xAxis->setLabel("H");
    }
}
