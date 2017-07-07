#include "dialog.h"
#include "ui_dialog.h"
#include <QFileInfo>


Dialog::Dialog(Worker *mainworker, Helper *muhelper, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    guiWorker=mainworker;
    guimuHelper=muhelper;
    global_mu=ui->mu_spinbox->value();
    global_beta=ui->beta_spinbox->value();
    sample_name=ui->sample_lineEdit->text();
    maxCells=ui->maxCells_lineEdit->text().toInt();
    dataPackSize=ui->dataPackSize_lineEdit->text().toInt();
    stopprocess=false;
    stopMuRecalclulation=false;
    stopFocusRecalclulation=false;
    started=false;
    fps_frameCount=0;
    printAndClean=0;
    print_count=0;
    aerogelOrIsing=0;
    saveBoundary=0;
    replotInterval=20;
    drawLimits=0;
    loadLimits=0;

    connect(this, SIGNAL(startWorker()), mainworker, SLOT(on_start_button_clicked()));
    connect(mainworker, SIGNAL(newDataPack(QHash<int, QHash<int, double> >)), this, SLOT(receivingDataPack(QHash<int, QHash<int, double> >)));
    connect(mainworker, SIGNAL(newDataPack(QHash<int, QHash<int, double> >)), muhelper, SLOT(receivingDataPack(QHash<int, QHash<int, double> >)));
    connect(this, SIGNAL(muChangeNeeded(double)), muhelper, SLOT(muChanged(double)));
    connect(this, SIGNAL(focusChangeNeeded(QCPRange,QCPRange )), muhelper, SLOT(focusChanged(QCPRange,QCPRange )));
    connect(this, SIGNAL(clearRawData()), muhelper, SLOT(rawDataClearing()));
    connect(muhelper, SIGNAL(muRecalculated(double)), this, SLOT(muChangeDone(double)));
    connect(muhelper, SIGNAL(focusRecalculated()), this, SLOT(focusChangeDone()));
    connect(this, SIGNAL(requestFreeEnergy()), muhelper, SLOT(freeEnergyNeeded()));
    connect(muhelper, SIGNAL(freeEnergyRecalculated(QVector<double>,QVector<double>)), this, SLOT(freeEnergyCalculated(QVector<double>,QVector<double>)));

    connect(mainworker, SIGNAL(systemInitialized()), this, SLOT(initializeColorMap()));
    connect(mainworker, SIGNAL(systemInitialized()), muhelper, SLOT(initializeRawData()));
    connect(&replotTimer, SIGNAL(timeout()), this, SLOT(frameReplot()));
    prepareCustomPlot();

    replotTimer.start(replotInterval);
    fps_time.start();
    pps_time.start();

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::prepareCustomPlot()
{
    ui->mainData->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |QCP::iSelectAxes);
    ui->mainData->xAxis->setSelectableParts(QCPAxis::spAxis);
    ui->mainData->yAxis->setSelectableParts(QCPAxis::spAxis);
    ui->mainData->axisRect()->setBackground(Qt::transparent);
    connect(ui->mainData, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));



    densEmap = new QCPColorMap(ui->mainData->xAxis, ui->mainData->yAxis);
    ui->mainData->addPlottable(densEmap);

    densEmap->setInterpolate(false);
    densEmap->setDataScaleType(QCPAxis::stLogarithmic);

    QCPColorGradient colGrad;
    colGrad.loadPreset(QCPColorGradient::gpPolar);
    colGrad.colorStops().remove(0);
    colGrad.setColorStopAt(0,Qt::white);
    colGrad.setColorStopAt(0.0001,Qt::white);
    colGrad.setColorStopAt(0.00011,QColor(255, 0.196078*255, 255, 255));
    densEmap->setGradient(colGrad);

    freeEnAxisRect = new QCPAxisRect(ui->mainData);
    freeEnAxisRect->axis(QCPAxis::atBottom)->setSelectableParts(QCPAxis::spNone);
    freeEnAxisRect->axis(QCPAxis::atLeft)->setSelectableParts(QCPAxis::spNone);
    freeEnAxisRect->setBackground(Qt::transparent);
    ui->mainData->plotLayout()->addElement(1, 0, freeEnAxisRect);
    freeEnAxisRect->setMaximumSize(QSize(QWIDGETSIZE_MAX, 250));
    freeEnAxisRect->axis(QCPAxis::atBottom)->setLayer("axes");
    freeEnAxisRect->axis(QCPAxis::atBottom)->grid()->setLayer("grid");

    ui->mainData->plotLayout()->setRowSpacing(0);
    freeEnAxisRect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msBottom);
    freeEnAxisRect->setMargins(QMargins(0, 0, 0, 0));
    connect(ui->mainData->xAxis, SIGNAL(rangeChanged(QCPRange)), freeEnAxisRect->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));
    connect(freeEnAxisRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), ui->mainData->xAxis, SLOT(setRange(QCPRange)));

    ui->mainData->yAxis->setLabel("𝓗");//manually draw tilda on top or I as a subscript for lattice-gas and Ising models respectively
    QFont myFont("Helvetica", 40);
    QFont myFontTicks("Helvetica", 28);
    myFont.setLetterSpacing(QFont::PercentageSpacing,120);
    ui->mainData->yAxis->setLabelFont(myFont);
    if(aerogelOrIsing)
    {
        freeEnAxisRect->axis(QCPAxis::atBottom)->setLabel("𝜚");
        freeEnAxisRect->axis(QCPAxis::atLeft)->setLabel("𝜔");
    }
    else
    {
        freeEnAxisRect->axis(QCPAxis::atBottom)->setLabel("M");
        freeEnAxisRect->axis(QCPAxis::atLeft)->setLabel("ƒ");
    }

    freeEnAxisRect->axis(QCPAxis::atBottom)->setLabelFont(myFont);
    freeEnAxisRect->axis(QCPAxis::atLeft)->setLabelFont(myFont);
    freeEnAxisRect->axis(QCPAxis::atBottom)->setTickLabelFont(myFontTicks);
    freeEnAxisRect->axis(QCPAxis::atLeft)->setTickLabelFont(myFontTicks);
    ui->mainData->yAxis->setTickLabelFont(myFontTicks);
    freeEnAxisRect->axis(QCPAxis::atLeft)->grid()->setVisible(0);
    freeEnAxisRect->axis(QCPAxis::atBottom)->grid()->setVisible(0);
    ui->mainData->xAxis->setBasePen(Qt::NoPen);
    ui->mainData->xAxis->setTickLabels(false);
    ui->mainData->xAxis->setTicks(false);
    freeEnAxisRect->axis(QCPAxis::atLeft)->setAutoTickStep(0);
    freeEnAxisRect->axis(QCPAxis::atLeft)->setTickStep(5.0);
    freeEnAxisRect->axis(QCPAxis::atLeft)->setRangeLower(-9.5);
    freeEnAxisRect->axis(QCPAxis::atLeft)->setRangeUpper(1.0);
    freeEnAxisRect->axis(QCPAxis::atLeft)->setLabelPadding(35);
    ui->mainData->xAxis->grid()->setVisible(0);
    ui->mainData->yAxis->grid()->setVisible(0);
    ui->mainData->rescaleAxes();

    QCPMarginGroup *group = new QCPMarginGroup(ui->mainData);
    ui->mainData->axisRect()->setMarginGroup(QCP::msLeft|QCP::msRight, group);
    freeEnAxisRect->setMarginGroup(QCP::msLeft|QCP::msRight, group);
}

