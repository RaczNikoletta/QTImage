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
    void saveToDatabase(QString file);
    void deleteFromFile(QString text);

    void on_pushButton_clicked();

    void on_deleteButton_clicked();

    void on_listView_clicked(const QModelIndex &index);



    //void on_listWidget_currentRowChanged(int currentRow);

    void on_pushButton_2_clicked();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_textEdited(const QString &arg1);

private:
    Ui::MainWindow *ui;
    QString selectedImagePath="";
    QStringList list;
    QStringListModel *model;
    QFile pathFile;
    bool fileExists;
    QList<Image> imageList;
    QList<Image> searchList;
    QSqlDatabase db;
    QSqlQuery addImage;
    QSqlQuery deleteUnsaved;
    QSqlQueryModel imageListModel;
    QSqlQueryModel searchListModel;
    QList<bool>saved;
    int curRow;

    QSqlTableModel *imageTableModel;


    void openDatabase(const QString &filename);
    void initializeDatabase();
    void fillIamgeList(bool init=false);
    void imageDataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight);
};
#endif // MAINWINDOW_H
