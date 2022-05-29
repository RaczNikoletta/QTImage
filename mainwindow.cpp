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
    //ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection); //you can select any number of items
    model = new QStringListModel(this);
    //readFromFile();
    ui->setupUi(this);
    openDatabase("db.sqlite");
    QCommonStyle style;
    ui->backButton->setIcon(style.standardIcon(QStyle::SP_ArrowLeft));
    ui->nextButton->setIcon(style.standardIcon(QStyle::SP_ArrowRight));
    fillIamgeList(true);
    //createLanguageMenu();


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
   selectionModel = ui->listView->selectionModel();

   connect(selectionModel,&QItemSelectionModel::currentChanged,this,&MainWindow::rowChanged);
   connect(imageTableModel,&QSqlTableModel::dataChanged,this,&MainWindow::imageDataChanged);


   searchListModel.setHeaderData(0,Qt::Horizontal,"Path");
   searchListModel.setHeaderData(1,Qt::Horizontal,"Tag");
   searchListModel.setHeaderData(2,Qt::Horizontal,"Comment");
   //ui->searchList->setModel(&searchListModel);
   //connect(imageTableModel,&QSqlTableModel::dataChanged,this,&MainWindow::imageDataChanged);
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


void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
    /*QMutableListIterator<Image> i(imageList);
    while(i.hasNext())
    {
        if(i.next().tag.contains(ui->lineEdit->text()))
        {
            qDebug () << "Remove: " << i.next().tag;
            i.remove();
            for(int j=0;j<imageList.size();j++)
            {
                qDebug () << "Tag: " << imageList.at(j).tag;
            }

        }else
        {
            //fillIamgeList();
            //imageTableModel->select();
        }
    }

    while(i.hasNext())
    {
        tempList.push_back(i.next().path);
    }
    model->setStringList(tempList);
    ui->listView->setModel(model);

    //imageTableModel->select();*/
}


void MainWindow::on_lineEdit_textEdited(const QString &arg1)
{
    searchList.clear();
    ui->listWidget->clear();
    QSqlQuery query(db);
    //int i =0;
    query.prepare("SELECT * FROM image WHERE path LIKE :argum or tag LIKE :argum or comment LIKE :argum");
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