void Dialog::initializeColorMap()
{
    densEmap->data()->setRange(QCPRange(0, global_N), QCPRange(-global_N/2, global_N/2));
    if(densEmap->data()->keyRange().size()>(int)(sqrt(maxCells*densEmap->data()->keyRange().size()/densEmap->data()->valueRange().size())))
    {
        densEmap->data()->setSize((int)(sqrt(maxCells*densEmap->data()->keyRange().size()/densEmap->data()->valueRange().size())),(int)(sqrt(maxCells*densEmap->data()->valueRange().size()/densEmap->data()->keyRange().size())));
    }
    else
    {
        densEmap->data()->setSize((int)(densEmap->data()->keyRange().size()),(int)(densEmap->data()->valueRange().size()));
    }

    mainColorMapData = new QCPColorMapData(densEmap->data()->keySize(),densEmap->data()->valueSize(),densEmap->data()->keyRange(),densEmap->data()->valueRange());
    densEmap->rescaleAxes(true);


    freeEnAxisRect->axis(QCPAxis::atBottom)->setAutoTicks(0);
    freeEnAxisRect->axis(QCPAxis::atBottom)->setAutoTickLabels(0);
    if(aerogelOrIsing)
    {
        QVector<double> XtickVector(6); for(int i=0;i<XtickVector.size();i++) XtickVector[i]=i*double(global_N)/5;  //aerogel X ticks
        QVector<QString> XtickVectorLabels(6);for(int i=0;i<XtickVectorLabels.size();i++) XtickVectorLabels[i]=QString::number(double(i)/5);
        freeEnAxisRect->axis(QCPAxis::atBottom)->setTickVector(XtickVector);
        freeEnAxisRect->axis(QCPAxis::atBottom)->setTickVectorLabels(XtickVectorLabels);
    }
    else
    {
        QVector<double> XtickVector(5); for(int i=0;i<XtickVector.size();i++) XtickVector[i]=i*double(global_N)/4;     //ising X ticks
        QVector<QString> XtickVectorLabels(5);for(int i=0;i<XtickVectorLabels.size();i++) XtickVectorLabels[i]=QString::number(double(i-2)/2);
        freeEnAxisRect->axis(QCPAxis::atBottom)->setTickVector(XtickVector);
        freeEnAxisRect->axis(QCPAxis::atBottom)->setTickVectorLabels(XtickVectorLabels);
    }

    ui->mainData->yAxis->setAutoTicks(0);
    ui->mainData->yAxis->setAutoTickLabels(0);
    QVector<double> YtickVector(21); for(int i=0;i<YtickVector.size();i++) YtickVector[i]=(i-10)*double(global_N)/5;     //Y ticks
    QVector<QString> YtickVectorLabels(21);for(int i=0;i<YtickVectorLabels.size();i++) YtickVectorLabels[i]=QString::number(double(i-10)/5);
    ui->mainData->yAxis->setTickVector(YtickVector);
    ui->mainData->yAxis->setTickVectorLabels(YtickVectorLabels);

    ui->mainData->yAxis->setRange(-2.1*global_N,0*global_N);

    addFreeEnergy();

    if(loadLimits)
    {
        findLimits();
    }
    if(drawLimits)
    {
        findLimits();
        addLimits();
    }

    started=true;
}

