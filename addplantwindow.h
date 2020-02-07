#ifndef ADDPLANTWINDOW_H
#define ADDPLANTWINDOW_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QMap>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class AddPlantWindow;
}
QT_END_NAMESPACE

class AddPlantWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AddPlantWindow(QWidget *parent = nullptr);
    ~AddPlantWindow();

signals:
    void sendNewPlant(QMap<QString, QString>);

private slots:
    void on_addPlantToActive_clicked();

private:
    Ui::AddPlantWindow *ui;
    QSqlTableModel *model;
    QMap<QString, QString> mapData();
};

#endif // ADDPLANTWINDOW_H
