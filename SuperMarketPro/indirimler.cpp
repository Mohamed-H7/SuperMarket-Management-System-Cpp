#include "indirimler.h"
#include "ui_indirimler.h"
//tr()
struct Categoryy2
{
    QString id;
    QString name;
};
QList<Categoryy2> UKategoriler2;

indirimler::indirimler(QSqlDatabase db, int sec, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::indirimler)
{
    ui->setupUi(this);

    sorgu_indirim = new QSqlQuery(db);
    urunKategori();

    if (sec == 1)
    {
        ui->label->setText(tr("indirimi düzenleyin"));
        ui->ekle_btn->setText("Düzenle");
    }

    ui->baslangic_tarihi->setDate(QDate::currentDate());
    ui->bitis_tarihi->setDate(QDate::currentDate());
    ui->comboBox_Kategori->setDisabled(true);

}

indirimler::~indirimler()
{
    delete ui;
}

//// Helper Fonk..:
void indirimler::urunKategori()
{
    sorgu_indirim->prepare("select * from UrunKategori");
    sorgu_indirim->exec();

    while (sorgu_indirim->next())
    {
        QString KId = sorgu_indirim->value(0).toString();
        QString KAd = sorgu_indirim->value(1).toString();
        ui->comboBox_Kategori->addItem(KAd);

        Categoryy2 Kategori = {KId, KAd};
        UKategoriler2.append(Kategori);
    }
}

//when we call this fonk..
void indirimler::indirimBilgiGuncelleme()
{
    QString format = "yyyy-MM-dd";
    QDate BaslangicTarih = QDate::fromString(Strt_date, format);
    QDate BitisTarih = QDate::fromString(End_date, format);

    ui->indirim_ad->setText(indirim_ad);
    ui->indirim_yuzdesi->setText(indirim_yuzdesi);
    ui->baslangic_tarihi->setDate(BaslangicTarih);
    ui->bitis_tarihi->setDate(BitisTarih);
    ui->urun_numara->setText(urun_numara);

    if(indirim_tipi == 0){ // 0 ise urun tipi
        ui->urun_numara->setText(urun_numara);


        ui->radioButton_urun->setChecked(true);
        ui->radioButton_urun->setDisabled(false);
        ui->radioButton_kategori->setDisabled(true);

        ui->urun_numara->setDisabled(false);
        ui->comboBox_Kategori->setDisabled(true);
        ui->comboBox_Kategori->setCurrentIndex(0);
    }
    else if(indirim_tipi == 1){ //1 ise kategori tipi
        ui->comboBox_Kategori->setCurrentIndex(IKategori);

        ui->radioButton_kategori->setDisabled(false);
        ui->radioButton_kategori->setChecked(true);

        ui->radioButton_urun->setDisabled(true);

        ui->urun_numara->setDisabled(true);
        ui->comboBox_Kategori->setDisabled(false);
        ui->urun_numara->clear();
    }

}
///

void indirimler::on_radioButton_urun_clicked()
{
    ui->urun_numara->setDisabled(false);
    ui->comboBox_Kategori->setDisabled(true);
}

void indirimler::on_radioButton_kategori_clicked()
{
    ui->urun_numara->setDisabled(true);
    ui->comboBox_Kategori->setDisabled(false);
}

/// Buton Fonk..:
void indirimler::on_iptal_btn_clicked()
{
    ui->indirim_ad->clear();
    ui->indirim_yuzdesi->clear();
    ui->baslangic_tarihi->setDate(QDate::currentDate());
    ui->bitis_tarihi->setDate(QDate::currentDate());
    ui->urun_numara->clear();
    ui->comboBox_Kategori->setCurrentIndex(0);
    ui->label_durumu->setStyleSheet("color: white");
    close();
}

