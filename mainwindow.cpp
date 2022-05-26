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
    //readFromFile();
    ui->setupUi(this);
    openDatabase("db.sqlite");
    fillIamgeList(true);

    addImage=QSqlQuery(db);
    addImage.prepare("INSERT INTO image (path, tag, comment) VALUES (:path, :tag, :comment)");

    deleteUnsaved=QSqlQuery(db);
    deleteUnsaved.prepare("DELETE FROM image WHERE path = :path");

    /*addImage.bindValue(":path","random");
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
   //model->setStringList(list);
    //ui->listView->setModel(model);
}

MainWindow::~MainWindow()
{
    for(int i=0;i<saved.size();i++)
    {
        if(saved[i]==false)
        {
            deleteUnsaved.bindValue(":path",imageList[i].path);
            if(deleteUnsaved.exec()){
                qDebug() << "Delete ok ";
                fillIamgeList();
                imageTableModel->select();
            }else{
                qDebug() << "Delete not ok " << deleteUnsaved.lastError();

        }
    }
    }
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
    saved[index.row()] = true;

}


void MainWindow::on_deleteButton_clicked()
{
    QModelIndex index = ui->listView->currentIndex();
    deleteUnsaved.bindValue(":path",imageList[index.row()].path);
    qDebug() << imageList[index.row()].path;
    if(deleteUnsaved.exec()){
        qDebug() << "Delete ok ";
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
    if(!query.exec("CREATE TABLE image (path VARCHAR(100) PRIMARY KEY, tag TEXT, comment TEXT)"))
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


/*void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
    if(currentRow!=-1){
        ui->tagLabel->setText(imageList.at(currentRow).tag);
        ui->commentLabel->setText(imageList.at(currentRow).comment);
    }
    else{
        ui->tagLabel->clear();
        ui->commentLabel->clear();
       }


}*/


void MainWindow::on_pushButton_2_clicked()
{
    deleteUnsaved.prepare("UPDATE image SET tag = :tag, comment = :comment where path = :path");
    deleteUnsaved.bindValue(":tag",ui->tagLabel->text());
    deleteUnsaved.bindValue(":comment",ui->commentLabel->text());
    deleteUnsaved.bindValue(":path",imageList.at(curRow).path);
    if(deleteUnsaved.exec())
    {
      qDebug() << "DB updated successfully";
      fillIamgeList();
      imageTableModel->select();
    }else
    {
        qDebug () << "DB update failed " << deleteUnsaved.lastError();
    }
}

