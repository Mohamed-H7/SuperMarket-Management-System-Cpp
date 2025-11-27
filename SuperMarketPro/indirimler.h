#ifndef INDIRIMLER_H
#define INDIRIMLER_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QMessageBox>

namespace Ui {
class indirimler;
}

class indirimler : public QDialog
{
    Q_OBJECT

public:
    explicit indirimler(QSqlDatabase,int sec = 0, QWidget *parent = nullptr);
    ~indirimler();

    QString indirim_id;
    QString indirim_ad;
    QString indirim_yuzdesi;
    QString Strt_date;
    QString End_date;
    QString urun_numara;

    int IKategori;
    int indirim_tipi;

    void indirimBilgiGuncelleme();

signals:
    void IndirimEkledi();

private slots:
    void on_iptal_btn_clicked();

    void on_ekle_btn_clicked();

    void on_radioButton_urun_clicked();

    void on_radioButton_kategori_clicked();

private:
    Ui::indirimler *ui;

    QSqlQuery *sorgu_indirim;

    void urunKategori();
};

#endif // INDIRIMLER_H
