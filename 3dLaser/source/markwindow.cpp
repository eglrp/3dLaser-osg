#include "markwindow.h"
#include "ui_markwindow.h"
#include <string>
using namespace std;

/************************ȫ�ֱ�����****************************/

/************************ȫ�ֺ�����****************************/
MarkWindow::MarkWindow(QWidget *parent) :
QDialog(parent),
ui(new Ui::MarkWindow)
{
    ui->setupUi(this);
    this->setVisible(false);
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
}

MarkWindow::~MarkWindow()
{
    delete ui;
}