#include "yeniurun.h"
#include "ui_yeniurun.h"

struct Categoryy
{
    QString id;
    QString name;
};
QList<Categoryy> UKategoriler;

yeniUrun::yeniUrun(QSqlDatabase db, int sec, QWidget *parent)
    : QDialog(parent), ui(new Ui::yeniUrun)
{
    ui->setupUi(this);

    sorgu_urun = new QSqlQuery(db);
    urunKategori();

    if (sec == 1)
    {
        ui->label->setText(tr("Ürünü düzenleyin"));
        ui->pushButton->setText(tr("Düzenle"));
    }

    // parent->tstst();
}

yeniUrun::~yeniUrun()
{
    delete ui;
}

void yeniUrun::urunKategori()
{
    sorgu_urun->prepare("select * from UrunKategori");
    sorgu_urun->exec();

    while (sorgu_urun->next())
    {
        QString KId = sorgu_urun->value(0).toString();
        QString KAd = sorgu_urun->value(1).toString();
        ui->comboBox_Kategori->addItem(KAd);

        Categoryy Kategori = {KId, KAd};
        UKategoriler.append(Kategori);
    }
}

void yeniUrun::urunBilgiGuncelleme()
{
    ui->urunAd->setText(UAd);
    ui->urunBarkod->setText(UBarkod);
    ui->urunFiyat->setText(UFiyat);
    ui->urunMaliyeti->setText(UMaliyet);
    ui->urunMiktar->setText(UStok);
    ui->comboBox_Kategori->setCurrentIndex(UKategori);
}

void yeniUrun::on_pushButton_2_clicked() // Iptal Buton
{
    ui->urunAd->clear();
    ui->urunBarkod->clear();
    ui->urunFiyat->clear();
    ui->urunMiktar->clear();
    ui->urunMaliyeti->clear();
    ui->comboBox_Kategori->setCurrentIndex(0);
    ui->label_durumu->setStyleSheet("color: white");
    close();
}

void yeniUrun::on_pushButton_clicked() // Eklek Buton ve Düzenle
{
    QString UrunAd = ui->urunAd->text();
    QString Barkod = ui->urunBarkod->text();
    QString Urun_Fiyati = ui->urunFiyat->text();
    QString Urunun_Maliyeti = ui->urunMaliyeti->text();
    QString Miktar_Stokta = ui->urunMiktar->text();
    QString KAd = ui->comboBox_Kategori->currentText();

    if (UrunAd.isEmpty() || Barkod.isEmpty() || Urun_Fiyati.isEmpty() || Urunun_Maliyeti.isEmpty() || Miktar_Stokta.isEmpty() || KAd == "Kategori seçiniz")
    {
        ui->label_durumu->setText(tr("Lütfen tüm alanları doldurun"));
        ui->label_durumu->setStyleSheet("color: red");
        return;
    }

    if (Barkod == UBarkod)
    {
    }
    else
    {
        sorgu_urun->prepare("select * from Urunler where Barkod = ?");
        sorgu_urun->addBindValue(Barkod);
        sorgu_urun->exec();
        if (sorgu_urun->next())
        {
            ui->label_durumu->setText(tr("Barkod kullanılıyor, lütfen değiştirin"));
            ui->label_durumu->setStyleSheet("color: red");
            return;
        }
    }

    QString Kategori_ID;
    for (int in = 0; in < UKategoriler.length(); ++in)
    {
        if (KAd == UKategoriler[in].name)
        {
            Kategori_ID = UKategoriler[in].id;
        }
    }

    if (ui->pushButton->text() == "Düzenle" || ui->pushButton->text() == "Edit")
    {
        sorgu_urun->prepare("UPDATE Urunler SET "
                            "Urun_Adi = ?, Barkod = ?, Urun_Fiyati = ?,"
                            "Urunun_Maliyeti = ?, Miktar_Stokta = ?, Kategori_ID = ? "
                            "WHERE Urun_ID = ?");
        sorgu_urun->addBindValue(UrunAd);
        sorgu_urun->addBindValue(Barkod);
        sorgu_urun->addBindValue(Urun_Fiyati);
        sorgu_urun->addBindValue(Urunun_Maliyeti);
        sorgu_urun->addBindValue(Miktar_Stokta);
        sorgu_urun->addBindValue(Kategori_ID);
        sorgu_urun->addBindValue(UID); //

        if (!sorgu_urun->exec())
        {
            QMessageBox::information(this, "Hata", "urun guncellmedi: " + sorgu_urun->lastError().text());
            return;
        }

        ui->label_durumu->setText(tr("Ürün başarıyla guncelledi"));
        ui->label_durumu->setStyleSheet("color: green");
        // return;
    }
    else
    {
        sorgu_urun->prepare("INSERT INTO Urunler (Urun_Adi, Barkod, Urun_Fiyati, Urunun_Maliyeti, Miktar_Stokta, Kategori_ID) "
                            "VALUES (?,?,?,?,?,?)");
        sorgu_urun->addBindValue(UrunAd);
        sorgu_urun->addBindValue(Barkod);
        sorgu_urun->addBindValue(Urun_Fiyati);
        sorgu_urun->addBindValue(Urunun_Maliyeti);
        sorgu_urun->addBindValue(Miktar_Stokta);
        sorgu_urun->addBindValue(Kategori_ID);

        if (!sorgu_urun->exec())
        {
            QMessageBox::information(this, "Hata", "urun eklenemedi: " + sorgu_urun->lastError().text());
            return;
        }

        ui->label_durumu->setText(tr("Ürün başarıyla eklendi"));
        ui->label_durumu->setStyleSheet("color: green");

        ui->urunAd->clear();
        ui->urunBarkod->clear();
        ui->urunFiyat->clear();
        ui->urunMiktar->clear();
        ui->urunMaliyeti->clear();
        ui->comboBox_Kategori->setCurrentIndex(0);
    }

    emit UrunuEkledi();
}
