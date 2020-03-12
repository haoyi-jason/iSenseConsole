#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "snode_interface.h"
#include "command_server.h"
#include "chartview.h"
namespace Ui {
class MainWindow;
}

class isense_chart;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void handle_sonde_connect();
    void handle_snode_type_changed(int index);
    void handle_snode_model_changed(int index);
    void add_interface(int type, QString hostName, int port);
    void handle_log_message(QString msg);
    void handle_data_received(QString, QByteArray);
    void handle_snode_model_select(int index);
    void handle_snode_setting_select(int index);
    void handle_snode_get_param();
    void handle_snode_set_param();
    void handle_snode_start_stop();
    void handle_snode_view_chart();
    void handle_device_setup_update(QByteArray b);
    void handle_device_model_update(QString name, int id);

    void handle_simulator_open();
    void handle_simulator_close();
    void handle_simulator_model_select(int index);

    void handle_chart_destroy(QObject *obj);

    void parseConfig(QByteArray json);
    snode_interface* addInterface(QString type, QString host, int port);
private slots:
    void on_pushButton_clicked();
    void on_pb_add_interface_clicked();
    void on_pb_del_interface_clicked();

    void on_pbListen_clicked();

    void on_pbClose_clicked();

    void on_pbAddSimulator_clicked();


    void on_actionE_xit_triggered();

    void on_action_Add_triggered();

    void on_action_Delete_triggered();

    void on_action_Start_triggered();

    void on_action_Add_2_triggered();

    void on_action_Clear_triggered();

    void on_actionE_xit_2_triggered();

    void on_action_Load_triggered();

    void on_action_Save_triggered();

    void on_pb_a_clicked();

    void on_pb_b_clicked();

private:
    void closeEvent(QCloseEvent *e);
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    //snode_interface m_snode;
    snode_interface_model *m_ifmodel;
    snode_simulator_model *m_simulator;
    command_server *m_cmdServer;
    QStringListModel *m_logModel;
    QList<chartView*> m_charts;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QAction *minimizeAction,*maximizeAction,*restoreAction,*quitAction;
};

#endif // MAINWINDOW_H
