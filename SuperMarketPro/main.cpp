#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>

extern int loginStatus;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ///Language icin:
    QString firstLine = "Turkish";
    QString Dosya = "setting.txt";
    QFile file(Dosya);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString Message = "Dosya (setting.txt) okumak için açılamıyor.\nLütfen dosyayı kontrol edip tekrar deneyin.";
        qDebug()<<Message;
    }else{
        QTextStream veriler(&file);
        firstLine = veriler.readLine();
        qDebug()<<firstLine;
        file.close();
    }

    QTranslator translator;
    if(firstLine == "English"){
        qDebug()<<"firstLine";
        translator.load(":/dil/language/english.qm");
        a.installTranslator(&translator);


    }
    ///

    QSqlDatabase DataBase = QSqlDatabase::addDatabase("QSQLITE");
    QString Veritabani_Path = "SuperMarketPro_DataBase.db";
    DataBase.setDatabaseName(Veritabani_Path);

    if (!DataBase.open())
    {
        qDebug() << "DataBase not conected";
    }
    else
    {
        qDebug() << "DataBase is Conected";
    }
    ///// Giris Ekrani:
    login *Giris_Ekrani = new login(DataBase);
    Giris_Ekrani->exec();

    // loginStatus = 1;//

    if (loginStatus == 1)
    {
        MainWindow *w = new MainWindow(DataBase);
        w->show();
        return a.exec();
    }
    /////

    a.exit();
    return 0;
}
