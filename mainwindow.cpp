#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addplantwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //database init stuff
    databaseConnect();
    model = new QSqlTableModel(this);
    connect(this, SIGNAL(newPlantSelect(QModelIndex)), this, SLOT(on_plantTableView_clicked(QModelIndex)));
    ui->statusLabel->hide();
    ui->needsWaterLabel->hide();
    ui->dateObtainEdit->setDate(QDate::currentDate());
    ui->lastWateredDate->setDate(QDate::currentDate());
    ui->plantTableView->setSortingEnabled(true);
    databaseInit(model);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::databaseConnect()
{
    const QString DRIVER("QSQLITE");
    if(QSqlDatabase::isDriverAvailable(DRIVER))
        {
            QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER);

            db.setDatabaseName(":memory:");

            if(!db.open())
                qWarning() << "databaseConnect error: " << db.lastError().text();
        }
        else
            qWarning() << "databaseConnect error: no driver " << DRIVER << " available";
}

void MainWindow::databaseInit(QSqlTableModel *model)
{
    //sqlite table settings and view settings
    QSqlQuery query("CREATE TABLE plants(id INTEGER PRIMARY KEY, name TEXT, dateobtained TEXT, notes TEXT, attributes TEXT, wateringschedule INT, nextwaterdate TEXT, lastwaterdate TEXT)");
    if(!query.isActive())
            qWarning() << "databaseInit error: " << query.lastError().text();
    //query.exec("INSERT INTO plants(name, dateobtained, attributes, wateringschedule) VALUES('echeveria', '01/01/2020', 'succulent', '21')");
    model->setTable("plants");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Plant Name"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date Obtained"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Next Water Day"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Last Water Day"));
    ui->plantTableView->setModel(model);
    ui->plantTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //setting selection ability now so details can be displayed in detailsbox later
    ui->plantTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->plantTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    //hidden columns are things in database which should not appear in table, only when clicked on for details
    ui->plantTableView->setColumnHidden(0, true); //id
    ui->plantTableView->setColumnHidden(3, true); //notes
    ui->plantTableView->setColumnHidden(4, true); //attributes
    ui->plantTableView->setColumnHidden(5, true); //water every x days
    ui->plantTableView->show();
}

void MainWindow::receiveData(QMap<QString, QString> newPlantData)
{
    const int rowCurrent = model->rowCount();
    model->insertRows(rowCurrent, 1);
    model->setData(model->index(rowCurrent, 1), newPlantData["name"]);
    model->setData(model->index(rowCurrent, 2), newPlantData["obtaindate"]);
    model->setData(model->index(rowCurrent, 3), newPlantData["notes"]);
    model->setData(model->index(rowCurrent, 4), newPlantData["attributes"]);
    model->setData(model->index(rowCurrent, 5), newPlantData["waterdaycount"]);
    model->setData(model->index(rowCurrent, 7), newPlantData["lastwaterdate"]);
    model->submitAll();
    statusLabelChange("New Plant " + newPlantData["name"] + " Added!");
    //receipt of new plant causes the plants row to be selected automatically
    ui->plantTableView->setCurrentIndex(model->index(rowCurrent, 1));
    emit(newPlantSelect(model->index(rowCurrent, 1)));
}

void MainWindow::on_addPlantBtn_clicked()
{
    AddPlantWindow *nAddPlantWindow = new AddPlantWindow();
    nAddPlantWindow->show();
    connect(nAddPlantWindow, SIGNAL(sendNewPlant(QMap<QString, QString>)), this, SLOT(receiveData(QMap<QString, QString>)));
}


