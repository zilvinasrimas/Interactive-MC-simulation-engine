#ifndef MYMGLDRAW_H
#define MYMGLDRAW_H

#include "dialog.h"
#include <mgl2/qmathgl.h>
#include <QObject>


class Dialog;

class myMGLDraw : public mglDraw, public QObject
{

public:
    myMGLDraw(Dialog *mainDialog);
    int Draw(mglGraph *gr);
    void Reload();

signals:

public slots:

private:
    Dialog *mainGui;
    mglGraph *mainGr;

};

#endif // MYMGLDRAW_H
