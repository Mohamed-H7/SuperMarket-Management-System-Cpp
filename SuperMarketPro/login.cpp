#include "login.h"
#include "ui_login.h"

int loginStatus = 0;
int user_type = 0;

QString Username;
QString Ad_Soyad;
QString Email;
QString Telefon;
QString Gorev;

//// Helper Fonk..:
QString generateSHA256Hash(const QString &input)
{
    QByteArray inputData = input.toUtf8();
    QByteArray hash = QCryptographicHash::hash(inputData, QCryptographicHash::Sha256);
    return hash.toHex();
}

bool verifyPassword(const QString &inputPassword, const QString &storedHash)
{
    QString inputHash = generateSHA256Hash(inputPassword);
    return inputHash == storedHash;
}
///

login::login(QSqlDatabase db, QWidget *parent)
    : QDialog(parent), ui(new Ui::login)
{
    ui->setupUi(this);

    sorgu_LO = new QSqlQuery(db);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowTitle("Giriş Ekranı");

    ui->giris_sayflar->setCurrentIndex(0);
}

login::~login()
{
    delete ui;
}

void login::on_kapat_button_clicked()
{
    loginStatus = 0;
    close();
}

void login::on_kaydol_toolButton_clicked()
{
    ui->user_lineEdit->clear();
    ui->sifre_lineEdit->clear();
    ui->giris_durumu->setText(tr("kullanıcı adı veya şifre hatalıdır."));
    ui->giris_durumu->setStyleSheet("color: white");
    ui->giris_sayflar->setCurrentIndex(1);
}

void login::on_giris_toolButton_clicked()
{
    ui->kullanici_ad_lineEdit->clear();
    ui->sifre_lineEdit_2->clear();
    ui->ad_soyad_lineEdit->clear();
    ui->email_lineEdit->clear();
    ui->telefon_lineEdit->clear();
    ui->yonetici_radioButton->setChecked(false);
    ui->Kasiyer_radioButton->setChecked(false);

    ui->Kaydol_durumu->setText(tr("Tüm veriler doldurulmalıdır"));
    ui->Kaydol_durumu->setStyleSheet("color: white");

    ui->giris_sayflar->setCurrentIndex(0);
}

/// Login ve Register:
void login::on_pushButton_clicked()
{
    QString username = ui->user_lineEdit->text();
    QString password = ui->sifre_lineEdit->text();
    QString storedHash = generateSHA256Hash(password);

    sorgu_LO->prepare("select * from Kullanicilar where Kullanici_Adi = ? and Sifre = ?");
    sorgu_LO->addBindValue(username);
    sorgu_LO->addBindValue(storedHash);
    sorgu_LO->exec();

    if (sorgu_LO->next())
    {
        Username = sorgu_LO->value(1).toString();
        Ad_Soyad = sorgu_LO->value(3).toString();
        Email = sorgu_LO->value(4).toString();
        Telefon = sorgu_LO->value(5).toString();
        Gorev = sorgu_LO->value(6).toString();

        QString Kullanici_Gorevi = sorgu_LO->value(6).toString();
        if (Kullanici_Gorevi == "Yonetici")
        {
            user_type = 1;
        }
        else
        {
            user_type = 0;
        }

        QString durumu = sorgu_LO->value(7).toString();
        if (durumu == "pasif")
        {
            ui->giris_durumu->setText(tr("Hesabınız pasif, lütfen yöneticiyle iletişime geçin"));
            ui->giris_durumu->setStyleSheet("color: red");
            return;
        }
        loginStatus = 1;
        accept();
    }
    else
    {
        ui->giris_durumu->setStyleSheet("color: red");
    }
}

void login::on_kaydon_pushButton_clicked()
{
    QString kullanici = ui->kullanici_ad_lineEdit->text();
    QString sifre = ui->sifre_lineEdit_2->text();
    QString SifrelemeHash = generateSHA256Hash(sifre); // Sifreleme
    QString AdSoyad = ui->ad_soyad_lineEdit->text();
    QString email = ui->email_lineEdit->text();
    QString telefon = ui->telefon_lineEdit->text();
    QString gorevi;
    QString durumu = "pasif";
    if (ui->yonetici_radioButton->isChecked())
    {
        gorevi = "Yonetici";
    }
    else if (ui->Kasiyer_radioButton->isChecked())
    {
        gorevi = "Kasiyer";
    }
    else
    {
        ui->Kaydol_durumu->setStyleSheet("color: red");
        return;
    }

    if (kullanici.isEmpty() || sifre.isEmpty() || AdSoyad.isEmpty() || email.isEmpty() || telefon.isEmpty())
    {
        ui->Kaydol_durumu->setStyleSheet("color: red");
        return;
    }

    sorgu_LO->prepare("insert into Kullanicilar (Kullanici_Adi, Sifre, Ad_Soyad, Email, Telefon, Gorev, durum)"
                      " values (?,?,?,?,?,?,?)");
    sorgu_LO->addBindValue(kullanici);
    sorgu_LO->addBindValue(SifrelemeHash);
    sorgu_LO->addBindValue(AdSoyad);
    sorgu_LO->addBindValue(email);
    sorgu_LO->addBindValue(telefon);
    sorgu_LO->addBindValue(gorevi);
    sorgu_LO->addBindValue(durumu);

    if (!sorgu_LO->exec())
    {
        QString txt = "Aşağıdaki hata nedeniyle veriler eklenemedi:\n" + sorgu_LO->lastError().text();
        QMessageBox::information(this, "Hata", txt);
    }
    else
    {
        ui->Kaydol_durumu->setText(tr("Hesap başarıyla oluşturuldu, giriş yapabilirsiniz"));
        ui->Kaydol_durumu->setStyleSheet("color: Green");
    }
}
///