void MainWindow::on_plantTableView_clicked(const QModelIndex &index)
{
    //gets extra info from database not displayed in table and displays in details box
    //enums: 0-id 1-name 2-date obtained 3-notes 4-attributes 5-watering sched 6-next water date 7-last water date
    int selectedRow = index.row();
    QString itemText = index.sibling(selectedRow, 1).data().toString();
    QSqlQuery query;
    query.prepare("SELECT * from PLANTS where name = :name");
    query.bindValue(":name", itemText);
    query.exec();
    query.first(); //even though only 1 row can be selected/plant names must be unique, still needed... workaround?
    QString plantName = query.value(1).toString();
    QDate obtainDate = QDate::fromString(query.value(2).toString(), "dd/MM/yyyy");
    QDate lastWaterDate = QDate::fromString(query.value(7).toString(), "dd/MM/yyyy");
    QString plantNotes = query.value(3).toString();
    QString plantAttributes = query.value(4).toString();
    int waterDayValue = query.value(5).toInt();
    QStringList plantAttributeList = plantAttributes.split(",");
    MainWindow::resetCheckboxes();
    //dealing w old checkboxes
    QList<QCheckBox *> allBoxes = ui->plantTypeBox->findChildren<QCheckBox *>();
    //rechecking checkboxes, kinda ugly fix later
    for(int i = 0; i < plantAttributeList.length(); i++){
        if(ui->plantTypeBox->findChild<QCheckBox *>(plantAttributeList[i], Qt::FindDirectChildrenOnly)){
            ui->plantTypeBox->findChild<QCheckBox *>(plantAttributeList[i], Qt::FindDirectChildrenOnly)->setCheckState(Qt::Checked);
        }
    }
    //finally set everything so the user can see it
    ui->plantName->setText(plantName);
    ui->dateObtainEdit->setDate(obtainDate);
    ui->notesEdit->setText(plantNotes);
    ui->waterDays->setValue(waterDayValue);
    ui->lastWateredDate->setDate(lastWaterDate);
    setNextWaterDate(waterDayValue, selectedRow);
}

void MainWindow::on_submitEditBtn_clicked()
{
    QModelIndex index = ui->plantTableView->selectionModel()->currentIndex();
    int selectedRow = index.row();
    model->setData(model->index(selectedRow, 1), ui->plantName->toPlainText());
    model->setData(model->index(selectedRow, 2), ui->dateObtainEdit->date().toString("dd/MM/yyyy"));
    model->setData(model->index(selectedRow, 3), ui->notesEdit->toPlainText());
    QList<QCheckBox *> attributeBoxes = ui->plantTypeBox->findChildren<QCheckBox *>();
    QString attributeNames;
    for (int i = 0; i < attributeBoxes.length(); i++){
        if(attributeBoxes[i]->isChecked()){
            attributeNames.append(attributeBoxes[i]->objectName()+",");
        }
    }
    model->setData(model->index(selectedRow, 4), attributeNames);
    int waterDay = ui->waterDays->value();
    QString waterDayCount = QString::number(waterDay);
    model->setData(model->index(selectedRow, 5), waterDayCount);
    model->submitAll();
    setNextWaterDate(waterDay, selectedRow);
    statusLabelChange("Submitted Changes!");
}

void MainWindow::on_resetEditBtn_clicked()
{
    QModelIndex index = ui->plantTableView->selectionModel()->currentIndex();
    MainWindow::on_plantTableView_clicked(index);
    statusLabelChange("Reset Changes!");
}

void MainWindow::resetCheckboxes()
{
    QList<QCheckBox *> allBoxes = ui->plantTypeBox->findChildren<QCheckBox *>();
    for(int i = 0; i < allBoxes.length(); i++){
        allBoxes[i]->setCheckState(Qt::Unchecked); //manual checkbox reset, not elegant but works for now...
    }
}

void MainWindow::statusLabelChange(QString message)
{
    //causes status label to display for 3 seconds w message from button which calls it
    ui->statusLabel->show();
    ui->statusLabel->setText(message);
    QTimer::singleShot(5000, ui->statusLabel, &QWidget::hide);
}

void MainWindow::setNextWaterDate(int waterDay, int selectedRow)
{
    QDate lastDate = ui->lastWateredDate->date();
    QDate nextDate = lastDate.addDays(waterDay);
    if(nextDate<=QDate::currentDate()){
        ui->needsWaterLabel->show();
    }
    else{
        ui->needsWaterLabel->hide();
    }
    ui->nextWaterDate->setDate(nextDate);
    model->setData(model->index(selectedRow, 6), ui->nextWaterDate->date().toString("dd/MM/yyyy"));
    model->submitAll();
    ui->plantTableView->setCurrentIndex(model->index(selectedRow, 1));
}

void MainWindow::on_markWateredBtn_clicked()
{
    ui->lastWateredDate->setDate(QDate::currentDate());
    QModelIndex index = ui->plantTableView->selectionModel()->currentIndex();
    int selectedRow = index.row();
    model->setData(model->index(selectedRow, 7), ui->lastWateredDate->date().toString("dd/MM/yyyy"));
    setNextWaterDate(ui->waterDays->value(), selectedRow);
    statusLabelChange("Watered plant!");
}
