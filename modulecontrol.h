#ifndef MODULECONTROL_H
#define MODULECONTROL_H

#include <QDialog>

namespace Ui {
class ModuleControl;
}

class ModuleControl : public QDialog
{
    Q_OBJECT

public:
    explicit ModuleControl(QWidget *parent = nullptr);
    ~ModuleControl();

private:
    Ui::ModuleControl *ui;
};

#endif // MODULECONTROL_H
