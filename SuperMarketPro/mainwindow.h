#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <login.h>
#include <ciftekran_table.h>
#include <yeniurun.h>
#include <indirimler.h>
#include "notificationpopup.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDate>

#include <QSortFilterProxyModel>
#include <QRegularExpression>

#include <QtCharts>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>


QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QSqlDatabase, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_anasayfa_clicked();

    void on_magaza_clicked();

    void on_rap_an_clicked();

    void on_indirimler_clicked();

    void on_anasayfa_ic_clicked();

    void on_magaza_ic_clicked();

    void on_rap_an_ic_clicked();

    void on_indirimler_ic_clicked();

    void on_ayarlar_ic_clicked();

    void on_ayarlar_clicked();

    void on_kapat_button_clicked();

    void on_menubar_kapat_button_clicked();

    void on_FullScreen_Button_clicked();

    void on_minus_Button_clicked();

    void on_satisR_button_clicked();

    void on_indirimR_button_clicked();

    void on_EnvanterR_button_clicked();

    /////////////////////////////////////

    void on_num_1_clicked();

    void on_num_2_clicked();

    void on_num_3_clicked();

    void on_num_4_clicked();

    void on_num_5_clicked();

    void on_num_6_clicked();

    void on_num_7_clicked();

    void on_num_8_clicked();

    void on_num_9_clicked();

    void on_num_0_clicked();

    void on_sil_button_clicked();

    void on_Menuden_Sil_button_clicked();

    void on_iptal_button_clicked();

    void on_CiftEkran_button_clicked();

    void on_urunler_list_clicked(const QModelIndex &index);

    void on_gir_button_clicked();

    void on_odeme_button_clicked();

    void on_KontrolPaneli_clicked();

    void on_KontrolPaneli_ic_clicked();

    void on_hesap_durumu_activated(int index);

    void on_tableView_hesaplar_clicked(const QModelIndex &index);

    void on_pushButton_aktif_clicked();

    void on_pushButton_pasif_clicked();

    void on_urun_ekle_clicked();

    void on_kategori_urunler_activated(int index);

    void on_lineEdit_urun_arayin_textChanged(const QString &arg1);

    void on_urunler_table_clicked(const QModelIndex &index);

    void on_uruun_sil_clicked();

    void on_urun_duzenle_clicked();

    void on_noti_button_clicked();

    void on_radioButton_7gun_clicked();

    void on_radioButton_30gun_clicked();

    void on_radioButton_tum_clicked();

    void on_Depo_durumu_activated(int index);

    void on_indirimi_ekle_clicked();

    void on_indirimi_sil_clicked();

    void on_indirimi_duzenleyin_clicked();

    void on_indirim_durumu_activated(int index);

    void on_indirimler_tablo_clicked(const QModelIndex &index);

    void on_sutungrafik_guncelleme_clicked();

    void on_dil_comboBox_activated(int index);

    void on_update_clicked();

    void on_changePass_clicked();

private:
    Ui::MainWindow *ui;

    int FullScreen_tf = 0; // True- false
    QRect m_normalGeometry;

    QSqlQuery *sorgu;
    QSqlQuery *updateQuery;//indirimDurumu() icin sadece
    QSqlQuery *indirimSorgu;

    void HesaplarYazdir();

    void MagazaSayfasi();

    void UrunlerGuncelleme();

    void indirimDurumu();
    void IndirimlerGuncelleme();

    double indirimCalc(QString UrunId,double UrunFiyat);

    NotificationPopup *notificationPopup;

    CiftEkran_Table *Ekran;
    yeniUrun *YeniUrun_Ekrani;
    yeniUrun *UrunEdit_Ekrani;
    indirimler *YeniIndirim_Ekrani;
    indirimler *IndirimEdit_Ekrani;

    void bugunTarihi();

    void analizler(int sec);

    void istatistikler();

    void raporlar();
    void indirimRapolar();
    void depoRaporlar();

    void indirimleri();

    void Pie_Chart();
    void bar_chart();
};
#endif // MAINWINDOW_H
