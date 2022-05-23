#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <fstream>
#include <iostream>
#include <QFile>
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

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

private:
    Ui::MainWindow *ui;
    QString selectedImagePath="";
    QStringList list;
    QStringListModel *model;
     QFile pathFile;
};
#endif // MAINWINDOW_H
