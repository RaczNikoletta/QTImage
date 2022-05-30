#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <iostream>
#include <fstream>
#include <QVBoxLayout>
#include <QCommonStyle>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    model = new QStringListModel(this);
    ui->setupUi(this);
    openDatabase("db.sqlite");
    QCommonStyle style;
    ui->backButton->setIcon(style.standardIcon(QStyle::SP_ArrowLeft));
    ui->nextButton->setIcon(style.standardIcon(QStyle::SP_ArrowRight));
    fillIamgeList(true);

    addImage=QSqlQuery(db);
    addImage.prepare("INSERT INTO image (path, tag, comment) VALUES (:path, :tag, :comment)");

    deleteUnsaved=QSqlQuery(db);
    deleteUnsaved.prepare("DELETE FROM image WHERE path = :path");

    updateQ=QSqlQuery(db);
    updateQ.prepare("UPDATE image SET tag = :tag, category = :category, comment = :comment where path = :path");

    appSettings=new QSettings("Qttargy", "imageViewer", this);

    appSettings->beginGroup("styles");
    appSettings->beginGroup("Dark");
    appSettings->setValue("background-color",QColor(0, 85, 255));
    appSettings->setValue("selection-color",QColor(170, 0, 255));
    styles=appSettings->childGroups();
    styles.sort();
    appSettings->endGroup();
    appSettings->endGroup();
    appSettings->beginGroup("styles");
    appSettings->beginGroup("Default");
    appSettings->endGroup();
    appSettings->endGroup();


   imageTableModel = new QSqlTableModel(this,db);
   imageTableModel->setTable("image");
   imageTableModel->setHeaderData(0,Qt::Horizontal,"Path");
   imageTableModel->setHeaderData(1,Qt::Horizontal,"Tag");
   imageTableModel->setHeaderData(2,Qt::Horizontal,"Comment");
   imageTableModel->select();
   ui->listView->setModel(imageTableModel);
   selectionModel = ui->listView->selectionModel();

   connect(selectionModel,&QItemSelectionModel::currentChanged,this,&MainWindow::rowChanged);
   connect(imageTableModel,&QSqlTableModel::dataChanged,this,&MainWindow::imageDataChanged);


   searchListModel.setHeaderData(0,Qt::Horizontal,"Path");
   searchListModel.setHeaderData(1,Qt::Horizontal,"Tag");
   searchListModel.setHeaderData(2,Qt::Horizontal,"Comment");
}

MainWindow::~MainWindow()
{
    for(int i=0;i<imageList.size();i++)
    {
        if(saved[i]==false)
        {
            deleteUnsaved.bindValue(":path",imageList[i].path);
            if(deleteUnsaved.exec()){
                qDebug() << "destructor Delete ok ";
                //fillIamgeList();
                //imageTableModel->select();
            }else{
                qDebug() << "destructor Delete not ok " << deleteUnsaved.lastError();

        }
    }
    }
    db.close();
    delete appSettings;
    delete ui;
}


void MainWindow::on_selectImpushButton_clicked()
{
    QString filter;
    QString filename = QFileDialog::getOpenFileName(this, "Select an image", "","Images (*.jpg; *.jpeg; *.png; *.bmp)", &filter);
    if(!filename.isEmpty())
    {
        qDebug() << filename;
        qDebug() << filter;
        QPixmap pixmap(filename);
        ui->picturelabel->setPixmap(pixmap.scaled(ui->picturelabel->size(), Qt::KeepAspectRatio));
        ui->picturelabel->setAlignment(Qt::AlignCenter);
        //model->insertRow(model->rowCount());
        saved.append(false);
        saveToDatabase(filename);
        //imageTableModel.

        //list.append(filename);
        //QModelIndex index = model->index(model->rowCount()-1);
        //model->setData(index,filename);
    }
}

void MainWindow::saveToDatabase(QString file)
{
    addImage.bindValue(":path",file);
    addImage.bindValue(":category","");
    addImage.bindValue(":tag","");
    addImage.bindValue(":comment","");

    if(addImage.exec())
    {
        fillIamgeList();
        imageTableModel->select();
    }else{
        qDebug() << addImage.lastError();
    }
}



void MainWindow::on_pushButton_clicked()
{
    /*foreach(const QModelIndex &index,
            ui->listView->selectionModel()->selectedIndexes())
        saveToFile(model->itemFromIndex(index)->text());
        */ //selected elements can be more than one
    QModelIndex index = ui->listView->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    saved[index.row()] = true;

}


void MainWindow::on_deleteButton_clicked()
{
    QModelIndex index = ui->listView->currentIndex();
    deleteUnsaved.bindValue(":path",imageList[index.row()].path);
    qDebug() << imageList[index.row()].path;
    if(deleteUnsaved.exec()){
        qDebug() << "Delete ok ";
        saved[curRow]=false;
        fillIamgeList();
        imageTableModel->select();
    }else{
        qDebug() << "Delete not ok " << deleteUnsaved.lastError();
    }
    QModelIndex index2 = ui->listView->currentIndex();
    QString itemText = index2.data(Qt::DisplayRole).toString();
    QPixmap pixmap(itemText);
    ui->picturelabel->setPixmap(pixmap.scaled(ui->picturelabel->size(), Qt::KeepAspectRatio));
    ui->picturelabel->setAlignment(Qt::AlignCenter);

}


