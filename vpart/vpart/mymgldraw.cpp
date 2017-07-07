#include "mymgldraw.h"
#include <vector>
#include <list>

myMGLDraw::myMGLDraw(Dialog *mainDialog)
{
    mainGui=mainDialog;
}

int myMGLDraw::Draw(mglGraph *gr)
{
    mainGr=gr;
    int xdim=mainGui->dim_x,ydim=mainGui->dim_y,zdim=mainGui->dim_z;
    double bodge_factor=8.0/(zdim+1.0); //for colouring to y coordinate... dont even ask...

    if(mainGui->matrixdat.nx==1)
    {
        mainGui->matrixdat.Create(xdim,ydim,zdim);
        mainGui->transdat.Create(xdim,ydim,zdim);

        std::list <std::vector<int> > matrixdata;
        for(long k=0;k<zdim;k+=1) for(long j=0;j<ydim;j+=1)   for(long i=0;i<xdim;i+=1)
        {
            std::vector<int> temp1,temp2;
            temp1.push_back(i);temp1.push_back(j);temp1.push_back(k);
            matrixdata.push_back(temp1);
        }

        for(unsigned int i=0;i<mainGui->global_cells.size();i++)
        if(mainGui->global_cells[i][0]%2==0)
        {
            std::vector<int> temp1;
            temp1.push_back(mainGui->global_cells[i][0]/2);temp1.push_back(mainGui->global_cells[i][1]/2);temp1.push_back(mainGui->global_cells[i][2]/2);
            matrixdata.remove(temp1);
        }

        mainGui->mx.Create(matrixdata.size());
        mainGui->my.Create(matrixdata.size());
        mainGui->mz.Create(matrixdata.size());
        for(unsigned int i=0;matrixdata.size()>0;i++)
        {
            mainGui->mx.a[i]=matrixdata.back()[0];
            mainGui->my.a[i]=matrixdata.back()[1];
            mainGui->mz.a[i]=matrixdata.back()[2];

            mainGui->matrixdat.a[matrixdata.back()[0]+xdim*(matrixdata.back()[1]+ydim*matrixdata.back()[2])] = 2;
            mainGui->transdat.a[matrixdata.back()[0]+xdim*(matrixdata.back()[1]+ydim*matrixdata.back()[2])] = bodge_factor*matrixdata.back()[2];
            matrixdata.pop_back();

        }
    }


    gr->SetSize(800,600);
    gr->Rotate(70,10,0);

    gr->SetFontSize(8);
    gr->Label('x',"x",0); gr->Label('y',"y",0);gr->Label('z',"z",0);
    gr->SetRange('x',0,xdim-1);
    gr->SetRange('y',0,ydim-1);
    gr->SetRange('z',0,zdim-1);
    gr->SetTicks('x',10,0,10);
    gr->SetTicks('y',15,0,0);
    gr->SetTicks('z',10,0,0);
    gr->SetTickRotate(1);

    gr->Box();
    gr->Axis();
    //gr->Dots(mainGui->mx,mainGui->my,mainGui->mz);

    mainGr->Alpha(1);
    if(xdim==18) mainGr->SetRange('c', mainGui->transdat);
    mainGr->SetRange('c', mainGui->transdat);
    mainGr->Surf3C(mainGui->matrixdat,mainGui->transdat);

    return 0;
}

void myMGLDraw::Reload()
{

}