void indirimler::on_ekle_btn_clicked()
{

    QString format = "yyyy-MM-dd";
    QDate bugun = QDate::currentDate();

    QString indirimAd = ui->indirim_ad->text();
    int indirimYuzde = ui->indirim_yuzdesi->text().toInt();
    QDate Basla_Tarih = ui->baslangic_tarihi->date();
    QDate Bitis_Tarih = ui->bitis_tarihi->date();

    QString UrunNo;


    QString KategoriAd = ui->comboBox_Kategori->currentText();

    QString Kategori_ID;
    QString UrunId;

    if (indirimAd.isEmpty() || ui->indirim_yuzdesi->text().isEmpty()
     ||  (KategoriAd == "Kategori seçiniz" && ui->radioButton_kategori->isChecked()) ||
        (ui->radioButton_urun->isChecked() && ui->urun_numara->text().isEmpty()))
    {
        ui->label_durumu->setText(tr("Lütfen tüm alanları doldurun"));
        ui->label_durumu->setStyleSheet("color: red");
        return;
    }

    if(Bitis_Tarih < Basla_Tarih){
        ui->label_durumu->setText(tr("Lütfen geçerli bir tarih seçin"));
        ui->label_durumu->setStyleSheet("color: red");
        return;
    }

    if(ui->radioButton_urun->isChecked()){
        UrunNo = ui->urun_numara->text();

        sorgu_indirim->prepare("select * from Urunler where Barkod = ? ");
        sorgu_indirim->addBindValue(UrunNo);
        sorgu_indirim->exec();

        if (sorgu_indirim->next())
        {
            UrunId = sorgu_indirim->value(0).toString();
        }
        else
        {
            ui->label_durumu->setText(tr("Urun Bulanmadi.."));
            ui->label_durumu->setStyleSheet("color: red");
            return;
        }
    }
    else if(ui->radioButton_kategori->isChecked()){
        for (int in = 0; in < UKategoriler2.length(); ++in)
        {
            if (KategoriAd == UKategoriler2[in].name)
            {
                Kategori_ID = UKategoriler2[in].id;
            }
        }
    }


    if (ui->ekle_btn->text() == "Ekle"){

        if(ui->radioButton_urun->isChecked()){
            sorgu_indirim->prepare("INSERT INTO indirimler (urun_id, indirim_adi, indirim_yuzdesi, StartDate, EndDate, indirim_durumu) "
                                   "VALUES (?,?,?,?,?,?)");
            sorgu_indirim->addBindValue(UrunId);
        }
        else if(ui->radioButton_kategori->isChecked()){
            sorgu_indirim->prepare("INSERT INTO indirimler (kategori_id, indirim_adi, indirim_yuzdesi, StartDate, EndDate, indirim_durumu) "
                                   "VALUES (?,?,?,?,?,?)");
            sorgu_indirim->addBindValue(Kategori_ID);
        }

        sorgu_indirim->addBindValue(indirimAd);
        sorgu_indirim->addBindValue(indirimYuzde);
        sorgu_indirim->addBindValue(Basla_Tarih.toString(format));
        sorgu_indirim->addBindValue(Bitis_Tarih.toString(format));

        QString indirimDurumu;
        if(bugun >= Basla_Tarih && bugun < Bitis_Tarih){
            indirimDurumu = "mevcut";
        }
        else if(bugun < Basla_Tarih){
            indirimDurumu = "baslamadi";
        }
        else if(bugun > Bitis_Tarih){
            indirimDurumu = "bitti";
        }
        else if(bugun == Bitis_Tarih){
            indirimDurumu = "bitecek";
        }
        sorgu_indirim->addBindValue(indirimDurumu);

        if (!sorgu_indirim->exec())
        {
            QMessageBox::information(this, "Hata", "indirim eklenemedi: " + sorgu_indirim->lastError().text());
            return;
        }

        ui->label_durumu->setText(tr("indirim başarıyla eklendi"));
        ui->label_durumu->setStyleSheet("color: green");

        ui->indirim_ad->clear();
        ui->indirim_yuzdesi->clear();
        ui->baslangic_tarihi->setDate(QDate::currentDate());
        ui->bitis_tarihi->setDate(QDate::currentDate());
        ui->urun_numara->clear();
    }
    else if (ui->ekle_btn->text() == "Düzenle"){


        if(ui->radioButton_urun->isChecked()){
            sorgu_indirim->prepare("UPDATE indirimler SET "
                                   "urun_id = ?, indirim_adi = ?,"
                                   "indirim_yuzdesi = ?, StartDate = ?, EndDate = ?, indirim_durumu = ? "
                                   "WHERE indirim_ID = ?");
            sorgu_indirim->addBindValue(UrunId);
        }
        else if(ui->radioButton_kategori->isChecked()){
            sorgu_indirim->prepare("UPDATE indirimler SET "
                                   "kategori_id = ?, indirim_adi = ?,"
                                   "indirim_yuzdesi = ?, StartDate = ?, EndDate = ?, indirim_durumu = ? "
                                   "WHERE indirim_ID = ?");
            sorgu_indirim->addBindValue(Kategori_ID);
        }

        sorgu_indirim->addBindValue(indirimAd);
        sorgu_indirim->addBindValue(indirimYuzde);
        sorgu_indirim->addBindValue(Basla_Tarih.toString(format));
        sorgu_indirim->addBindValue(Bitis_Tarih.toString(format));

        QString indirimDurumu;
        if(bugun >= Basla_Tarih && bugun < Bitis_Tarih){
            indirimDurumu = "mevcut";
        }
        else if(bugun < Basla_Tarih){
            indirimDurumu = "baslamadi";
        }
        else if(bugun > Bitis_Tarih){
            indirimDurumu = "bitti";
        }
        else if(bugun == Bitis_Tarih){
            indirimDurumu = "bitecek";
        }
        sorgu_indirim->addBindValue(indirimDurumu);
        sorgu_indirim->addBindValue(indirim_id);

        if (!sorgu_indirim->exec())
        {
            QMessageBox::information(this, "Hata", "indirim guncellmedi: " + sorgu_indirim->lastError().text());
            return;
        }

        ui->label_durumu->setText(tr("indirim başarıyla guncelledi"));
        ui->label_durumu->setStyleSheet("color: green");
    }

    emit IndirimEkledi();

}
///
