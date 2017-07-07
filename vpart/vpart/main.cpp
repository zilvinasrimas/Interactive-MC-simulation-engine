
#include <QApplication>
#include <QtCore>
#include "dialog.h"
#include "worker.h"
#include "helper.h"
#include "mudensdialog.h"
#include "sample3ddialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType< QHash<int,QHash<int, double> > > ("QHash<int,QHash<int, double> >");
    qRegisterMetaType< QCPRange > ("QCPRange");
    qRegisterMetaType< QVector<double> > ("QVector<double>");

    QThread *mainworkthread = new QThread;
    Worker *mainworker = new Worker;
    QThread *muhelperthread = new QThread;
    Helper *muhelper = new Helper;
    Dialog *w=new Dialog(mainworker,muhelper);

    mainworker->secondary_constructor(w);
    muhelper->secondary_constructor(w);
    MuDensDialog *w2=new MuDensDialog(w,mainworker,muhelper);
    Sample3DDialog *w3=new Sample3DDialog(w,mainworker,muhelper);

    QObject::connect(w, SIGNAL(destroyed()), mainworkthread, SLOT(quit()));
    QObject::connect(w, SIGNAL(destroyed()), muhelperthread, SLOT(quit()));

    mainworker->moveToThread(mainworkthread);
    mainworkthread->start();
    muhelper->moveToThread(muhelperthread);
    muhelperthread->start();


    w->show();
    w2->show();
    w3->show();

    return a.exec();
}