void Dialog::frameReplot()
{
    densEmap->rescaleDataRange(false);

    ui->mainData->replot();

    if(fps_time.elapsed()>2000)
    {
        ui->fps_lcd->display(int((double(fps_frameCount)/double(fps_time.elapsed())*1000)));
        fps_time.start();
        fps_frameCount=0;

        if(started) emit requestFreeEnergy();
    }
    fps_frameCount++;
}

void Dialog::freeEnergyCalculated(QVector<double> freeEnDataIndex,QVector<double> freeEnData)
{
    ui->mainData->graph(0)->setData(freeEnDataIndex,freeEnData);  
}

void Dialog::printMainScreen(int counter)
{
    QString dirName = "../out/"+sample_name+"_mu_"+QString::number(global_mu);
    QDir(".").mkdir(dirName);
    QString fileName = "../out/"+sample_name+"_mu_"+QString::number(global_mu)+"/graph_"+QString::number(counter)+"_b_"+QString::number(global_beta)+"_mu_"+QString::number(global_mu)+".png";
    ui->mainData->savePng(fileName,0,0,1.0,-1);


    QDir(".").mkdir("../out/vycor_output");
    QFile potSurffile("../out/vycor_output/potSurf_"+QString::number(counter)+"_b_"+QString::number(global_beta)+"_mu_"+QString::number(global_mu)+".dat");
    potSurffile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream out(&potSurffile);

    out<<global_N<<"    "<<global_beta<<"   "<<global_mu<<endl;
    for(long i=0;i<freeEn.size();i++)
    {
        out<<freeEnIndex[i]<<"   "<<freeEnIndex[i]/global_N<<"   "<<freeEn[i]<<endl;
    }
    potSurffile.close();

}

