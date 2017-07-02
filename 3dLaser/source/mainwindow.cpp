﻿#include "QMessageBox"
#include <Windows.h>
#include <qdebug>
#include <QFileDialog>
#include "macro.h"
#include "mainwindow.h"
using namespace std;

const std::string domain_oriLayerMatrix = "oriLayerMatrix";//域名-层矩阵初始值域名
const std::string domain_lastUsedLayerMatrix = "lastUsedLayerMatrix";//域名-最近更新的层矩阵域名
const std::string domain_ordinaryLayerPrm = "ordinaryLayerPrm";//域名-普通层参数域名

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    paraw  = new ParaWindow();
    markw  = new MarkWindow();
    mcurvw = new McurvWindow();
    ctrlCard = new CtrlCard();
    f = new File();

    crystal = paraw->getCrystalRef();
    motor = paraw->getMotorRef();
    laser = paraw->getLaserRef();
    scaner = paraw->getScanerRef();
    plat = paraw->getPlatRef();

    run = false;
    initPara();
    initMainWindow();
    initCtrlBoard();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (timerRun)
        delete timerRun;
    if (ctrlCard)
        delete ctrlCard;
}
#if 0
void MainWindow::slot_FileOpen(QString fPath)
{
    QStringList fp;
    if (fPath.isEmpty())
    {
        fp = QFileDialog::getOpenFileNames(this, tr("打开文件"), "", "", 0, 0);
    }
    else
    {
        fp.push_back(fPath);
    }
    if (!fp.isEmpty())
    {
        QString fName;
        foreach(fName, fp)
        {
            string str = fName.toLocal8Bit().data();
            osg::ref_ptr<osg::Node> fNode = osgDB::readNodeFile(str);
            if(fNode)
            {
                osg::ref_ptr<osg::MatrixTransform> mTrans = new osg::MatrixTransform;
                mTrans->addChild(fNode);
                //mTrans->setMatrix(osg::Matrix::translate(osg::Vec3d(0,0,0)));
                curViewer->getRoot()->addChild(mTrans);
                fileList.push_back(fName);
            }
            else
            {
                QMessageBox::warning(this, tr("打开文件"), "文件打开错误", QMessageBox::Yes, QMessageBox::No);
            }
        }
    }
    else
    {
        QMessageBox::warning(this, tr("打开文件"), "文件打开错误", QMessageBox::Yes, QMessageBox::No);
    }

}
#else
// Open 3D mesh model or 2D image
void MainWindow::slot_FileOpen()
{
    QStringList fl;
    if(fileList.isEmpty())
        fl = QFileDialog::getOpenFileNames(this,tr("打开文件"), "", MI.inputPointCloudFilters.join(";;"), 0, 0);
    else
        fl.push_back(fileList.first());

    if (fl.isEmpty())	return;
    else
    {
        //Save path away so we can use it again
        QString path = fl.first();
        path.truncate(path.lastIndexOf("/"));
        pathList.push_back(path);
    }
    openFile(fl.first());
}

void MainWindow::openFile(QString fileName)
{
    QFileInfo fi(fileName);
    QString extension = fi.suffix();
    bool b;
    b = MI.allKnowPointCloudInputFormats.contains(extension.toLower(),Qt::CaseSensitive);
    if(!b)
    {
        QString errorMsgFormat(tr("无法打开文件:\n\"%1\"\n\n详细信息: 文件格式 ") + extension + tr(" 不支持."));
        QMessageBox::critical(this,tr("文件打开错误"),errorMsgFormat.arg(fileName));
        return;
    }

    osg::ref_ptr<osg::Group> group = MI.openMesh(extension,fileName);
    getPoints(fileName);
    //以上各节点name均已不为空
    if(group)
    {
        group->setName(qPrintable(fi.fileName()));
        modelGroup->addChild(group);

        for(unsigned int i=0;i < group->getNumChildren(); i++)
        {
            osg::ref_ptr<osg::Node> node=group->getChild(i);//mt

            //保存用户自定义数据
            osg::ref_ptr<osg::MatrixTransform> mt = dynamic_cast<osg::MatrixTransform *>(node.get());
            mt->setUserValue(domain_oriLayerMatrix, mt->getMatrix());
            mt->setUserValue(domain_lastUsedLayerMatrix, mt->getMatrix());

            //保存默认普通层点云参数
            osg::ref_ptr<ordinaryParameter> op = new ordinaryParameter;
            setUserPrmForNode(mt, op.get());
        }


        //测试段代码---------------------------------------------------------------------------------------------
        osg::ref_ptr<osg::Vec3Array> va= getVertexArray(group);
        int num=va->size();
        crystal->pointCloud.pointNum = num;
        osg::Vec3 v3(va->at(0));
        QMessageBox::information(this,"提示","num = " + QString("%1  %2  %3  %4").arg(num).arg(v3.x()).arg(v3.y()).arg(v3.z()));
        //测试段代码---------------------------------------------------------------------------------------------

        //显示模式：点云图
        updateOSGDisplay(POINTCLOUD);
        updateParam();
    }
    else
    {
        QMessageBox::critical(this,"Error", fileName + "  内容不被支持！");
        return;
    }
}
#endif

