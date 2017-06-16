﻿#ifndef PARAWINDOW_H
#define PARAWINDOW_H

#include <QDialog>
#include "parameter.h"
#include <osg/ref_ptr>

namespace Ui {
class ParaWindow;
}

class ParaWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ParaWindow(QWidget *parent = 0);
    ~ParaWindow();
    void setMotorRef(Motor *ref);
    Motor *getMotorRef();
    void setCrystalRef(Crystal *ref);
    Crystal *getCrystalRef();
    void setScanerRef(Scaner *ref);
    Scaner *getScanerRef();
    void setLaserRef(Laser *ref);
    Laser *getLaserRef();
    void setPlatRef(Plat *ref);
    Plat *getPlatRef();

private:
    Ui::ParaWindow *ui;
    osg::ref_ptr<Motor> motor;
    osg::ref_ptr<Scaner> scaner;
    osg::ref_ptr<Laser> laser;
    osg::ref_ptr<Crystal> crystal;
    osg::ref_ptr<Plat> plat;


private slots:
    void slot_btn_motor();
    void slot_btn_scan();
    void slot_btn_sort();
    void initParam();
    void updateMotorPara();
    void updateScanPara();
    void updateSortPara();
};

#endif // PARAWINDOW_H
