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
    model->setStringList(list);
    ui->listView->setModel(model);
}

MainWindow::~MainWindow()
{
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
    QModelIndex index = ui->listView->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    saveToFile(itemText);

}


void MainWindow::on_deleteButton_clicked()
{
    QModelIndex index = ui->listView->currentIndex();
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