void MainWindow::slot_FileSave()
{

}

void MainWindow::slot_FileRecent()
{

}

void MainWindow::slot_FileExit()
{

}

void MainWindow::slot_FileClearPathList()
{
    fileList.clear();
}

void MainWindow::slot_LaserPara()
{
    paraw->updatePara();
    paraw->show();
}

void MainWindow::slot_LaserRegImp()
{

}

void MainWindow::slot_LaserRegExp()
{

}

void MainWindow::slot_LaserBatchSet()
{
    mcurvw->show();
}

void MainWindow::slot_LaserTagGen()
{
    markw->show();
}

void MainWindow::slot_LaserCal()
{

}

void MainWindow::slot_LaserPosTest()
{

}

void MainWindow::slot_LaserPlatSetH()
{

}

void MainWindow::slot_ViewTool()
{

}

void MainWindow::slot_ViewStat()
{

}

void MainWindow::slot_ViewCtrl()
{

}

void MainWindow::slot_ViewForth()
{
    osg::ref_ptr<MyTrackballManipulator> manipulator =
            dynamic_cast<MyTrackballManipulator *>(curViewer->getCameraManipulator());
    osg::Vec3d eye,center,up;
    manipulator->getHomePosition(eye,center,up);
    eye = osg::Vec3d(0.0,0.0,350.0);
    up = osg::Vec3d(0.0,1.0,0.0);

    manipulator->setHomePosition(eye,center,up);
    curViewer->home();//原点
    if(manipulator->isOrthoProjection())
        initProjectionAsOrtho();//若是正视投影，则将视图移至初始位（相当于该投影下的home点）
}

void MainWindow::slot_ViewTop()
{
    osg::ref_ptr<MyTrackballManipulator> manipulator =
            dynamic_cast<MyTrackballManipulator *>(curViewer->getCameraManipulator());
    osg::Vec3d eye,center,up;
    manipulator->getHomePosition(eye,center,up);

    eye = osg::Vec3d(0.0,350.0,0.0);
    up = osg::Vec3d(0.0,0.0,-1.0);
    manipulator->setHomePosition(eye,center,up);
    curViewer->home();//原点
    if(manipulator->isOrthoProjection())
        initProjectionAsOrtho();//若是正视投影，则将视图移至初始位（相当于该投影下的home点）
}

void MainWindow::slot_ViewRight()
{
    osg::ref_ptr<MyTrackballManipulator> manipulator =
            dynamic_cast<MyTrackballManipulator *>(curViewer->getCameraManipulator());
    osg::Vec3d eye,center,up;
    manipulator->getHomePosition(eye,center,up);
    eye = osg::Vec3d(350.0,0.0,0.0);
    up = osg::Vec3d(0.0,1.0,0.0);

    manipulator->setHomePosition(eye,center,up);
    curViewer->home();//原点
    if(manipulator->isOrthoProjection())
        initProjectionAsOrtho();//若是正视投影，则将视图移至初始位（相当于该投影下的home点）
}

void MainWindow::slot_ViewUpdate()
{

}

void MainWindow::slot_ViewOriModel()
{

}

void MainWindow::slot_ViewModModel()
{

}

