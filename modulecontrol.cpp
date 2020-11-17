#include "modulecontrol.h"
#include "ui_modulecontrol.h"

ModuleControl::ModuleControl(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModuleControl)
{
    ui->setupUi(this);
}

ModuleControl::~ModuleControl()
{
    delete ui;
}
