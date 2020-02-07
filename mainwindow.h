#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMap>
#include <QDebug>
#include <QTimer>
#include <iostream>
#include "addplantwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void sendDatabase(QSqlTableModel *model);
    void newPlantSelect(const QModelIndex &index);


private slots:
    void on_addPlantBtn_clicked();
    void receiveData(QMap<QString, QString>);
    void on_plantTableView_clicked(const QModelIndex &index);
    void on_submitEditBtn_clicked();
    void on_resetEditBtn_clicked();
    void on_markWateredBtn_clicked();

private:
    Ui::MainWindow *ui;
    AddPlantWindow *nAddPlantWindow;
    QSqlTableModel *model;
    void databaseConnect();
    void databaseInit(QSqlTableModel *model);
    void resetCheckboxes();
    void statusLabelChange(QString);
    void setNextWaterDate(int, int);

};
#endif // MAINWINDOW_H