void MainWindow::slot_ViewApartModel()
{

}
void MainWindow::slot_SortY2X(){}
void MainWindow::slot_SortX2Y(){}
void MainWindow::slot_SortOri(){}
void MainWindow::slot_SortShort(){}
void MainWindow::slot_BlockX2Y(){}
void MainWindow::slot_BlockY2X(){}
void MainWindow::slot_BlockShort(){}
void MainWindow::slot_BlockPara(){}
void MainWindow::slot_OperLaserOri(){}
void MainWindow::slot_OperPlatHome(){}
void MainWindow::slot_OperSetCurPos2LaserOri(){}
void MainWindow::slot_OperSaveCurPos2LaserOri(){}
void MainWindow::slot_DebugAtchModel(){}
void MainWindow::slot_DebugHotTest(){}
void MainWindow::slot_DebugSplitModel(){}
void MainWindow::slot_DebugStopFun(){}
void MainWindow::slot_StdModCube(){}
void MainWindow::slot_StdModLine(){}
void MainWindow::slot_StdModPlat(){}
void MainWindow::slot_StdModSphere(){}
void MainWindow::slot_HelpAboutMe(){}
void MainWindow::slot_LanCHN(){}
void MainWindow::slot_UpdateMainWin()
{

    QString runText;
    QString laserText;
    TRun.count++;
    getTime(&TRun);
    runText = QString("总运行时间: %1:%2:%3")
            .arg(QString::number(TRun.hour))
            .arg(QString::number(TRun.minut))
            .arg(QString::number(TRun.second));
    lb_StRunTime->setText(runText);
    if (run)
        TLaser.count++;
    getTime(&TLaser);
    laserText = QString("总雕刻时间: %1:%2:%3")
            .arg(QString::number(TLaser.hour))
            .arg(QString::number(TLaser.minut))
            .arg(QString::number(TLaser.second));
    lb_StLaserTime->setText(laserText);

    //refresh mainwindow para display
    if (paraw->getIsUpdate())
        updateParam();
    //just for test
    spyPutIn();
}

void MainWindow::slot_MovTo()
{
    int rtn = 0;

    plat->DstPos.x = ui->le_PlatMValX->text().toFloat();
    plat->DstPos.y = ui->le_PlatMValY->text().toFloat();
    plat->DstPos.z = ui->le_PlatMValZ->text().toFloat();
    plat->MovPos = plat->DstPos - plat->CurPos;
    //*************设置轴*****************
    rtn = ctrlCard->Setup_Speed(motor->motorX.num, motor->motorX.v0, motor->motorX.v, motor->motorX.a);
    rtn += ctrlCard->Axis_Pmove(motor->motorX.num, plat->MovPos.x);
    rtn += ctrlCard->Setup_Speed(motor->motorX.num, motor->motorX.v0, motor->motorX.v, motor->motorX.a);
    rtn += ctrlCard->Axis_Pmove(motor->motorX.num, plat->MovPos.y);
    rtn += ctrlCard->Setup_Speed(motor->motorX.num, motor->motorX.v0, motor->motorX.v, motor->motorX.a);
    rtn += ctrlCard->Axis_Pmove(motor->motorX.num, plat->MovPos.z);
    if (DRV_FAIL == rtn)
    {
        QMessageBox::warning(this, "错误", "轴速度设置失败", QMessageBox::Ok);
        return;
    }
}

void MainWindow::spyPutIn()
{
    int rtn = 0;
    QString str;
    for (int i = 0; i < 22; i++)
    {
        rtn = ctrlCard->Read_Input(i);
        if (IO_L == rtn)
            str = "低电平";
        else if (IO_H == rtn)
            str = "高电平";
        else
            str = "错误";
        lb_StInfo->setText("IO" + QString::number(i, 10) + "触发" + str);
    }
}

void MainWindow::slot_MotorStop()
{
    for (int i = 0; i < 3; i++)
    {
        ctrlCard->StopRun(i+1, SUDDEN_STOP);
    }
}

void MainWindow::slot_CrvView()
{

}

void MainWindow::slot_CrvApp()
{
    crystal->size.x = ui->le_CrvSizeX->text().toFloat();
    crystal->size.y = ui->le_CrvSizeY->text().toFloat();
    crystal->size.z = ui->le_CrvSizeZ->text().toFloat();
    crystal->mov.x = ui->le_MovX->text().toFloat();
    crystal->mov.y = ui->le_MovY->text().toFloat();
    crystal->mov.z = ui->le_MovZ->text().toFloat();
}

