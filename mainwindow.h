#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <fstream>
#include <iostream>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    struct Image
    {
        QString path, tag, comment;
        Image(const QString &path,const QString &tag= "",const QString &comment = ""):
            path(path),
            tag(tag),
            comment(comment)
        {}
    };

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_selectImpushButton_clicked();
    void saveToFile(QString file);
    void readFromFile();
    void deleteFromFile(QString text);

    void on_pushButton_clicked();

    void on_deleteButton_clicked();

    void on_listView_clicked(const QModelIndex &index);



    void on_listWidget_currentRowChanged(int currentRow);

private:
    Ui::MainWindow *ui;
    QString selectedImagePath="";
    QStringList list;
    QStringListModel *model;
    QFile pathFile;
    bool fileExists;
    QList<Image> imageList;
    QSqlDatabase db;
    QSqlQuery addImage;
    QSqlQueryModel imageListModel;

    QSqlTableModel *imageTableModel;


    void openDatabase(const QString &filename);
    void initializeDatabase();
    void fillIamgeList();
    void imageDataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight);
};
#endif // MAINWINDOW_H
