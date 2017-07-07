#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include "dialog.h"
#include "part_mc.h"
#include "part_mc_base.h"
#include "part_mc_base_y12.h"

class par;
class Dialog;

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = 0);

signals:
    void newDataPack(QHash<int,QHash<int, double> >);
    void systemInitialized();
    void sendMuDensData(double,double);
    void sendMuZkDensData(double,double,double,double,double);

public slots:
    void on_start_button_clicked();
    void secondary_constructor(Dialog *mainDialog);

private slots:
    void run_part_mc_base(par pr);
    void run_part_mc_base_y12(par pr);


private:
    Dialog *mainGui;

};

#endif // WORKER_H