void MainWindow::slot_BlockParaApply()
{
    crystal->blockSet.size.x = ui->le_BlkValX->text().toFloat();
    crystal->blockSet.size.y = ui->le_BlkValY->text().toFloat();
    crystal->blockSet.isCenter = ui->cb_BlkCen->isChecked();
    crystal->blockSet.angle = ui->le_BlkValAngle->text().toFloat();
    crystal->blockSet.width = ui->le_BlkValWidth->text().toFloat();
    crystal->blockSet.stdDev = ui->le_BlkValStdDev->text().toFloat();
    if (ui->rbtn_BlkNor->isChecked())
        crystal->blockSet.fuzzySet = Fuz_Normal;
    else if (ui->rbtn_BlkUni->isChecked())
        crystal->blockSet.fuzzySet= Fuz_Uniform;
}

void MainWindow::slot_VoiceAlarm()
{

}

void MainWindow::slot_PlatOrigin()
{

}

void MainWindow::slot_PlatReset()
{
    int rtn = DRV_OK;
    int dir = 0;
    S_Motor m[MAXAXIS] = {motor->motorX, motor->motorY, motor->motorZ};
    for (int i=0; i < MAXAXIS; i++)
    {
        if (0 == dir && m[i].limitP)
            dir = 1;
        else if (0 == dir && m[i].limitN)
            dir = -1;
        rtn += ctrlCard->Setup_Speed(m[i].num, dir*m[i].v0, dir*m[i].v, m[i].a);
        if (DRV_FAIL == rtn)
        {
            ctrlCard->StopRun(m[i].num, SUDDEN_STOP);
            lb_StInfo->setText("初始化速度错误");
            return;
        }
    }
}

void MainWindow::slot_PlatCtrl()
{

}

void MainWindow::slot_MovMode()
{

}

void MainWindow::slot_LaserCtrl()
{
    int rtn = DRV_FAIL;
    QPushButton *pt = qobject_cast <QPushButton*>(sender());
    if ("开激光" == pt->text())
    {
        rtn = ctrlCard->Set_Laser(0, LASER_ON);
        if (DRV_OK == rtn)
            ui->btn_CrvClsLas->setText("关激光");
        else
            lb_StInfo->setText("打开激光错误");
    }
    else if ("关激光" == pt->text())
    {
        rtn = ctrlCard->Set_Laser(0, LASER_OFF);
        if (DRV_OK == rtn)
            ui->btn_CrvClsLas->setText("开激光");
        else
            lb_StInfo->setText("关闭激光错误");
    }
}

void MainWindow::getTime(T *t)
{
    if (NULL == t)
        qDebug()<<"null ptr";
    int count = t->count;
    t->second = count%60;
    count = count/60;
    t->minut = count%60;
    count = count/60;
    t->hour = count%60;
}

//################Private Function#############

void MainWindow::initMainWindow()
{
    updateParam();
    initStBar();
    initOSG();
}
void MainWindow::initPara()
{
    QFileInfo fi(INI_PATH);
    if (fi.isFile())
    {
        paraw->readIniFile();
    }
    else
    {
        paraw->initParam();
    }
    paraw->updatePara();
}

