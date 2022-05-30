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
#include <QListWidgetItem>
#include <QItemSelectionModel>
#include <QTranslator>
#include <QSettings>
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    struct Image
    {
        QString path, category, tag, comment;
        Image(const QString &path,const QString &category="",const QString &tag= "",const QString &comment = ""):
            path(path),
            category(category),
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

    void on_pushButton_clicked();

    void on_deleteButton_clicked();

    void on_listView_clicked(const QModelIndex &index);


    void on_pushButton_2_clicked();


    void on_lineEdit_textEdited(const QString &arg1);

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_3_clicked();

    void on_nextButton_clicked();

    void on_backButton_clicked();

    void on_actionHungarian_2_triggered();

    void on_actionSpanish_triggered();



    void on_actionEnglish_2_triggered();

    void on_actionDefault_3_triggered();

    void on_actionBlue_3_triggered();

    void on_actionCompact_triggered();

    void on_actionDefault_4_triggered();
    void showMenu(const QPoint &pos);

    void showWindow();

    //imagesizes
    void zoom();
    void reduce();
    void original();

private:
    Ui::MainWindow *ui;
    QString selectedImagePath="";
    QFile pathFile;
    bool fileExists;
    QList<Image> imageList;
    QList<Image> searchList;
    QSqlDatabase db;
    QSqlQuery addImage;
    QSqlQuery deleteUnsaved;
    QSqlQuery updateQ;
    QSqlQueryModel imageListModel;
    QSqlQueryModel searchListModel;
    QItemSelectionModel *selectionModel;
    QList<bool>saved;
    int curRow;
    //for translator
    QTranslator trans;
    QTranslator transQt;
    QString currLang;
    QString langPath;

    QAction *zoomin;
    QAction *reduced;
    QSqlTableModel *imageTableModel;
    QSettings *appSettings;
    QStringList styles;
    //-1 if reduced 1 if zoomed
    int zoomed=0;
    bool compactOn = false;


    void openDatabase(const QString &filename);
    void initializeDatabase();
    void fillIamgeList(bool init=false);
    void imageDataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight);
    void rowChanged(const QModelIndex &current, const QModelIndex &previous);
    //loads a language by the given language shortcut
    void loadLanguage(const QString &rLanguage);
    //creates language menu
    void createLanguageMenu(void);
    void switchTranslator(QTranslator& translator, const QString& filename);


};
#endif // MAINWINDOW_H