void Dialog::receivingDataPack(QHash<int, QHash<int, double> > receivedDataPack)
{
    QHashIterator<int, QHash<int, double> > iDPoverDens(receivedDataPack);
    while (iDPoverDens.hasNext())
    {
        iDPoverDens.next();
        QHashIterator<int, double> iDPoverE(iDPoverDens.value());
        while (iDPoverE.hasNext())
        {
            iDPoverE.next();

            insertDataPoint(iDPoverDens.key(), -iDPoverE.key()-global_mu*iDPoverDens.key(), iDPoverE.value(), densEmap->data());
    //        qDebug()<<densEmap->dataRange().lower<<"  "<<densEmap->dataRange().upper<<"  "<<iDPoverE.value()<<"   "<<densEmap->data()->data(iDPoverDens.key(), -iDPoverE.key()-global_mu*iDPoverDens.key());
//          qDebug() <<"size " <<receivedDataPack.size()<<"dens= " << iDPoverDens.key() <<"E= " << iDPoverE.key() << "dt= "<< iDPoverE.value();
        }
    }

    if(pps_time.elapsed()>2000)
    {
        ui->pps_lcd->display(int((double(pps_count)/double(pps_time.elapsed())*1000)*dataPackSize));
        pps_time.start();
        pps_count=0;
    }
    pps_count++;
}

void Dialog::insertDataPoint(double key, double value, double dt, QCPColorMapData *dat)
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

void Dialog::on_mu_slider_valueChanged(int value)
{
    ui->mu_spinbox->setValue(-10+double(value)/10000);
}

void Dialog::on_beta_slider_valueChanged(int value)
{
    ui->beta_spinbox->setValue(double(value)/1000);
}

void Dialog::on_mu_spinbox_valueChanged(double arg1)
{
    ui->mu_slider->setValue((arg1+10)*10000);
    global_mu=ui->mu_spinbox->value();
    stopMuRecalclulation=true;
    if(started)
    {
        emit muChangeNeeded(global_mu);
        if(drawLimits) updateLimits();
    }

}

void Dialog::muChangeDone(double newMu)
{
    if(newMu==global_mu)
    {
        densEmap->data()->operator =(*mainColorMapData);
        ui->mainData->replot();
    }
}

void Dialog::on_beta_spinbox_valueChanged(double arg1)
{
    ui->beta_slider->setValue(arg1*1000);
    global_beta=ui->beta_spinbox->value();
}

void Dialog::on_stop_button_clicked()
{
    stopprocess=true;
    ui->pps_lcd->display(0);
}

void Dialog::on_start_button_clicked()
{
    runType=ui->runType_comboBox->currentText();
    drawLimits=ui->drawLimits_checkBox->isChecked();
    aerogelOrIsing=ui->aerogelOrIsing_checkBox->isChecked();
    wettability=ui->y_lineEdit->text().toDouble();
    configureAerogelAxes();

    if((!started) or (stopprocess))
    {
        stopprocess=false;
        emit startWorker();
    }
}

void Dialog::on_clear_button_clicked()
{
    densEmap->data()->fill(0.0);
    emit clearRawData();
    densEmap->rescaleDataRange(true);
    ui->mainData->replot();
}

void Dialog::on_sample_lineEdit_textEdited(const QString &arg1)
{
    QFileInfo checkFile("../in/"+arg1+".dat");
    if(checkFile.exists())
    {
        sample_name=arg1;
        ui->sample_lineEdit->setStyleSheet("QLineEdit{background: rgb(170, 255, 170);}");
    }
    else
    {
        ui->sample_lineEdit->setStyleSheet("QLineEdit{background: rgb(255, 170, 170);}");
    }
}

void Dialog::selectionChanged()
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

void Dialog::keyPressEvent(QKeyEvent *keypressed)
{
    if(keypressed->text() == "c") on_clear_button_clicked();
    if(keypressed->text() == ",") ui->mu_spinbox->setValue(ui->mu_spinbox->value()-0.01);
    if(keypressed->text() == ".") ui->mu_spinbox->setValue(ui->mu_spinbox->value()+0.01);
    if(keypressed->text() == "k") ui->beta_spinbox->setValue(ui->beta_spinbox->value()+0.01);
    if(keypressed->text() == "m") ui->beta_spinbox->setValue(ui->beta_spinbox->value()-0.01);
    if(keypressed->text() == "f") on_focus_button_clicked();
    if(keypressed->text() == "s") on_start_button_clicked();
    if(keypressed->text() == "p") on_print_button_clicked();
}

void Dialog::on_focus_button_clicked()
{
    stopFocusRecalclulation=true;
    if(started) emit focusChangeNeeded(ui->mainData->xAxis->range(),ui->mainData->yAxis->range());
}

void Dialog::focusChangeDone()
{
    if(stopFocusRecalclulation) return;
    densEmap->data()->operator =(*mainColorMapData);
    ui->mainData->replot();
}