void MainWindow::updateParam()
{
    //parameters crystal
    ui->lb_PointNumVal->setText(QString::number(crystal->pointCloud.pointNum, 10));
    ui->lb_PointMinValX->setText(QString::number(crystal->pointCloud.pointMin.x, 'f', 2));
    ui->lb_PointMinValY->setText(QString::number(crystal->pointCloud.pointMin.y, 'f', 2));
    ui->lb_PointMinValZ->setText(QString::number(crystal->pointCloud.pointMin.z, 'f', 2));
    ui->lb_PointMaxValX->setText(QString::number(crystal->pointCloud.pointMax.x, 'f', 2));
    ui->lb_PointMaxValY->setText(QString::number(crystal->pointCloud.pointMax.y, 'f', 2));
    ui->lb_PointMaxValZ->setText(QString::number(crystal->pointCloud.pointMax.z, 'f', 2));
    ui->lb_PointLenValX->setText(QString::number(crystal->pointCloud.pointMax.x-
                                                 crystal->pointCloud.pointMin.x, 'f', 2));
    ui->lb_PointLenValY->setText(QString::number(crystal->pointCloud.pointMax.y-
                                                 crystal->pointCloud.pointMin.y, 'f', 2));
    ui->lb_PointLenValZ->setText(QString::number(crystal->pointCloud.pointMax.z-
                                                 crystal->pointCloud.pointMin.z, 'f', 2));
    ui->lb_PointObjValX->setText(QString::number(crystal->size.x, 'f', 2));
    ui->lb_PointObjValY->setText(QString::number(crystal->size.y, 'f', 2));
    ui->lb_PointObjValZ->setText(QString::number(crystal->size.z, 'f', 2));

    //parameters plat
    ui->lb_PlatAbsValX->setText(QString::number(plat->mechPos.x, 'f', 2));
    ui->lb_PlatAbsValY->setText(QString::number(plat->mechPos.y, 'f', 2));
    ui->lb_PlatAbsValZ->setText(QString::number(plat->mechPos.z, 'f', 2));
    ui->lb_PlatRelValX->setText(QString::number(plat->relPos.x, 'f', 2));
    ui->lb_PlatRelValY->setText(QString::number(plat->relPos.y, 'f', 2));
    ui->lb_PlatRelValZ->setText(QString::number(plat->relPos.z, 'f', 2));
    ui->lb_PlatValX->setText(QString::number(plat->size.x, 'f', 2));
    ui->lb_PlatValY->setText(QString::number(plat->size.y, 'f', 2));
    ui->lb_PlatValZ->setText(QString::number(plat->size.z, 'f', 2));
    ui->le_PlatSetValX->setText(QString::number(plat->size.x, 'f', 2));
    ui->le_PlatSetValY->setText(QString::number(plat->size.y, 'f', 2));
    ui->le_PlatSetValZ->setText(QString::number(plat->size.z, 'f', 2));

    //parameters engraving
    ui->le_BlkValX->setText(QString::number(crystal->blockSet.size.x, 'f', 2));
    ui->le_BlkValY->setText(QString::number(crystal->blockSet.size.y, 'f', 2));
    ui->le_BlkValStdDev->setText(QString::number(crystal->blockSet.stdDev, 'f', 2));
    ui->le_BlkValAngle->setText(QString::number(crystal->blockSet.angle, 'f', 2));
    ui->le_BlkValWidth->setText(QString::number(crystal->blockSet.width, 'f', 2));
    if (Fuz_Normal == crystal->blockSet.fuzzySet)
    {
        ui->rbtn_BlkNor->setChecked(TRUE);
        ui->rbtn_BlkUni->setChecked(FALSE);
    }
    else
    {
        ui->rbtn_BlkNor->setChecked(FALSE);
        ui->rbtn_BlkUni->setChecked(TRUE);
    }
    if (TRUE == crystal->blockSet.isCenter)
        ui->cb_BlkCen->setChecked(TRUE);
    else
        ui->cb_BlkCen->setChecked(FALSE);
    if (TRUE == crystal->isAlarm)
        ui->cb_MCrvVo->setChecked(TRUE);
    else
        ui->cb_MCrvVo->setChecked(FALSE);
    if (single_model_one == ui->cmb_MCrvMode->currentIndex())
        crystal->curvMode = single_model_one;
    else if (single_model_mass == ui->cmb_MCrvMode->currentIndex())
        crystal->curvMode = single_model_mass;
    else if (multi_model_mass == ui->cmb_MCrvMode->currentIndex())
        crystal->curvMode = single_model_mass;
    else
        crystal->curvMode = single_model_one;

    ui->le_CrvSizeX->setText(QString::number(crystal->size.x, 'f', 2));
    ui->le_CrvSizeY->setText(QString::number(crystal->size.y, 'f', 2));
    ui->le_CrvSizeZ->setText(QString::number(crystal->size.z, 'f', 2));
    ui->le_MovX->setText(QString::number(crystal->mov.x, 'f', 2));
    ui->le_MovY->setText(QString::number(crystal->mov.y, 'f', 2));
    ui->le_MovZ->setText(QString::number(crystal->mov.z, 'f', 2));
}

