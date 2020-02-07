#include "addplantwindow.h"
#include "mainwindow.h"
#include "ui_addplantwindow.h"

AddPlantWindow::AddPlantWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddPlantWindow)
{
    ui->setupUi(this);
    ui->dateObtainedEdit->setDate(QDate::currentDate());
    ui->lastWateredDate->setDate(QDate::currentDate());
}

AddPlantWindow::~AddPlantWindow()
{
    delete ui;
}

void AddPlantWindow::on_addPlantToActive_clicked()
{
    QMap<QString, QString> newPlantData = mapData();
    if(newPlantData["name"] != ""){
        emit sendNewPlant(newPlantData);
        AddPlantWindow::close();
    }
    else{
        QMessageBox warningBox;
        warningBox.setText("Please insert a valid name!");
        warningBox.exec();
    }
}

QMap<QString, QString> AddPlantWindow::mapData()
{
    QList<QCheckBox *> attributeBoxes = ui->plantTypeBox->findChildren<QCheckBox *>();
    QString attributeNames;
    for (int i = 0; i < attributeBoxes.length(); i++){
        if(attributeBoxes[i]->isChecked()){
            attributeNames.append(attributeBoxes[i]->objectName()+",");
        }
    }
    QMap<QString, QString> newPlantData;
    QString plantName = ui->plantNameEdit->toPlainText();
    QString obtainDate = ui->dateObtainedEdit->date().toString("dd/MM/yyyy");
    QString lastWaterDate = ui->lastWateredDate->date().toString("dd/MM/yyyy");
    QString notes = ui->notesEdit->toPlainText();
    int waterDay = ui->waterDays->value();
    QString waterDayCount = QString::number(waterDay);
    if(plantName != ""){
        newPlantData.insert("name", plantName);
        newPlantData.insert("obtaindate", obtainDate);
        newPlantData.insert("notes", notes);
        newPlantData.insert("attributes", attributeNames);
        newPlantData.insert("waterdaycount", waterDayCount);
        newPlantData.insert("lastwaterdate", lastWaterDate);
    }
    return newPlantData;
}