void Dialog::on_fullView_button_clicked()
{
    stopFocusRecalclulation=true;
    if(started) emit focusChangeNeeded(QCPRange(0, global_N), QCPRange(-global_N/2, global_N/2));
}

void Dialog::on_maxCells_lineEdit_editingFinished()
{
    maxCells=ui->maxCells_lineEdit->text().toInt();
}

void Dialog::addFreeEnergy()
{
    ui->mainData->clearGraphs();

    ui->mainData->addGraph(freeEnAxisRect->axis(QCPAxis::atBottom), freeEnAxisRect->axis(QCPAxis::atLeft));
    ui->mainData->graph(0)->setPen(QPen(QColor(Qt::black), 3));
}

void Dialog::findLimits()
{
    lowTLimit.clear();
    highTLimit.clear();

    double temp,tlim;
    QFile infile1("../in/limits/lt_limit_"+sample_name+"_y_"+QString::number(wettability)+".dat");
    infile1.open(QIODevice::ReadWrite);
    QTextStream in1(&infile1);
    QFile infile2("../in/limits/ht_limit_"+sample_name+"_y_"+QString::number(wettability)+".dat");
    infile2.open(QIODevice::ReadWrite);
    QTextStream in2(&infile2);
    for(long long i=0;i<global_N+1;i++)
    {
        in1>>temp>>temp>>tlim>>temp>>temp;
        lowTLimit.push_back(tlim);
        in2>>temp>>temp>>tlim>>temp>>temp;
        highTLimit.push_back(tlim);
    }
    infile1.close();
    infile2.close();
}

void Dialog::addLimits()
{
    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    ui->mainData->graph()->setPen(QPen(Qt::red,3));
    ui->mainData->addGraph(ui->mainData->xAxis,ui->mainData->yAxis);
    ui->mainData->graph()->setPen(QPen(Qt::darkGreen,3));

    for(int i=0;i<global_N+1;i++)
    {
        ui->mainData->graph(1)->addData(i,-highTLimit[i]-global_mu*i);
        ui->mainData->graph(2)->addData(i,-lowTLimit[i]-global_mu*i);

    }

    if(runType=="elimits")
    {
        ui->mainData->addGraph();
        ui->mainData->graph()->setPen(QPen(Qt::black));
        ui->mainData->addGraph();
        ui->mainData->graph()->setPen(QPen(Qt::black));

        for(int i=0;i<global_N+1;i++)
        {
            ui->mainData->graph(3)->addData(i,-entrpUpLim[i]-global_mu*i);
            ui->mainData->graph(4)->addData(i,-entrpLowLim[i]-global_mu*i);
        }
    }
}

void Dialog::updateLimits()
{
    ui->mainData->graph(1)->clearData();
    ui->mainData->graph(2)->clearData();


    for(int i=0;i<global_N+1;i++)
    {
        ui->mainData->graph(1)->addData(i,-highTLimit[i]-global_mu*i);
        ui->mainData->graph(2)->addData(i,-lowTLimit[i]-global_mu*i);

    }

    if(runType=="elimits")
    {
        ui->mainData->graph(3)->clearData();
        ui->mainData->graph(4)->clearData();

        for(int i=0;i<global_N+1;i++)
        {
            ui->mainData->graph(3)->addData(i,-entrpUpLim[i]-global_mu*i);
            ui->mainData->graph(4)->addData(i,-entrpLowLim[i]-global_mu*i);
        }
    }
}

void Dialog::on_dataPackSize_lineEdit_editingFinished()
{
    dataPackSize=ui->dataPackSize_lineEdit->text().toInt();
}

void Dialog::on_print_button_clicked()
{
    saveBoundary=1;
    print_count++;
    QString fileName = "../out/print/graph_"+QString::number(print_count)+"_b_"+QString::number(global_beta_past)+"_mu_"+QString::number(global_mu)+".png";
    ui->mainData->savePng(fileName,0,0,1.0,-1);
}

void Dialog::configureAerogelAxes()
{
    if(aerogelOrIsing)
    {
        freeEnAxisRect->axis(QCPAxis::atBottom)->setLabel("𝜚");
        freeEnAxisRect->axis(QCPAxis::atLeft)->setLabel("𝜔");
    }
    else
    {
        freeEnAxisRect->axis(QCPAxis::atBottom)->setLabel("M");
        freeEnAxisRect->axis(QCPAxis::atLeft)->setLabel("ƒ");
    }
    emit muDensConfigureAerogelAxes();
}