void MainWindow::initStBar()
{
    lb_StInfo = new QLabel(this);
    lb_StInfo->setAlignment(Qt::AlignLeft);
    lb_StInfo->setMaximumWidth(200);
    lb_StInfo->setMinimumWidth(200);

    lb_StRunTime = new QLabel(this);
    lb_StRunTime->setAlignment(Qt::AlignLeft);
    lb_StRunTime->setMaximumWidth(200);
    lb_StRunTime->setMinimumWidth(200);

    lb_StLaserTime = new QLabel(this);
    lb_StLaserTime->setAlignment(Qt::AlignLeft);
    lb_StLaserTime->setMaximumWidth(200);
    lb_StLaserTime->setMinimumWidth(200);

    statusBar()->addPermanentWidget(lb_StInfo);
    statusBar()->addPermanentWidget(lb_StRunTime);
    statusBar()->addPermanentWidget(lb_StLaserTime);
    initTimer();

}
void MainWindow::initTimer()
{
    timerRun = new QTimer();
    if (!timerRun)
    {
        qDebug()<<"Fail init timer!";
        delete timerRun;
        return;
    }
    connect(timerRun, SIGNAL(timeout()), this, SLOT(slot_UpdateMainWin()));
    //间隔1s
    timerRun->setInterval(1000);
    timerRun->start();
}

void MainWindow::initOSG()
{
    curViewer = new osgContainer();
    refShape=new osg::Geode;
    modelGroup=new osg::Group;
    pointCloudGroup=new osg::Group;
    ui->ly_viewWt->addWidget(curViewer);

    curRoot = curViewer->getRoot();
    curRoot->addChild(refShape.get());
    curRoot->addChild(modelGroup.get());
    curRoot->addChild(pointCloudGroup.get());

    pointCloudGroup->addChild(ordinaryPointsGroup.get());
    pointCloudGroup->addChild(texturePointsGroup.get());
    pointCloudGroup->addChild(picturePointsGroup.get());
    pointCloudGroup->addChild(additionalPointsGroup.get());

    osg::ref_ptr<PickAndOtherEventToSignalHandler> ph = new PickAndOtherEventToSignalHandler;
    curViewer->addEventHandler(ph.get());
    connect(ph,SIGNAL(signalSendOutPickedNode(osg::Node*,osg::Node*)),
            this,SLOT(on_mousePickedNode(osg::Node*,osg::Node*)));
    connect(ph,SIGNAL(signalSendOutDragging()),this,SLOT(on_mouseDraggingOnOSG()));

    initProjectionAsOrtho();
    setAxesVisible(true);
    updateLighting(false);

    updateRefShape(createCrystalFrame(curCrystalType, curCrystalSize, curCrystalZRot, curCrystalHeight, curCrystalDiameter));

}
void MainWindow::initProjectionAsOrtho()
{
    osg::ref_ptr<osg::Camera> camera = curViewer->getCamera();
    osg::ref_ptr<osg::Viewport> viewport=camera->getViewport();
    double left = -viewport->width()*0.1;//正投影参数
    double right = -left;
    double bottom = -viewport->height()*0.1;
    double top = -bottom;
    double znear = -5000.;
    double zfar = 5000.;

    camera->setProjectionMatrixAsOrtho( left, right, bottom, top, znear, zfar);
}