void MainWindow::on_listView_clicked(const QModelIndex &index)
{

    if(index.row()!=-1){
        ui->tagLabel->setText(imageList.at(index.row()).tag);
        ui->commentLabel->setText(imageList.at(index.row()).comment);
        curRow = index.row();

        QString itemText = index.data(Qt::DisplayRole).toString();
        QPixmap pixmap(itemText);
        ui->picturelabel->setPixmap(pixmap.scaled(ui->picturelabel->size(), Qt::KeepAspectRatio));
        ui->picturelabel->setAlignment(Qt::AlignCenter);
    }
    else{
        ui->tagLabel->clear();
        ui->commentLabel->clear();
       }
}

void MainWindow::openDatabase(const QString &filename)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);

    if(db.open()){
        qDebug() << "DB opened";
        if(db.tables().size()==0)
        {
            qDebug() << "DB is empty";
            initializeDatabase();
        }
    }else
    {
        qDebug() << "DB open error";
    }

}

void MainWindow::initializeDatabase()
{
    QSqlQuery query(db);
    for(QString tablename: db.tables())
    {
        query.exec("DROP TABLE " + tablename);
    }
    if(!query.exec("CREATE TABLE image (path VARCHAR(100) PRIMARY KEY,category TEXT, tag TEXT, comment TEXT)"))
    {
        qDebug() << "ERROR create image table" << query.lastError();
    }


}

void MainWindow::fillIamgeList(bool init)
{

    imageList.clear();
    QSqlQuery query(db);
    int i=0;
    query.exec("SELECT * FROM image");
    while(query.next())
    {

        imageList.push_back(Image(query.value("path").toString(),
                                  query.value("category").toString(),
                                  query.value("tag").toString(),
                                  query.value("comment").toString()));
    }
    //ui->listWidget->setCurrentRow(0);
    if(init)
    {
        for(int i=0;i<imageList.size();i++)
        {
            saved.append(true);
        }
    }
}

void MainWindow::imageDataChanged(const QModelIndex &,const QModelIndex &)
{
    fillIamgeList();

}




void MainWindow::on_pushButton_2_clicked()
{
    updateQ.bindValue(":tag",ui->tagLabel->text());
    updateQ.bindValue(":category",ui->categoryLabel->text());
    updateQ.bindValue(":comment",ui->commentLabel->text());
    updateQ.bindValue(":path",imageList.at(curRow).path);
    if(updateQ.exec())
    {
      qDebug() << "DB updated successfully";
      fillIamgeList();
      imageTableModel->select();
    }else
    {
        qDebug () << "DB update failed " << updateQ.lastError();
    }
}




void MainWindow::on_lineEdit_textEdited(const QString &arg1)
{
    searchList.clear();
    ui->listWidget->clear();
    QSqlQuery query(db);
    //int i =0;
    query.prepare("SELECT * FROM image WHERE path LIKE :argum or category LIKE:argum or tag LIKE :argum or comment LIKE :argum");
    query.bindValue(":argum",QString("%%1%").arg(arg1));
    QString toFind = QString("%%1%").arg(arg1);
    //searchListModel.setQuery("SELECT * FROM image WHERE path LIKE '"+QString("%%1%").arg(arg1)+"' or tag LIKE '"+QString("%%1%").arg(arg1)+"' or comment LIKE "+QString("%%1%").arg(arg1)+"");
    if(query.exec()){
        //qDebug() << "Query ok";
    while(query.next())
    {
        //i++;
        //qDebug() << i;
        ui->listWidget->addItem(query.value("path").toString());
        searchList.push_back(Image(query.value("path").toString(),
                                   query.value("category").toString(),
                                   query.value("tag").toString(),
                                   query.value("comment").toString()));
    }
    }else
    {
        qDebug() << "Query failed: " << query.lastError();
    }
    //qDebug() << searchList.size();
    //searchTableModel->select();
    if(ui->lineEdit->text()==""){
           ui->listWidget->clear();
    }

}


void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QWidget *window = new QWidget;
    //QPushButton *button1 = new QPushButton("One");
    QLabel *imageLabel = new QLabel;
    QString itemText =item->text();
    QPixmap pixmap(itemText);
    imageLabel->setPixmap(pixmap);
    imageLabel->setScaledContents(true);
    imageLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(imageLabel);
    //layout->addWidget(button1);

    window->setLayout(layout);
    window->show();
}


void MainWindow::on_pushButton_3_clicked()
{
    QWidget *window = new QWidget;
    //QPushButton *button1 = new QPushButton("One");
    QLabel *imageLabel = new QLabel;
    QListWidgetItem *item = ui->listWidget->currentItem();
    QString itemText =item->text();
    QPixmap pixmap(itemText);
    imageLabel->setPixmap(pixmap);
    imageLabel->setScaledContents(true);
    imageLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(imageLabel);
    //layout->addWidget(button1);

    window->setLayout(layout);
    window->show();
}


