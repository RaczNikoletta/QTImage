#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <iostream>
#include <fstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection); //you can select any number of items
    model = new QStringListModel(this);
    readFromFile();
    ui->setupUi(this);
    openDatabase("db.sqlite");
    fillIamgeList();

    /*addImage=QSqlQuery(db);
    addImage.prepare("INSERT INTO image (path, tag, comment) VALUES (:path, :tag, :comment)");
    addImage.bindValue(":path","random");
    addImage.bindValue(":tag", "tag");
    addImage.bindValue(":comment", "comment");
                     if (addImage.exec())
                     {
                     }
                     else
                     {
                         qDebug() << addImage.lastError();
                     }*/

   imageTableModel = new QSqlTableModel(this,db);
   imageTableModel->setTable("image");
   imageTableModel->setHeaderData(0,Qt::Horizontal,"Path");
   imageTableModel->setHeaderData(1,Qt::Horizontal,"Tag");
   imageTableModel->setHeaderData(2,Qt::Horizontal,"Comment");
   imageTableModel->select();
   ui->listView->setModel(imageTableModel);
   connect(imageTableModel,&QSqlTableModel::dataChanged,this,&MainWindow::imageDataChanged);
   model->setStringList(list);
    //ui->listView->setModel(model);
}

MainWindow::~MainWindow()
{
    db.close();
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
        model->insertRow(model->rowCount());
        list.append(filename);
        QModelIndex index = model->index(model->rowCount()-1);
        model->setData(index,filename);
    }
}

void MainWindow::saveToFile(QString file)
{
   const QString path("imagePaths.txt");
   QFile pathFile(path);
   QTextStream textStream(&pathFile);
   if(pathFile.open(QIODevice::ReadWrite | QIODevice::Append)){
       while(!pathFile.atEnd()){
           QString line = textStream.readLine();
           if(line.isNull()){
               break;
           }
           else if(QString::compare(file,line)==0){
               fileExists = true;
           }
       }
       if(!fileExists){
        textStream << file+"\n";
       }

   }else
       qDebug() << "Failed to open file";

   fileExists = false;
   pathFile.close();



}

void MainWindow::readFromFile()
{

    const QString path("imagePaths.txt");
    QFile pathFile(path);
    if(pathFile.open(QIODevice::ReadOnly)){
        //qDebug() << "open";
        QTextStream textStream(&pathFile);
        while(true){
            //qDebug() << "in while";
            QString line = textStream.readLine();
            if(line.isNull()){
                break;
            }
            else if(line!=""){
                list.append(line);
                qDebug() << line;
                QModelIndex index = model->index(model->rowCount()-1);
                model->setData(index,line);
            }
        }
    }else
        qDebug() << "Failed to open file";
}

void MainWindow::deleteFromFile(QString text)
{

    const QString path("imagePaths.txt");
       QFile pathFile(path);
       if(pathFile.open(QIODevice::ReadWrite | QIODevice::Text)){
           //qDebug() << "open";
           QString s;
           QTextStream textStream(&pathFile);
           while(!textStream.atEnd()){
               //qDebug() << "in while";
               QString line = textStream.readLine();
                   if(line !=text){
                       s.append(line+"\n");
                       QModelIndex index = model->index(model->rowCount()-1);
                       model->setData(index,line);
                   }
                   //qDebug() << line;
               }

       pathFile.resize(0);
       textStream << s;
       pathFile.close();
       }

}


void MainWindow::on_pushButton_clicked()
{
    /*foreach(const QModelIndex &index,
            ui->listView->selectionModel()->selectedIndexes())
        saveToFile(model->itemFromIndex(index)->text());
        */ //selected elements can be more than one
    QModelIndex index = ui->listWidget->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    saveToFile(itemText);

}


void MainWindow::on_deleteButton_clicked()
{
    QModelIndex index = ui->listWidget->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    model->removeRow(index.row());
    list.removeAt(index.row());
    deleteFromFile(itemText);
}


void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    QString itemText = index.data(Qt::DisplayRole).toString();
    QPixmap pixmap(itemText);
    ui->picturelabel->setPixmap(pixmap.scaled(ui->picturelabel->size(), Qt::KeepAspectRatio));
    ui->picturelabel->setAlignment(Qt::AlignCenter);
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
        }else{
            //addImage.prepare("INSERT INTO image (path, tag, comment) VALUES (:path, :tag, :comment)");

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
    if(!query.exec("CREATE TABLE image (path VARCHAR(100) PRIMARY KEY, tag TEXT, comment TEXT)"))
    {
        qDebug() << "ERROR create image table" << query.lastError();
    }


}

void MainWindow::fillIamgeList()
{
    int i = 0;
    qDebug() << " "+ i;
    imageList.clear();
    ui->listWidget->clear();
    QSqlQuery query(db);
    query.exec("SELECT * FROM image");
    while(query.next())
    {
        i++;
        qDebug() << " "+ i;
        ui->listWidget->addItem(query.value("path").toString());
        imageList.push_back(Image(query.value("path").toString(),
                                  query.value("tag").toString(),
                                  query.value("comment").toString()));
    }
    ui->listWidget->setCurrentRow(0);
}

void MainWindow::imageDataChanged(const QModelIndex &,const QModelIndex &)
{
    fillIamgeList();
}


void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
    if(currentRow!=-1){
        ui->tagLabel->setText(imageList.at(currentRow).tag);
        ui->commentLabel->setText(imageList.at(currentRow).comment);
    }
    else{
        ui->tagLabel->clear();
        ui->commentLabel->clear();
       }


}