int MainWindow::initCtrlBoard()
{
    int rtn = CTRL_ERROR;
    int msgRtn = 0;
    //*************初始化8940A1卡**************
    rtn = ctrlCard->Init_Board();
    if (rtn <= 0)
    {
        msgRtn = QMessageBox::warning(this, "错误", "控制卡初始化失败!", QMessageBox::Ok);
        if (QMessageBox::Ok == msgRtn)
        {
            lb_StInfo->setText("控制卡初始化失败!");
        }
        switch (rtn)
        {
        case CTRL_NO_CARD_ERROR:
            msgRtn = QMessageBox::warning(this, "错误", "没有安装ADT8937卡!", QMessageBox::Ok);
            if (QMessageBox::Ok == msgRtn)
            {
                lb_StInfo->setText("没有安装ADT8937卡!");
            }
            break;
        case CTRL_NO_DRIVE_ERROR:
            msgRtn = QMessageBox::warning(this, "错误", "没有安装端口驱动程序!", QMessageBox::Ok);
            if (QMessageBox::Ok == msgRtn)
            {
                lb_StInfo->setText("没有安装端口驱动程序!");
            }
            break;
        case CTRL_PCI_ERROR:

            msgRtn = QMessageBox::warning(this, "错误", "PCI桥故障!", QMessageBox::Ok);
            if (QMessageBox::Ok == msgRtn)
            {
                lb_StInfo->setText("PCI桥故障!");
            }
            break;
         default:
            break;
        }
    }
    else
    {
        lb_StInfo->setText("运动控制卡可以使用!");
    }
    //*************获取版本号**************
    int ver = ctrlCard->Get_HardWareVer();
    QString str = "硬件版本:"+ QString::number(ver, 10);
    lb_StInfo->setText(str);
    //*************初始化限位**************
    S_Motor m[MAXAXIS] = {motor->motorX, motor->motorY, motor->motorZ};
    for (int i = 0; i< MAXAXIS; i++)
    {
        rtn += ctrlCard->set_limit(0, m[i].num, m[i].limitP, m[i].limitN, m[i].limitL);
    }
    if (DRV_FAIL == rtn)
    {
        QMessageBox::warning(this, "错误", "初始化限位错误", QMessageBox::Ok);
    }
    //*************初始化速度**************
    for (int i = 0; i< MAXAXIS; i++)
    {
        rtn += ctrlCard->Setup_Speed(m[i].num, m[i].v0, m[i].v, m[i].a);
    }
    if (DRV_FAIL == rtn)
    {
        QMessageBox::warning(this, "错误", "轴速度设置失败", QMessageBox::Ok);
        return rtn;
    }

    return rtn;
}

//为node设置用户自定义数据
//这里主要是用来为模型或图片层设置 点云参数
void MainWindow::setUserPrmForNode(osg::Node *node, Parameter *prm)
{
    node->setUserData(prm);
}
//更新显示模式（实体/线框/点云）
void MainWindow::updateOSGDisplay(DisplayMode mode)
{
    osg::ref_ptr<osg::StateSet> stateset=modelGroup->getOrCreateStateSet();
    osg::ref_ptr<osg::PolygonMode> pm=new osg::PolygonMode;
    switch(mode)
    {
        case POINTCLOUD:
            setPointCloudVisible(true);
            updateLighting(true);
            break;
        default:
            break;
    }
}