void MainWindow::on_nextButton_clicked()
{

    if(ui->listView->currentIndex().row()!=(imageList.size()-1))
    {
        ui->listView->setCurrentIndex(ui->listView->currentIndex().siblingAtRow(ui->listView->currentIndex().row()+1));
    }else
    {
        ui->listView->setCurrentIndex(ui->listView->currentIndex().siblingAtRow(0));
    }
}


void MainWindow::on_backButton_clicked()
{
    if(ui->listView->currentIndex().row()!=0)
    {
        ui->listView->setCurrentIndex(ui->listView->currentIndex().siblingAtRow(ui->listView->currentIndex().row()-1));
    }else
    {
          ui->listView->setCurrentIndex(ui->listView->currentIndex().siblingAtRow(imageList.size()-1));
    }

}

void MainWindow::rowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if(current.row()!=-1){
        ui->tagLabel->setText(imageList.at(current.row()).tag);
        ui->categoryLabel->setText(imageList.at(current.row()).category);
        ui->commentLabel->setText(imageList.at(current.row()).comment);
        curRow = current.row();

        QString itemText = imageList.at(current.row()).path;
        QPixmap pixmap(itemText);
        ui->picturelabel->setPixmap(pixmap.scaled(ui->picturelabel->size(), Qt::KeepAspectRatio));
        ui->picturelabel->setAlignment(Qt::AlignCenter);
    }
    else{
        ui->tagLabel->clear();
        ui->commentLabel->clear();
       }
}



void MainWindow::switchTranslator(QTranslator& translator, const QString& filename) {
     //qDebug() << "switchTranslator";
 if(!currLang.isEmpty())
 {
 // remove the old translator
 qApp->removeTranslator(&translator);
 //qDebug() << "curr lang is not empty";
 }

 // load the new translator
QString path = QApplication::applicationDirPath();
    path.append("/languages/");
    //qDebug() << path;
 if(translator.load(path + filename)){ //Here Path and Filename has to be entered because the system didn't find the QM Files else
  qApp->installTranslator(&translator);
  ui->retranslateUi(this);
  //qDebug() << "Translator installed";
 }if(!translator.load(path + filename))
 {
     //: No such language to load
     //QMessageBox::critical(this, tr("Language not found"), tr("Language file Translation_%1.qm not found").arg(currLang));
     //qDebug() << "Translation qm not found";
   if (!currLang.isEmpty())
                 {
                     translator.load("languages/Translation_"+currLang+".qm");
                     qApp->installTranslator(&translator);
                 }
 }
}

void MainWindow::loadLanguage(const QString& rLanguage) {
    //qDebug() << "loadLanguage curr" + currLang+ " rlang: " +rLanguage;

 if(currLang != rLanguage) {
  currLang = rLanguage;
  QLocale locale = QLocale(currLang);
  QLocale::setDefault(locale);
  //qDebug() << "not the same language";
  QString languageName = QLocale::languageToString(locale.language());
  switchTranslator(trans, QString("Translation_%1.qm").arg(rLanguage));
  switchTranslator(transQt, QString("qt_%1.qm").arg(rLanguage));
  //ui.statusBar->showMessage(tr("Current Language changed to %1").arg(languageName));
 }
}








void MainWindow::on_actionHungarian_2_triggered()
{

    loadLanguage("hu");
}


void MainWindow::on_actionSpanish_triggered()
{
    loadLanguage("es");
}


void MainWindow::on_actionEnglish_2_triggered()
{
    loadLanguage("en");
}


void MainWindow::on_actionDark_2_triggered()
{
    appSettings->beginGroup("styles");
    QString profileName = ui->actionDark_2->text();
    appSettings->beginGroup("Dark");
    QColor color=appSettings->value("background-color").value<QColor>();
    QColor selection = appSettings->value("selection-color").value<QColor>();
    //QString string=appSettings->value("text").toString();
    //ui->commentLabel->setText(string);
    //ui->commentLabel->setStyleSheet(QString("font: 75 20pt \"MS Shell Dlg 2\";\ncolor: ")+color.name()+";");
    //ui->tabWidget->setStyleSheet(QString("backgorund-color: rgb(0, 0, 0);selection-color: rgb(170, 0, 255);"font: 9pt\ "Ravie\";"));
     ui->tabWidget->setStyleSheet(QString("font: 7pt \"Ravie\";\nbackground-color:" +color.name()+";\nselection-color:"+selection.name()));
    appSettings->endGroup();
    appSettings->endGroup();
}


void MainWindow::on_actionDefault_2_triggered()
{
    appSettings->beginGroup("styles");
    QString profileName = ui->actionDefault_2->text();
    appSettings->beginGroup(profileName);
    QColor color=appSettings->value("background-color").value<QColor>();
    ui->tabWidget->setStyleSheet(QString(""));
    appSettings->endGroup();
    appSettings->endGroup();

}

