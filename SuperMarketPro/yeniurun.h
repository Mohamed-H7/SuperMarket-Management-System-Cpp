#ifndef YENIURUN_H
#define YENIURUN_H

#include <QDialog>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QMessageBox>

#include <QStandardItemModel>

namespace Ui
{
    class yeniUrun;
}

class yeniUrun : public QDialog
{
    Q_OBJECT

public:
    explicit yeniUrun(QSqlDatabase, int sec = 0, QWidget *parent = nullptr);
    ~yeniUrun();

    QString UAd;
    QString UBarkod;
    QString UFiyat;
    QString UMaliyet;
    QString UStok;
    int UKategori;
    QString UID;

    void urunBilgiGuncelleme(); // Urun düzenle icin

signals:
    void UrunuEkledi();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void urunKategori();

private:
    Ui::yeniUrun *ui;

    QSqlQuery *sorgu_urun;
};

#endif // YENIURUN_H