void MainWindow::setPointCloudVisible(bool visible)
{
    curRoot->setChildValue(pointCloudGroup, visible);
}
/// 更新OSG环境光照
void MainWindow::updateLighting(bool brightening)
{
    if(!curViewer) return;
    if(brightening)//环境光增强
    {
        curViewer->getLight()->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        curViewer->getLight()->setAmbient(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
    else//不增强
    {
        curViewer->getLight()->setDiffuse(osg::Vec4(0.7f, 0.7f, 0.7f, 1.0f));
        curViewer->getLight()->setAmbient(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
}
osg::Vec3Array *MainWindow::getVertexArray(osg::Node *node)
{
    vertexExtractor ve;
    node->accept(ve);
    return ve.getAllVertexArrayInWorldCoord();
}
/// 设置坐标轴可见性
void MainWindow::setAxesVisible(bool visible)
{
    curViewer->setAxesVisible(visible);
}
// 更新Ref shape
void MainWindow::updateRefShape(osg::Geode *geode)
{
    curRoot->removeChild(refShape);
    refShape = geode;
    curRoot->addChild(refShape);
}
/// 创建水晶参考
osg::Geode *MainWindow::createCrystalFrame(BasicSettingsDialog::CrystalType type, osg::Vec3 size, float zRot, float height, float diameter)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> colors=new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.f,1.f,0.f,1.f));
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    geode->addDrawable(geom);

    if(type==BasicSettingsDialog::BOX)
    {
        v->push_back(osg::Vec3(-0.5f*size.x(), -0.5f*size.y(), -0.5f*size.z()));
        v->push_back(osg::Vec3(0.5f*size.x(), -0.5f*size.y(), -0.5f*size.z()));
        v->push_back(osg::Vec3(0.5f*size.x(), 0.5f*size.y(), -0.5f*size.z()));
        v->push_back(osg::Vec3(-0.5f*size.x(), 0.5f*size.y(), -0.5f*size.z()));

        v->push_back(osg::Vec3(-0.5f*size.x(), -0.5f*size.y(), 0.5f*size.z()));
        v->push_back(osg::Vec3(0.5f*size.x(), -0.5f*size.y(), 0.5f*size.z()));
        v->push_back(osg::Vec3(0.5f*size.x(), 0.5f*size.y(), 0.5f*size.z()));
        v->push_back(osg::Vec3(-0.5f*size.x(), 0.5f*size.y(), 0.5f*size.z()));

        if(zRot != 0)
        {
            osg::Matrix m = osg::Matrix::rotate(osg::DegreesToRadians(zRot), osg::Vec3(0.f,0.f,1.f));
            for(unsigned int i=0; i<v->size(); ++i)
            {
                osg::Vec4 vt4(v->at(i),1.0f);
                vt4=vt4*m;
                v->at(i) = osg::Vec3(vt4.x(),vt4.y(),vt4.z());
            }
        }

        geom->setVertexArray(v.get());
        osg::ref_ptr<osg::DrawElementsUInt> lines = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);
        for(int i=0; i<8; ++i)
        {
            int j= i+1;
            if(i==3) j=0;
            else if(i==7) j=4;
            lines->push_back(i);
            lines->push_back(j);
        }
        for(int i=0; i<4; ++i)
        {
            lines->push_back(i);
            lines->push_back(i+4);
        }
        geom->addPrimitiveSet(lines);
    }
    else if(type==BasicSettingsDialog::CYLINDER)
    {
        float defaultAngleStep = 3.0f;//缺省步距角
        float r = 0.5f*diameter;
        float x,y,z;
        float rad;
        for(int i=0; i<2; ++i)
        {
            if(i==0) z=-0.5f*height;
            else z=0.5f*height;
            for(float agl=0.f; agl<=360.f; agl+=defaultAngleStep)
            {
                rad = osg::DegreesToRadians(agl);
                x = r*std::cos(rad);
                y = r*std::sin(rad);
                v->push_back(osg::Vec3(x,y,z));
            }
        }
        geom->setVertexArray(v.get());
        geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, v->size()/2)); // down
        geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, v->size()/2, v->size()/2)); // up
    }
    return geode.release();
}

void MainWindow::getPoints(const QString fileName)
{
    QFile in(fileName);
    QString line;
    bool isMin = false;
    bool isMax = false;
    if (!in.open(QIODevice::ReadOnly)) return;
        QTextStream f(&in);
        line = f.readLine();
        while(!line.isNull())
        {
            if ("$EXTMIN" == line)
            {
                while ("9" != line && "ENDSEC" != line && !line.isNull())
                {
                    line = f.readLine();
                    if ("10" == line)
                    {
                        line = f.readLine();
                        if (!line.isNull())
                            crystal->pointCloud.pointMin.x = line.toFloat();
                    }
                    if ("20" == line)
                    {
                        line = f.readLine();
                        if (!line.isNull())
                            crystal->pointCloud.pointMin.y = line.toFloat();
                    }
                    if ("30" == line)
                    {
                        line = f.readLine();
                        if (!line.isNull())
                            crystal->pointCloud.pointMin.z = line.toFloat();
                    }
                }
                isMin = true;
            }
            if ("$EXTMAX" == line)
            {
                while ("9" != line && "ENDSEC" != line && !line.isNull())
                {
                    line = f.readLine();
                    if ("10" == line)
                    {
                        line = f.readLine();
                        if (!line.isNull())
                            crystal->pointCloud.pointMax.x = line.toFloat();
                    }
                    if ("20" == line)
                    {
                        line = f.readLine();
                        if (!line.isNull())
                            crystal->pointCloud.pointMax.y = line.toFloat();
                    }
                    if ("30" == line)
                    {
                        line = f.readLine();
                        if (!line.isNull())
                            crystal->pointCloud.pointMax.z = line.toFloat();
                    }
                }
                isMax = true;
            }
            if (isMin && isMax)
                return;
            line = f.readLine();
        }
}
