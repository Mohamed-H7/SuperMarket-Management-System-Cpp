#include "mainwindow.h"
#include "ui_mainwindow.h"

/// called global varibales in another files
extern QString Username;
extern QString Ad_Soyad;
extern QString Email;
extern QString Telefon;
extern QString Gorev;
extern int user_type;
///

struct Category
{
    QString id;
    QString name;
};

////Global varibales:
int urunler_list_tablo_index = -1;
QStandardItemModel *model;

int hesaplar_tablo_index = -1;
QStandardItemModel *Hesaplar_model;
QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel();

QStandardItemModel *Urunler_model;
QSortFilterProxyModel *UrunFiltreModel = new QSortFilterProxyModel();
QSortFilterProxyModel *FiltreModelSeachBar = new QSortFilterProxyModel();
QList<Category> Kategoriler;
int urunler_table_index = -1;

QStandardItemModel *Depo_model;
QSortFilterProxyModel *DepoFiltreModel = new QSortFilterProxyModel();

int indirim_tablo_index = -1;
QStandardItemModel *indirimler_model;
QSortFilterProxyModel *indirimlerFiltreModel = new QSortFilterProxyModel();

double KDV = 0.15;
double Toplam = 0;
double ToplamKar = 0;

QString TT = "Toplam Tutar: 0 TL";

QSqlDatabase veriTabanin;

QList<QBarSet*> sets; //Bar char icin
///

MainWindow::MainWindow(QSqlDatabase db, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), notificationPopup(new NotificationPopup(this))

{
    ui->setupUi(this);

    //Settign for proje Windows:
    // Sonra acacagim bittigiimde:
    // this->setWindowFlags(Qt::FramelessWindowHint);
    // this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowTitle("SuperMarketPro");
    QIcon icon(":/logo/icon/logo/ico.ico");
    this->setWindowIcon(icon);

    //For DataBase:
    veriTabanin = db;
    sorgu = new QSqlQuery(db);
    updateQuery = new QSqlQuery(db);//indirimDurumu icin sadece

    //check User Type:
    if (user_type == 0)
    {
        ui->KontrolPaneli->setEnabled(false);
        ui->KontrolPaneli_ic->setEnabled(false);
    }

    //ayarlar Sayfasinda:
    ui->label_message->setText(tr("Merhaba ")+ Ad_Soyad);
    ui->lineEdit_username->setText(Username);
    ui->label_gorev->setText(tr("Görev: ")+ Gorev);
    ui->lineEdit_Email->setText(Email);
    ui->lineEdit_num->setText(Telefon);
    ui->lineEdit_AdSoyad->setText(Ad_Soyad);

    //Left Menu:
    ui->icon_name->setVisible(false);
    ui->anasayfa->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);

    //Kasiyer Tablosu:
    model = new QStandardItemModel(0, 3, this);
    model->setHorizontalHeaderLabels({tr("Ürün"), tr("Miktar"), tr("Fiyat")});
    ui->urunler_list->setModel(model);
    ui->urunler_list->setColumnWidth(1, 70);
    ui->urunler_list->setColumnWidth(2, 100);

    //Ihtiyacimiz ekrani:
    Ekran = new CiftEkran_Table(model, this);
    YeniUrun_Ekrani = new yeniUrun(veriTabanin);
    UrunEdit_Ekrani = new yeniUrun(veriTabanin, 1, this);
    YeniIndirim_Ekrani = new indirimler(veriTabanin);
    IndirimEdit_Ekrani = new indirimler(veriTabanin, 1);

    //Notification btn:
    notificationPopup->addNotification(tr("HosGeldiniz..!"));
    notificationPopup->addNotification(tr("Yeni raporlar alındı, bunlara (Raporlar ve Analizler) sayfasından ulaşabilirsiniz."));
    QIcon Notiicon(":/noti icons/icon/Notif icon/002-active.png");
    ui->noti_button->setIcon(Notiicon);

    //Fonkisyonlar:
    MagazaSayfasi();//Select Urunlar for Magaza Sayfasi
    HesaplarYazdir();//select Kullanicilar for kontrol panel for yonteci
    indirimDurumu();//indirim Durumu kontrol etmek ve guncellemek
    indirimleri();//select indirimler for indirim sayfasi

    //raporlar sayfasi:
    analizler(0);
    istatistikler();
    raporlar();
    indirimRapolar();
    depoRaporlar();

    Pie_Chart();
    bar_chart();

    //dil icin:
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
        file.close();
    }
    if(firstLine == "English"){
        ui->dil_comboBox->setCurrentIndex(1);
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

////Yardimci Fonkisyonlar:
void UrunuTabloyaekle(const QString &urun, QString &miktar, double fiyat,int indirim = 0)
{
    Q_UNUSED(indirim);//
    int rowCount = model->rowCount();
    int nextRow = rowCount;

    if (nextRow >= model->rowCount())
    {
        model->insertRow(nextRow);
    }

    QString TFiyat = QString::number(fiyat, 'f', 2) + " TL"; ////////

    QStandardItem *item = new QStandardItem();
    item->setText(urun);
    item->setTextAlignment(Qt::AlignCenter);
    model->setItem(nextRow, 0, item);

    QStandardItem *item2 = new QStandardItem();
    item2->setText(miktar);
    item2->setTextAlignment(Qt::AlignCenter);
    model->setItem(nextRow, 1, item2);

    QStandardItem *item3 = new QStandardItem();
    item3->setText(TFiyat);
    item3->setTextAlignment(Qt::AlignCenter);
    model->setItem(nextRow, 2, item3); ////////
}

int findNextAvailableId()
{
    QSqlQuery *query;
    query = new QSqlQuery(veriTabanin);
    query->prepare("SELECT Alisveris_ID FROM AlisversiKayit ORDER BY Alisveris_ID ASC");

    if (!query->exec())
    {
        qDebug() << "Error executing query: " << query->lastError();
        return -1;
    }

    int expectedId = 1;
    while (query->next())
    {
        int currentId = query->value(0).toInt();
        if (currentId != expectedId)
        {
            break;
        }
        expectedId++;
    }

    return expectedId;
}

void MainWindow::HesaplarYazdir()
{
    Hesaplar_model = new QStandardItemModel(0, 5, this);
    Hesaplar_model->setHorizontalHeaderLabels({tr("Kullanıcı Adı"), tr("Ad Soyad"), tr("Email"),
                                               tr("Telefon"), tr("Hesap Durumu")});

    sorgu->prepare("select * from Kullanicilar");
    sorgu->exec();

    int row_index = 0;
    int rowC = 0;
    while (sorgu->next())
    {
        Hesaplar_model->insertRow(++rowC);
        QString Dizi[5];
        Dizi[0] = sorgu->value(1).toString(); // kullaniciadi
        Dizi[1] = sorgu->value(3).toString(); // AdSoyad
        Dizi[2] = sorgu->value(4).toString(); // Email
        Dizi[3] = sorgu->value(5).toString(); // Telefon
        Dizi[4] = sorgu->value(7).toString(); // HesapDurumu

        for (int var = 0; var < 5; ++var)
        {
            QStandardItem *veri = new QStandardItem();
            veri->setText(Dizi[var]);
            veri->setTextAlignment(Qt::AlignCenter);

            if (Dizi[var] == "aktif")
            {
                veri->setBackground(QColor(Qt::green));
            }
            else if (Dizi[var] == "pasif")
            {
                veri->setBackground(QColor(Qt::red));
            }

            Hesaplar_model->setItem(row_index, var, veri);
        }
        row_index++;
    }

    ui->tableView_hesaplar->setModel(Hesaplar_model);
    ui->tableView_hesaplar->setColumnWidth(0, 170);
    ui->tableView_hesaplar->setColumnWidth(1, 170);
    ui->tableView_hesaplar->setColumnWidth(2, 300);
    ui->tableView_hesaplar->setColumnWidth(3, 100);
    ui->tableView_hesaplar->setColumnWidth(4, 150);
}

void MainWindow::MagazaSayfasi()
{
    sorgu->prepare("select * from UrunKategori");
    sorgu->exec();

    while (sorgu->next())
    {
        QString KId = sorgu->value(0).toString();
        QString KAd = sorgu->value(1).toString();
        ui->kategori_urunler->addItem(KAd);

        Category Kategori = {KId, KAd};
        Kategoriler.append(Kategori);

        QBarSet *set = new QBarSet(KAd);
        sets.append(set);
    }
    ////
    Urunler_model = new QStandardItemModel(0, 7, this);
    Urunler_model->setHorizontalHeaderLabels({tr("Urun ID"), tr("Urun Adi"), tr("Barkod"), tr("Urun Fiyati"),
                    tr("Urunun Maliyeti"), tr("Miktar Stokta"), tr("Kategori")});

    sorgu->prepare("select * from Urunler");
    sorgu->exec();

    int row_index = 0;
    int rowC = 0;
    while (sorgu->next())
    {
        Urunler_model->insertRow(++rowC);
        QString Dizi[7];
        Dizi[0] = sorgu->value(0).toString(); // Urun ID
        Dizi[1] = sorgu->value(1).toString(); // Urun Adi
        Dizi[2] = sorgu->value(2).toString(); // Barkod
        Dizi[3] = sorgu->value(3).toString(); // Urun_Fiyati
        Dizi[4] = sorgu->value(4).toString(); // Urunun_Maliyeti
        Dizi[5] = sorgu->value(5).toString(); // Miktar Stokta
        Dizi[6] = sorgu->value(6).toString(); // Kategori

        for (int var = 0; var < 7; ++var)
        {
            QStandardItem *veri = new QStandardItem();

            if (var == 6)
            {
                for (int in = 0; in < Kategoriler.length(); ++in)
                {
                    if (Dizi[var] == Kategoriler[in].id)
                    {
                        Dizi[var] = Kategoriler[in].name;
                    }
                }
            }

            veri->setText(Dizi[var]);
            veri->setTextAlignment(Qt::AlignCenter);

            Urunler_model->setItem(row_index, var, veri);
        }
        row_index++;
    }

    ui->urunler_table->setModel(Urunler_model);
    // ui->urunler_table->setColumnWidth(0, 170);
}

void MainWindow::UrunlerGuncelleme()
{
    Urunler_model->setRowCount(0);

    sorgu->prepare("select * from Urunler");
    sorgu->exec();

    int row_index = 0;
    int rowC = 0;
    while (sorgu->next())
    {
        Urunler_model->insertRow(++rowC);
        QString Dizi[7];
        Dizi[0] = sorgu->value(0).toString(); // Urun ID
        Dizi[1] = sorgu->value(1).toString(); // Urun Adi
        Dizi[2] = sorgu->value(2).toString(); // Barkod
        Dizi[3] = sorgu->value(3).toString(); // Urun_Fiyati
        Dizi[4] = sorgu->value(4).toString(); // Urunun_Maliyeti
        Dizi[5] = sorgu->value(5).toString(); // Miktar Stokta
        Dizi[6] = sorgu->value(6).toString(); // Kategori

        for (int var = 0; var < 7; ++var)
        {
            QStandardItem *veri = new QStandardItem();

            if (var == 6)
            {
                for (int in = 0; in < Kategoriler.length(); ++in)
                {
                    if (Dizi[var] == Kategoriler[in].id)
                    {
                        Dizi[var] = Kategoriler[in].name;
                    }
                }
            }

            veri->setText(Dizi[var]);
            veri->setTextAlignment(Qt::AlignCenter);

            Urunler_model->setItem(row_index, var, veri);
        }
        row_index++;
    }

    ui->kategori_urunler->setCurrentIndex(0);
    ui->lineEdit_urun_arayin->clear();
    ui->urunler_table->setModel(Urunler_model);
}

void MainWindow::indirimDurumu()
{
    QDate bugun = QDate::currentDate();

    sorgu->prepare("select * from indirimler");
    if (!sorgu->exec())
    {
        qDebug() << "Error indirimDurumu() query: " << sorgu->lastError();
        return;
    }

    while (sorgu->next()) {
        QString indirim_ID = sorgu->value(0).toString();
        QDate startDate = sorgu->value(5).toDate();
        QDate endDate = sorgu->value(6).toDate();
        QString indirim_durumu = sorgu->value(7).toString();

        QString yeniDurum;
        if(bugun >= startDate && bugun < endDate){
            yeniDurum = "mevcut";
        }
        else if(bugun < startDate){
            yeniDurum = "baslamadi";
        }
        else if(bugun > endDate){
            yeniDurum = "bitti";
        }
        else if(bugun == endDate){
            yeniDurum = "bitecek";
        }

        if (yeniDurum != indirim_durumu) {
            qDebug()<<indirim_ID;

            updateQuery->prepare("UPDATE indirimler SET indirim_durumu = ? WHERE indirim_ID = ? ");
            updateQuery->addBindValue(yeniDurum);
            updateQuery->addBindValue(indirim_ID);
            if (!updateQuery->exec())
            {
                qDebug() << "Error update in indirimDurumu(): " << updateQuery->lastError();
                return;
            }
        }
    }
}

void MainWindow::IndirimlerGuncelleme()
{
    indirimler_model->setRowCount(0);

    sorgu->prepare("SELECT indirimler.indirim_ID, indirimler.indirim_adi, "
                   "indirimler.indirim_yuzdesi, "
                   "indirimler.StartDate, "
                   "indirimler.EndDate, "
                   "indirimler.indirim_durumu, "
                   "urunler.urun_adi, "
                   "UrunKategori.kategori_adi "
                   "FROM indirimler "
                   "LEFT JOIN urunler ON indirimler.urun_id = urunler.urun_id "
                   "LEFT JOIN UrunKategori ON indirimler.kategori_id = UrunKategori.kategori_id "
                   "WHERE (indirimler.urun_id IS NOT NULL AND urunler.urun_id IS NOT NULL) "
                   "OR (indirimler.kategori_id IS NOT NULL AND UrunKategori.kategori_id IS NOT NULL) "
                   );
    sorgu->exec();

    int row_index = 0;
    int rowC = 0;
    while (sorgu->next())
    {
        indirimler_model->insertRow(++rowC);
        QString Dizi[7];
        Dizi[0] = sorgu->value(0).toString(); //indirim ID
        Dizi[1] = sorgu->value(1).toString(); //indirim Adi
        Dizi[2] = sorgu->value(2).toString(); //indirim Yuzdesi
        Dizi[3] = sorgu->value(3).toString(); //Başlangıç ​​tarihi
        Dizi[4] = sorgu->value(4).toString(); //Bitiş tarihi
        Dizi[5] = sorgu->value(5).toString(); //indirim durumu

        //İndirimin uygulandığı:
        if(sorgu->value(6).toString() == ""){
            Dizi[6] = sorgu->value(7).toString();
        }
        else{
            Dizi[6] = sorgu->value(6).toString();
        }

        for (int var = 0; var < 7; ++var)
        {
            QStandardItem *veri = new QStandardItem();
            veri->setText(Dizi[var]);
            veri->setTextAlignment(Qt::AlignCenter);
            if (var == 5)
            {
                if (Dizi[var] == "mevcut")
                {
                    veri->setBackground(QColor(Qt::green));
                }
                else if (Dizi[var] == "bitti")
                {
                    veri->setBackground(QColor(Qt::red));
                }
            }
            indirimler_model->setItem(row_index, var, veri);
        }
        row_index++;
    }

    ui->indirim_durumu->setCurrentIndex(0);
    ui->indirimler_tablo->setModel(indirimler_model);

}

double MainWindow::indirimCalc(QString UrunId,double UrunFiyat)
{
    double indirim_yuzdesi;
    double indirimTutari;

    indirimSorgu = new QSqlQuery(veriTabanin);

    QString urunKategori;
    indirimSorgu->prepare("select * from Urunler where urun_id = ?");
    indirimSorgu->addBindValue(UrunId);

    if (!indirimSorgu->exec())
    {
        qDebug() << "Error indirimCalc(): " << indirimSorgu->lastError();
        return 0;
    }

    if(indirimSorgu->next()){
        urunKategori = indirimSorgu->value(6).toString();
    }

    indirimSorgu->prepare("select indirim_yuzdesi from indirimler where (urun_id = ? or kategori_id = ?)  "
                          "AND (indirim_durumu = 'bitecek' OR indirim_durumu = 'mevcut')");
    indirimSorgu->addBindValue(UrunId);
    indirimSorgu->addBindValue(urunKategori);

    if (!indirimSorgu->exec())
    {
        qDebug() << "Error indirimCalc(): " << indirimSorgu->lastError();
        return 0;
    }

    if(indirimSorgu->next()){
        indirim_yuzdesi = indirimSorgu->value(0).toInt();
        indirimTutari = UrunFiyat * (indirim_yuzdesi/100);

    }
    else{
        return 0;
    }

    return indirimTutari;
}

void MainWindow::bugunTarihi()
{
    QDate bugun = QDate::currentDate();
    QString tarih = tr("Bugün, ") + bugun.toString("d MMM yyyy");
    ui->tarih_label->setText(tarih);
}

void MainWindow::analizler(int sec)
{
    /////////////////////////////////////////////////////////
    QString format = "yyyy-MM-dd";
    QDate bugun = QDate::currentDate();
    QDate raporGun;

    // QString txt = "Toplam Satış\n\n";

    double totalSales = 0;
    double maxSatis = 0;
    double minSatis = 0;
    int urunSayisi = 0;
    double toplamKar = 0;

    int trr = 0;

    if (sec == 7)
    { // 7 Gun
        raporGun = bugun.addDays(-7);
    }
    else if (sec == 30)
    { // 30 gun
        raporGun = bugun.addDays(-30);
    }
    else if (sec == 0)
    { // Tum
        sorgu->prepare("SELECT SUM(ToplamMiktar) AS TotalSales FROM AlisversiKayit");
        if (sorgu->exec() && sorgu->next())
        {
            totalSales = sorgu->value("TotalSales").toDouble();
        }
        else
        {
            qDebug() << "Error: " << sorgu->lastError().text();
        }
        ui->TotalSatis_label->setText(tr("Toplam Satış\n\n") + QString::number(totalSales) + " TL");
        ///////////////
        sorgu->prepare("SELECT * FROM AlisversiKayit");
        if (!sorgu->exec())
        {
            qDebug() << "Error executing query: " << sorgu->lastError();
            return;
        }

        maxSatis = 0;
        minSatis = 0;
        trr = 0;
        while (sorgu->next())
        {
            if (trr == 0)
            {
                minSatis = sorgu->value(1).toDouble();
                trr = 1;
            }

            if (maxSatis < sorgu->value(1).toDouble())
                maxSatis = sorgu->value(1).toDouble();

            if (minSatis >= sorgu->value(1).toDouble())
                minSatis = sorgu->value(1).toDouble();
        }
        ui->AzSatis_label->setText(tr("En az satış\n\n") + QString::number(minSatis) + " TL"); //,'f',2
        ui->CokSatis_label->setText(tr("En çok satış\n\n") + QString::number(maxSatis) + " TL");
        /////////////
        urunSayisi = 0;
        sorgu->prepare("SELECT SUM(Urun_Miktari) AS UrunSayisi FROM AlisversiDetaylar");

        if (sorgu->exec() && sorgu->next())
        {
            urunSayisi = sorgu->value("UrunSayisi").toInt();
        }
        else
        {
            qDebug() << "Error UrunSayisi:" << sorgu->lastError().text();
            return;
        }
        ui->SatilanUrunSayisi_label->setText(tr("Satılan ürün sayısı\n\n") + QString::number(urunSayisi)); // label
        //////////////////
        sorgu->prepare("SELECT SUM(ad.ToplamFiyat) - SUM(ad.Urun_Miktari * u.Urunun_Maliyeti) AS TotalProfit "
                       "FROM AlisversiDetaylar ad "
                       "JOIN Urunler u ON ad.Urun_ID = u.Urun_ID;");

        if (sorgu->exec() && sorgu->next())
        {
            toplamKar = sorgu->value("TotalProfit").toDouble();
            // qDebug() << "Total Profit:" << totalProfit;
        }
        else
        {
            qDebug() << "Error calculating total profit:" << sorgu->lastError().text();
            return;
        }

        ToplamKar = toplamKar;
        ui->Tkar_label->setText(tr("Toplam Kar\n\n") + QString::number(toplamKar) + " TL");
        return;
    }
    else
    {
        ui->TotalSatis_label->setText(tr("Toplam Satış\n\n0 TL"));
        ui->Tkar_label->setText(tr("Toplam Kar\n\n0 TL"));
        ui->SatilanUrunSayisi_label->setText(tr("Satılan ürün sayısı\n\n0"));
        ui->AzSatis_label->setText(tr("Daha az satış\n\n0 TL")); //,'f',2
        ui->CokSatis_label->setText(tr("En çok satış\n\n0 TL"));
        return;
    }

    sorgu->prepare("SELECT SUM(ToplamMiktar) AS TotalSales FROM AlisversiKayit WHERE DATE(Tarih) BETWEEN ? AND ?");
    sorgu->addBindValue(raporGun.toString(format));
    sorgu->addBindValue(bugun.toString(format));

    if (sorgu->exec() && sorgu->next())
    {
        totalSales = sorgu->value("TotalSales").toDouble();
    }
    else
    {
        qDebug() << "Error: " << sorgu->lastError().text();
        return;
    }
    ui->TotalSatis_label->setText(tr("Toplam Satış\n\n") + QString::number(totalSales) + " TL"); // lable

    // En çok satış && En az satış:
    sorgu->prepare("SELECT * FROM AlisversiKayit WHERE DATE(Tarih) BETWEEN ? AND ?");
    sorgu->addBindValue(raporGun.toString(format));
    sorgu->addBindValue(bugun.toString(format));
    if (!sorgu->exec())
    {
        qDebug() << "Error executing query: " << sorgu->lastError();
        return;
    }

    maxSatis = 0;
    minSatis = 0;
    trr = 0;
    while (sorgu->next())
    {
        if (trr == 0)
        {
            minSatis = sorgu->value(1).toDouble();
            trr = 1;
        }

        if (maxSatis < sorgu->value(1).toDouble())
            maxSatis = sorgu->value(1).toDouble();

        if (minSatis >= sorgu->value(1).toDouble())
            minSatis = sorgu->value(1).toDouble();
    }

    ui->AzSatis_label->setText(tr("Daha az satış\n\n") + QString::number(minSatis) + " TL"); //,'f',2
    ui->CokSatis_label->setText(tr("En çok satış\n\n") + QString::number(maxSatis) + " TL");
    //

    urunSayisi = 0;
    sorgu->prepare("SELECT SUM(ad.Urun_Miktari) AS UrunSayisi FROM AlisversiDetaylar ad "
                   "JOIN AlisversiKayit ak ON ad.Alisveris_ID = ak.Alisveris_ID "
                   "WHERE DATE(Tarih) BETWEEN DATE(?) AND DATE(?)");
    sorgu->addBindValue(raporGun.toString(format));
    sorgu->addBindValue(bugun.toString(format));

    if (sorgu->exec() && sorgu->next())
    {
        urunSayisi = sorgu->value("UrunSayisi").toDouble();
    }
    else
    {
        qDebug() << "Error calculating total profit:" << sorgu->lastError().text();
    }
    ui->SatilanUrunSayisi_label->setText("Satılan ürün sayısı\n\n" + QString::number(urunSayisi));
    // t
    //
    sorgu->prepare("SELECT SUM(ad.ToplamFiyat) - SUM(ad.Urun_Miktari * u.Urunun_Maliyeti) AS TotalProfit "
                   "FROM AlisversiDetaylar ad "
                   "JOIN Urunler u ON ad.Urun_ID = u.Urun_ID "
                   "JOIN AlisversiKayit ak ON ad.Alisveris_ID = ak.Alisveris_ID "
                   "WHERE DATE(Tarih) BETWEEN DATE(?) AND DATE(?)");
    sorgu->addBindValue(raporGun.toString(format));
    sorgu->addBindValue(bugun.toString(format));

    if (sorgu->exec() && sorgu->next())
    {
        toplamKar = sorgu->value("TotalProfit").toDouble();
    }
    else
    {
        qDebug() << "Error calculating total profit:" << sorgu->lastError().text();
    }

    ui->Tkar_label->setText(tr("Toplam Kar\n\n") + QString::number(toplamKar) + " TL");
}

void MainWindow::istatistikler()
{
    QString format = "yyyy-MM-dd";
    QDate bugun = QDate::currentDate();
    QString En_eskiTarih;

    sorgu->prepare("SELECT MIN(DATE(Tarih)) AS OldestDate FROM AlisversiKayit");

    if (sorgu->exec() && sorgu->next())
    {
        En_eskiTarih = sorgu->value("OldestDate").toString();
    }
    else
    {
        qDebug() << "Error calculating average monthly sales:" << sorgu->lastError().text();
        return;
    }

    QDate oldDate = QDate::fromString(En_eskiTarih, format);
    if (!oldDate.isValid())
    {
        qDebug() << "Invalid date format!";
        return;
    }

    int daysDifference = oldDate.daysTo(bugun);
    int monthsDifference = (bugun.year() - oldDate.year()) * 12 + (bugun.month() - oldDate.month());
    int yearsDifference = bugun.year() - oldDate.year();
    int weeksDifference = daysDifference / 7;

    double totalSales = 0;
    double Gunluk_Ortalama = 0;
    double Haftalik_Ortalama = 0;
    double Aylik_Ortalama = 0;
    double Yilik_Ortalama = 0;

    sorgu->prepare("SELECT SUM(ToplamMiktar) AS ToplamSales FROM AlisversiKayit WHERE DATE(Tarih) BETWEEN DATE(?) AND DATE(?)");
    sorgu->addBindValue(oldDate.toString(format));
    sorgu->addBindValue(bugun.toString(format));

    if (sorgu->exec() && sorgu->next())
    {
        totalSales = sorgu->value("ToplamSales").toDouble();
    }
    else
    {
        qDebug() << "Error calculating average yearly sales:" << sorgu->lastError().text();
        return;
    }

    Gunluk_Ortalama = totalSales / daysDifference;
    Haftalik_Ortalama = totalSales / weeksDifference;
    Aylik_Ortalama = totalSales / monthsDifference;
    Yilik_Ortalama = totalSales / (yearsDifference == 0 ? (yearsDifference + 1) : yearsDifference); // (yearsDifference+1)

    QString txt = tr("Ortalama satışlar:\n\n- Günlük: ") + QString::number(Gunluk_Ortalama, 'f', 2) +
                  tr(" TL\n\n- Haftalık: ") + QString::number(Haftalik_Ortalama, 'f', 2) +
                  tr(" TL \n\n- Aylık: ") + QString::number(Aylik_Ortalama, 'f', 2) +
                  tr(" TL\n\n- Yıllık: ") + QString::number(Yilik_Ortalama, 'f', 2) + " TL";
    ui->ortalama_label->setText(txt);
}

void MainWindow::raporlar()
{
    //En az satan ürün
    sorgu->prepare("SELECT u.Urun_ID, u.Urun_Adi, SUM(ad.Urun_Miktari) AS TotalSold "
                   "FROM Urunler u "
                   "JOIN AlisversiDetaylar ad ON u.Urun_ID = ad.Urun_ID "
                   "GROUP BY u.Urun_ID, u.Urun_Adi "
                   "ORDER BY TotalSold ASC "
                   "LIMIT 1;");

    if (sorgu->exec() && sorgu->next()){
        //d
        QString urunAdi = sorgu->value("Urun_Adi").toString();
        int totalSold = sorgu->value("TotalSold").toInt();

        ui->EnAzSatilanUrun->setText(tr("En az satan ürün\n\n(") + urunAdi +
                                     "), " + QString::number(totalSold) + tr(" ürün"));
    }
    else {
        qDebug() << "Error finding least sold product:" << sorgu->lastError().text();
    }

    //En çok satan ürün
    sorgu->prepare("SELECT u.Urun_ID, u.Urun_Adi, SUM(ad.Urun_Miktari) AS TotalSold "
                   "FROM Urunler u "
                   "JOIN AlisversiDetaylar ad ON u.Urun_ID = ad.Urun_ID "
                   "GROUP BY u.Urun_ID, u.Urun_Adi "
                   "ORDER BY TotalSold DESC "
                   "LIMIT 1;");

    if (sorgu->exec() && sorgu->next()){
        //d
        QString urunAdi = sorgu->value("Urun_Adi").toString();
        int totalSold = sorgu->value("TotalSold").toInt();

        ui->EnCokSatilanUrun->setText(tr("En çok satan ürün\n\n(") + urunAdi +
                                      "), " + QString::number(totalSold) + tr(" ürün"));
    }
    else {
        qDebug() << "Error finding least sold product:" << sorgu->lastError().text();
    }

    /////////////////////

    sorgu->prepare("SELECT u.Urun_ID, u.Urun_Adi, "
                   "SUM(ad.ToplamFiyat - (ad.Urun_Miktari * u.Urunun_Maliyeti)) AS TotalProfit "
                   "FROM Urunler u "
                   "JOIN AlisversiDetaylar ad ON u.Urun_ID = ad.Urun_ID "
                   "GROUP BY u.Urun_ID, u.Urun_Adi "
                   "ORDER BY TotalProfit ASC "
                   "LIMIT 1;");

    if (sorgu->exec() && sorgu->next()) {
        QString urunAdi = sorgu->value("Urun_Adi").toString();
        double totalProfit = sorgu->value("TotalProfit").toDouble();

        // qDebug() << "Least Profitable Product:" << urunAdi;
        // qDebug() << "Total Profit:" << totalProfit;

        ui->EnAzKaraliUrun->setText(tr("En az karlı ürün\n\n(") + urunAdi +
                                    tr("), kar: ") + QString::number(totalProfit) + " TL");
    } else {
        qDebug() << "Error finding least profitable product:" << sorgu->lastError().text();
    }

    //
    sorgu->prepare("SELECT u.Urun_ID, u.Urun_Adi, "
                   "SUM(ad.ToplamFiyat - (ad.Urun_Miktari * u.Urunun_Maliyeti)) AS TotalProfit "
                   "FROM Urunler u "
                   "JOIN AlisversiDetaylar ad ON u.Urun_ID = ad.Urun_ID "
                   "GROUP BY u.Urun_ID, u.Urun_Adi "
                   "ORDER BY TotalProfit DESC "
                   "LIMIT 1;");

    if (sorgu->exec() && sorgu->next()) {
        QString urunAdi = sorgu->value("Urun_Adi").toString();
        double totalProfit = sorgu->value("TotalProfit").toDouble();

        // qDebug() << "Most Profitable Product:" << urunAdi;
        // qDebug() << "Total Profit:" << totalProfit;

        ui->EnCokKaraliUrun->setText(tr("En karlı ürün\n\n(") + urunAdi +
                                     tr("), kar: ") + QString::number(totalProfit) + " TL");
    } else {
        qDebug() << "Error finding most profitable product:" << sorgu->lastError().text();
    }
}

void MainWindow::indirimRapolar()
{
    //
    sorgu->prepare("SELECT i.indirim_ID, i.indirim_adi, count(ad.indirim_ID) AS indirimsayisi "
                    "FROM indirimler i JOIN AlisversiDetaylar ad ON i.indirim_ID = ad.indirim_ID "
                    "GROUP BY i.indirim_ID, i.indirim_adi "
                    "ORDER BY indirimsayisi ASC LIMIT 1");

    if (sorgu->exec() && sorgu->next()){
        //d
        QString IndirimAdi = sorgu->value("indirim_adi").toString();
        int kere = sorgu->value("indirimsayisi").toInt();

        ui->EnAzIndirimler_label->setText(tr("En az kullanılan indirimler\n\n(") + IndirimAdi +
                                     "), " + QString::number(kere) + tr(" kere"));
    }
    else {
        qDebug() << "Error finding least sold product:" << sorgu->lastError().text();
    }

    //
    sorgu->prepare("SELECT i.indirim_ID, i.indirim_adi, count(ad.indirim_ID) AS indirimsayisi "
                   "FROM indirimler i JOIN AlisversiDetaylar ad ON i.indirim_ID = ad.indirim_ID "
                   "GROUP BY i.indirim_ID, i.indirim_adi "
                   "ORDER BY indirimsayisi DESC LIMIT 1");

    if (sorgu->exec() && sorgu->next()){
        //d
        QString IndirimAdi = sorgu->value("indirim_adi").toString();
        int kere = sorgu->value("indirimsayisi").toInt();

        ui->ensikIndirimler_label->setText(tr("En sık kullanılan indirimler\n\n(") + IndirimAdi +
                                          "), " + QString::number(kere) + tr(" kere"));
    }
    else {
        qDebug() << "Error finding least sold product:" << sorgu->lastError().text();
    }

    //
    sorgu->prepare("SELECT COUNT(*) FROM indirimler");

    if (sorgu->exec() && sorgu->next()){
        int indirim_sayisi = sorgu->value(0).toInt();

        ui->indirimSayisi_label->setText(tr("Toplam indirim sayısı\n\n") + QString::number(indirim_sayisi));
    }
    else {
        qDebug() << "Error finding least sold product:" << sorgu->lastError().text();
    }
}

void MainWindow::depoRaporlar()
{
    Depo_model = new QStandardItemModel(0, 4, this);
    Depo_model->setHorizontalHeaderLabels({tr("Urun ID"), tr("Urun Adi"), tr("Miktar Stokta"), tr("Durumu depoda")});

    sorgu->prepare("SELECT Urun_ID, Urun_Adi, Miktar_Stokta "
                   "FROM Urunler "
                   "WHERE Miktar_Stokta <= 10;");
    if (!sorgu->exec())
    {
        qDebug() << "Error depo rapor icin: " << sorgu->lastError();
        return;
    }

    int row_index = 0;
    int rowC = 0;
    while (sorgu->next())
    {
        Depo_model->insertRow(++rowC);
        QString Dizi[4];
        Dizi[0] = sorgu->value("Urun_Adi").toString(); // Urun Adi
        Dizi[1] = sorgu->value("Urun_ID").toString(); // Urun ID
        Dizi[2] = sorgu->value("Miktar_Stokta").toString(); // Miktari
        Dizi[3] = "Mevcut";
        for (int var = 0; var < 4; ++var)
        {
            QStandardItem *veri = new QStandardItem();
            veri->setText(Dizi[var]);
            veri->setTextAlignment(Qt::AlignCenter);
            if (var == 3)
            {
                //veri->setBackground(QColor(Qt::green));
                if(Dizi[var-1].toInt()<= 5){
                    Dizi[3] = "Bitecek";
                    veri->setText(Dizi[var]);
                    //veri->setBackground(QColor(Qt::yellow));
                }
                if(Dizi[var-1].toInt()<= 0){
                    Dizi[3] = "Bitti";
                    veri->setText(Dizi[var]);
                    //veri->setBackground(QColor(Qt::red));
                }
            }


            Depo_model->setItem(row_index, var, veri);
        }
        row_index++;
    }

    ui->depo->setModel(Depo_model);
}

void MainWindow::indirimleri()
{
    indirimler_model = new QStandardItemModel(0, 7, this);
    indirimler_model->setHorizontalHeaderLabels({tr("indirim ID"), tr("indirim Adi"), tr("indirim Yuzdesi"),
                tr("Başlangıç ​​tarihi"),tr("Bitiş tarihi"), tr("indirim durumu"),tr("İndirimin uygulandığı")});

    sorgu->prepare("SELECT indirimler.indirim_ID, indirimler.indirim_adi, "
                   "indirimler.indirim_yuzdesi, "
                   "indirimler.StartDate, "
                   "indirimler.EndDate, "
                   "indirimler.indirim_durumu, "
                   "urunler.urun_adi, "
                   "UrunKategori.kategori_adi "
                   "FROM indirimler "
                   "LEFT JOIN urunler ON indirimler.urun_id = urunler.urun_id "
                   "LEFT JOIN UrunKategori ON indirimler.kategori_id = UrunKategori.kategori_id "
                   "WHERE (indirimler.urun_id IS NOT NULL AND urunler.urun_id IS NOT NULL) "
                   "OR (indirimler.kategori_id IS NOT NULL AND UrunKategori.kategori_id IS NOT NULL) "
                   );
    // sorgu->prepare("select * from indirimler");
    sorgu->exec();

    int row_index = 0;
    int rowC = 0;
    while (sorgu->next())
    {
        indirimler_model->insertRow(++rowC);
        QString Dizi[7];
        Dizi[0] = sorgu->value(0).toString(); //indirim ID
        Dizi[1] = sorgu->value(1).toString(); //indirim Adi
        Dizi[2] = sorgu->value(2).toString(); //indirim Yuzdesi
        Dizi[3] = sorgu->value(3).toString(); //Başlangıç ​​tarihi
        Dizi[4] = sorgu->value(4).toString(); //Bitiş tarihi
        Dizi[5] = sorgu->value(5).toString(); //indirim durumu

        //İndirimin uygulandığı:
        if(sorgu->value(6).toString() == ""){
            Dizi[6] = sorgu->value(7).toString();
        }
        else{
            Dizi[6] = sorgu->value(6).toString();
        }


        for (int var = 0; var < 7; ++var)
        {
            QStandardItem *veri = new QStandardItem();
            veri->setText(Dizi[var]);
            veri->setTextAlignment(Qt::AlignCenter);
            if (var == 5)
            {
                if (Dizi[var] == "mevcut")
                {
                    veri->setBackground(QColor(Qt::green));
                }
                else if (Dizi[var] == "bitti")
                {
                    veri->setBackground(QColor(Qt::red));
                }
            }
            indirimler_model->setItem(row_index, var, veri);
        }
        row_index++;
    }

    ui->indirimler_tablo->setModel(indirimler_model);
    ui->indirimler_tablo->setColumnWidth(0, 80);
    ui->indirimler_tablo->setColumnWidth(1, 150);
    ui->indirimler_tablo->setColumnWidth(2, 110);
    ui->indirimler_tablo->setColumnWidth(6, 140);
    ui->indirimler_tablo->setColumnWidth(6, 140);
}

void MainWindow::Pie_Chart()
{

    QString sorguString = R"(
            SELECT
                UrunKategori.Kategori_Adi,
                SUM((AlisversiDetaylar.BirimFiyat - Urunler.Urunun_Maliyeti) * AlisversiDetaylar.Urun_Miktari) AS ToplamKar
            FROM
                Urunler
            JOIN
                AlisversiDetaylar ON Urunler.Urun_ID = AlisversiDetaylar.Urun_ID
            JOIN
                UrunKategori ON Urunler.Kategori_ID = UrunKategori.Kategori_ID
            GROUP BY
                UrunKategori.Kategori_Adi;
    )";

    sorgu->prepare(sorguString);

    if (!sorgu->exec()) {
        qDebug() << "Hata kar hesaplanırken: " << sorgu->lastError().text();
        return;
    }


    QPieSeries *series = new QPieSeries();
    while (sorgu->next()) {
        QString kategoriAdi = sorgu->value("Kategori_Adi").toString();
        double Kategori_ToplamKar = sorgu->value("ToplamKar").toDouble();
        double yuzde = (Kategori_ToplamKar/ToplamKar) * 100;
        //toplamoo += yuzde;//
        QString txt = kategoriAdi + " (" + QString::number(yuzde, 'f', 1)+"%)";
        series->append(txt, yuzde);
    }

    //pia chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->legend()->show();
    chart->setDropShadowEnabled(true);
    chart->setVisible(true);

    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setChart(chart);

}

void MainWindow::bar_chart()
{
    int max = 0;
    QStringList Subjectname;
    QLocale turkish(QLocale::Turkish, QLocale::Turkey);
    QDate currentDate = QDate::currentDate();
    for (int i = 1; i <= 6; ++i) {
        QDate previousMonth = currentDate.addMonths(-i);
        QString ay_ad = turkish.toString(previousMonth, "MMMM");
        Subjectname.append(ay_ad);


        sorgu->prepare(
            "SELECT k.Kategori_Adi, strftime('%m', ak.Tarih) AS Ay, "
            "SUM(ad.Urun_Miktari) AS ToplamUrunMiktari "
            "FROM AlisversiDetaylar ad "
            "JOIN Urunler u ON ad.Urun_ID = u.Urun_ID "
            "JOIN UrunKategori k ON u.Kategori_ID = k.Kategori_ID "
            "JOIN AlisversiKayit ak ON ad.Alisveris_ID = ak.Alisveris_ID "
            "WHERE ak.Tarih >= date('now', '-' || ? || ' months', 'start of month') "
            "AND ak.Tarih < date('now', '-' || ? || ' months', 'start of month') "
            "GROUP BY k.Kategori_Adi, strftime('%m', ak.Tarih) "
            "ORDER BY k.Kategori_Adi, Ay "
            );
        sorgu->addBindValue(i);//i
        sorgu->addBindValue(i-1);//i-1

        if(!sorgu->exec()){
            qDebug() << "Error bar chart:" << sorgu->lastError().text();
            return;
        }

        while (sorgu->next()) {
            QString KategoriAdi = sorgu->value("Kategori_Adi").toString();
            int TUrunMiktari = sorgu->value("ToplamUrunMiktari").toInt();
            if(max<=TUrunMiktari)
                max = TUrunMiktari;

            for (int var = 0; var < sets.size(); ++var) {
                if(KategoriAdi == sets[var]->label()){
                    sets[var]->append(TUrunMiktari);
                }
            }
        }

        for (int var = 0; var < sets.size(); ++var) {
            if(sets[var]->count() < i){
                sets[var]->append(0);
            }
        }
    }


    QBarSeries *series = new QBarSeries();
    for (int var = 0; var < sets.size(); ++var) {
        series->append(sets[var]);
    }

    ////////
    QChart *chart = new QChart();
    chart->addSeries(series);
    //chart->setTitle("");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(Subjectname);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, max+2);
    axisY->setLabelFormat("%d");//
    axisY->setTickCount((max+2) / 2 + 1);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setVisible(true);

    ui->graphicsView_2->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView_2->setChart(chart);
}

//sifreleme icin:
QString SifrelemeSHA256Hash(const QString &input)
{
    QByteArray inputData = input.toUtf8();
    QByteArray hash = QCryptographicHash::hash(inputData, QCryptographicHash::Sha256);
    return hash.toHex();
}
////////////////////

////Menu Fonkisyonlar:////
void MainWindow::on_anasayfa_clicked() { ui->stackedWidget->setCurrentIndex(0); }
void MainWindow::on_anasayfa_ic_clicked() { ui->stackedWidget->setCurrentIndex(0); }

void MainWindow::on_magaza_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    UrunlerGuncelleme();
}
void MainWindow::on_magaza_ic_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    UrunlerGuncelleme();
}

void MainWindow::on_rap_an_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    bugunTarihi();
}
void MainWindow::on_rap_an_ic_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    bugunTarihi();

}

void MainWindow::on_indirimler_clicked() { ui->stackedWidget->setCurrentIndex(3); }
void MainWindow::on_indirimler_ic_clicked() { ui->stackedWidget->setCurrentIndex(3); }

void MainWindow::on_KontrolPaneli_clicked() { ui->stackedWidget->setCurrentIndex(4); }
void MainWindow::on_KontrolPaneli_ic_clicked() { ui->stackedWidget->setCurrentIndex(4); }

void MainWindow::on_ayarlar_ic_clicked() {
    ui->stackedWidget->setCurrentIndex(5);
    ui->label_mesajDurumu->setStyleSheet("color: white");
    ui->label_settingDurumu->setStyleSheet("color: white");
}
void MainWindow::on_ayarlar_clicked() {
    ui->stackedWidget->setCurrentIndex(5);
    ui->label_mesajDurumu->setStyleSheet("color: white");
    ui->label_settingDurumu->setStyleSheet("color: white");
}

void MainWindow::on_kapat_button_clicked() { close(); } // ust menu

void MainWindow::on_noti_button_clicked()
{
    QIcon Notiicon(":/noti icons/icon/Notif icon/002-notification-1.png");
    ui->noti_button->setIcon(Notiicon);
    // QPoint buttonPos = ui->noti_button->mapToGlobal(QPoint(0, ui->noti_button->height()+4));
    // notificationPopup->move(buttonPos);
    // notificationPopup->show();

    int buttonCenterX = ui->noti_button->mapToGlobal(QPoint(0, 0)).x() + (ui->noti_button->width() / 2);

    QPoint popupPos(buttonCenterX - (notificationPopup->width() / 2),
                    ui->noti_button->mapToGlobal(QPoint(0, ui->noti_button->height() + 1)).y());

    notificationPopup->move(popupPos);
    notificationPopup->show();
}
////////////////////

////MenuBar Fonkisyonlar:////
void MainWindow::on_menubar_kapat_button_clicked() { close(); }

void MainWindow::on_minus_Button_clicked() { this->showMinimized(); }

void MainWindow::on_FullScreen_Button_clicked()
{
    QString stryle;
    if (FullScreen_tf == 0)
    {
        m_normalGeometry = geometry();
        this->showMaximized();

        stryle = R"(
                    QPushButton {
                        icon: url(:/menu icon/icon/menu icon/squares.png);
                    }
                )";
        ui->FullScreen_Button->setStyleSheet(stryle);

        FullScreen_tf = 1;
    }
    else
    {
        this->setGeometry(m_normalGeometry);
        FullScreen_tf = 0;

        stryle = R"(
                    QPushButton {
                        icon: url(:/menu icon/icon/menu icon/maximize.png);
                    }
                )";
        ui->FullScreen_Button->setStyleSheet(stryle);
    }
}
////////////////////

////Ana Sayfa Fonkisyonlar:////
void MainWindow::on_num_1_clicked()
{
    if (ui->lineEdit_urun_numara->hasFocus())
    {
        QString txt = ui->lineEdit_urun_numara->text() + "1";
        ui->lineEdit_urun_numara->setText(txt);
    }
    else if (ui->lineEdit_mikatr->hasFocus())
    {
        QString txt = ui->lineEdit_mikatr->text() + "1";
        ui->lineEdit_mikatr->setText(txt);
    }
    else
    {
    }
}

void MainWindow::on_num_2_clicked()
{
    if (ui->lineEdit_urun_numara->hasFocus())
    {
        QString txt = ui->lineEdit_urun_numara->text() + "2";
        ui->lineEdit_urun_numara->setText(txt);
    }
    else if (ui->lineEdit_mikatr->hasFocus())
    {
        QString txt = ui->lineEdit_mikatr->text() + "2";
        ui->lineEdit_mikatr->setText(txt);
    }
    else
    {
    }
}

void MainWindow::on_num_3_clicked()
{
    if (ui->lineEdit_urun_numara->hasFocus())
    {
        QString txt = ui->lineEdit_urun_numara->text() + "3";
        ui->lineEdit_urun_numara->setText(txt);
    }
    else if (ui->lineEdit_mikatr->hasFocus())
    {
        QString txt = ui->lineEdit_mikatr->text() + "3";
        ui->lineEdit_mikatr->setText(txt);
    }
    else
    {
    }
}

void MainWindow::on_num_4_clicked()
{
    if (ui->lineEdit_urun_numara->hasFocus())
    {
        QString txt = ui->lineEdit_urun_numara->text() + "4";
        ui->lineEdit_urun_numara->setText(txt);
    }
    else if (ui->lineEdit_mikatr->hasFocus())
    {
        QString txt = ui->lineEdit_mikatr->text() + "4";
        ui->lineEdit_mikatr->setText(txt);
    }
    else
    {
    }
}

void MainWindow::on_num_5_clicked()
{
    if (ui->lineEdit_urun_numara->hasFocus())
    {
        QString txt = ui->lineEdit_urun_numara->text() + "5";
        ui->lineEdit_urun_numara->setText(txt);
    }
    else if (ui->lineEdit_mikatr->hasFocus())
    {
        QString txt = ui->lineEdit_mikatr->text() + "5";
        ui->lineEdit_mikatr->setText(txt);
    }
    else
    {
    }
}

void MainWindow::on_num_6_clicked()
{
    if (ui->lineEdit_urun_numara->hasFocus())
    {
        QString txt = ui->lineEdit_urun_numara->text() + "6";
        ui->lineEdit_urun_numara->setText(txt);
    }
    else if (ui->lineEdit_mikatr->hasFocus())
    {
        QString txt = ui->lineEdit_mikatr->text() + "6";
        ui->lineEdit_mikatr->setText(txt);
    }
    else
    {
    }
}

void MainWindow::on_num_7_clicked()
{
    if (ui->lineEdit_urun_numara->hasFocus())
    {
        QString txt = ui->lineEdit_urun_numara->text() + "7";
        ui->lineEdit_urun_numara->setText(txt);
    }
    else if (ui->lineEdit_mikatr->hasFocus())
    {
        QString txt = ui->lineEdit_mikatr->text() + "7";
        ui->lineEdit_mikatr->setText(txt);
    }
    else
    {
    }
}

void MainWindow::on_num_8_clicked()
{
    if (ui->lineEdit_urun_numara->hasFocus())
    {
        QString txt = ui->lineEdit_urun_numara->text() + "8";
        ui->lineEdit_urun_numara->setText(txt);
    }
    else if (ui->lineEdit_mikatr->hasFocus())
    {
        QString txt = ui->lineEdit_mikatr->text() + "8";
        ui->lineEdit_mikatr->setText(txt);
    }
    else
    {
    }
}

void MainWindow::on_num_9_clicked()
{
    if (ui->lineEdit_urun_numara->hasFocus())
    {
        QString txt = ui->lineEdit_urun_numara->text() + "9";
        ui->lineEdit_urun_numara->setText(txt);
    }
    else if (ui->lineEdit_mikatr->hasFocus())
    {
        QString txt = ui->lineEdit_mikatr->text() + "9";
        ui->lineEdit_mikatr->setText(txt);
    }
    else
    {
    }
}

void MainWindow::on_num_0_clicked()
{
    if (ui->lineEdit_urun_numara->hasFocus())
    {
        QString txt = ui->lineEdit_urun_numara->text() + "0";
        ui->lineEdit_urun_numara->setText(txt);
    }
    else if (ui->lineEdit_mikatr->hasFocus())
    {
        QString txt = ui->lineEdit_mikatr->text() + "0";
        ui->lineEdit_mikatr->setText(txt);
    }
    else
    {
    }
}

void MainWindow::on_sil_button_clicked()
{
    ui->lineEdit_mikatr->clear();
    ui->lineEdit_urun_numara->clear();
}

// urunler_list tablosu:
void MainWindow::on_urunler_list_clicked(const QModelIndex &index) { urunler_list_tablo_index = index.row(); }

void MainWindow::on_Menuden_Sil_button_clicked()
{

    if (urunler_list_tablo_index >= 0 && urunler_list_tablo_index < model->rowCount())
    {
        QStandardItem *item = model->item(urunler_list_tablo_index, 2);
        double t_fiyat = item->text().remove(" TL").toDouble(); ////////

        Toplam -= t_fiyat;

        TT = tr("Toplam Tutar: ") + QString::number(Toplam, 'f', 2) + " TL";
        ui->total_label->setText(TT);

        if (Ekran->isVisible())
        {
            Ekran->Total_Yazdir(TT);
        }

        model->removeRow(urunler_list_tablo_index);
        urunler_list_tablo_index = -1;
    }
}

void MainWindow::on_iptal_button_clicked()
{
    model->clear();
    model->setHorizontalHeaderLabels({tr("Ürün"), tr("Miktar"), tr("Fiyat")});
    model->setRowCount(0);
    ui->urunler_list->setColumnWidth(1, 70);
    ui->urunler_list->setColumnWidth(2, 100);

    Toplam = 0;
    TT = tr("Toplam Tutar: ") + QString::number(Toplam, 'f', 2) + " TL";
    ui->total_label->setText(TT);

    if (Ekran->isVisible())
    {
        Ekran->Total_Yazdir(TT);
    }
}

void MainWindow::on_CiftEkran_button_clicked()
{
    Ekran->Total_Yazdir(TT);
    Ekran->show();
}

void MainWindow::on_gir_button_clicked()
{
    QString urun_no = ui->lineEdit_urun_numara->text();
    QString urun_miktar = ui->lineEdit_mikatr->text();

    if (urun_no.isEmpty() || urun_miktar.isEmpty())
    {
        return;
    }

    sorgu->prepare("select * from Urunler where Barkod = ? ");
    sorgu->addBindValue(urun_no);
    sorgu->exec();

    if (sorgu->next())
    {
        QString urun_id = sorgu->value(0).toString();
        QString urun_ad = sorgu->value(1).toString();
        double urun_fiyat = sorgu->value(3).toDouble();

        double ToplamFiyat;
        double indirimTutar = indirimCalc(urun_id,urun_fiyat);
        if(indirimTutar == 0){
            ToplamFiyat = urun_fiyat * urun_miktar.toDouble();
        }
        else{
            ToplamFiyat = (urun_fiyat - indirimTutar) * urun_miktar.toDouble();
        }

        UrunuTabloyaekle(urun_ad, urun_miktar, ToplamFiyat);
        Toplam += ToplamFiyat;

        TT = tr("Toplam Tutar: ") + QString::number(Toplam, 'f', 2) + " TL";

        ui->total_label->setText(TT);

        if (Ekran->isVisible())
        {
            Ekran->Total_Yazdir(TT);
        }
    }
    else
    {
        QMessageBox::information(this, "Hata MO", "Urun Bulanmadi..");
    }
}

void MainWindow::on_odeme_button_clicked()
{
    if (model->rowCount() == 0)
    {
        return;
    }

    QString format = "yyyy-MM-dd";
    QDate Bugun_Tarihi = QDate::currentDate();
    QString Tarih = Bugun_Tarihi.toString(format);

    double ToplamMiktar = Toplam;
    int ID = findNextAvailableId();
    // qDebug() << ID;
    if (ID == -1)
    {
        return;
    }

    //
    int miktarDurumu = 0;
    for (int row = 0; row < model->rowCount(); ++row)
    {
        QString Mustri_urunAd = model->item(row, 0)->text();         // Mustri Urun Ad
        int Mustri_urunMiktar = model->item(row, 1)->text().toInt(); // Mustri Urun Miktari

        sorgu->prepare("SELECT * FROM Urunler WHERE Urun_Adi = ?");
        sorgu->addBindValue(Mustri_urunAd);

        if (sorgu->exec() && sorgu->next())
        {
            int urunMiktarStokta = sorgu->value(5).toInt();
            if (urunMiktarStokta - Mustri_urunMiktar < 0)
            {
                QString mesaj = tr("Üründen yeterli miktarda yok (") + Mustri_urunAd + ")";
                notificationPopup->addNotification(mesaj);
                QIcon Notiicon(":/noti icons/icon/Notif icon/002-active.png");
                ui->noti_button->setIcon(Notiicon);
                miktarDurumu = 1;
            }
        }
        else
        {
            QMessageBox::information(this, "Hata", "Ürün bulunamadı: " + Mustri_urunAd);
            continue;
        }
    }

    if (miktarDurumu == 1)
    {
        return;
    }
    //

    sorgu->prepare("insert into AlisversiKayit (Alisveris_ID, ToplamMiktar, Tarih) values (?,?,?)");
    sorgu->addBindValue(ID);
    sorgu->addBindValue(ToplamMiktar);
    sorgu->addBindValue(Tarih);

    if (!sorgu->exec())
    {
        QMessageBox::information(this, "Hataa", sorgu->lastError().text());
        return;
    }

    //
    for (int row = 0; row < model->rowCount(); ++row)
    {
        QString urunAdi = model->item(row, 0)->text();
        int miktar = model->item(row, 1)->text().toInt();

        double birimFiyat;
        int urunID = -1;
        int MikatStokta = 0;
        int urunKategoriID = -1;
        sorgu->prepare("SELECT * FROM Urunler WHERE Urun_Adi = ?");
        sorgu->addBindValue(urunAdi);

        if (sorgu->exec() && sorgu->next())
        {
            urunID = sorgu->value(0).toInt();
            birimFiyat = sorgu->value(3).toDouble();

            MikatStokta = sorgu->value(5).toInt();
            urunKategoriID = sorgu->value(6).toInt();
        }
        else
        {
            QMessageBox::information(this, "Hata", "Ürün bulunamadı: " + urunAdi);
            continue;
        }

        double toplamFiyat;
        int indirimID = 0;
        double indirim_tutar = indirimCalc(QString::number(urunID) ,birimFiyat);
        if(indirim_tutar ==0){
            toplamFiyat = miktar * birimFiyat;
        }
        else{
            toplamFiyat = (birimFiyat - indirim_tutar) * miktar;
            //
            indirimSorgu = new QSqlQuery(veriTabanin);
            indirimSorgu->prepare("select * from indirimler where (urun_id = ? or kategori_id = ?)  "
                                  "AND (indirim_durumu = 'bitecek' OR indirim_durumu = 'mevcut')");
            indirimSorgu->addBindValue(urunID);
            indirimSorgu->addBindValue(urunKategoriID);
            indirimSorgu->exec();

            if(indirimSorgu->next()){
                indirimID = indirimSorgu->value(0).toInt();
            }
            //
        }


        sorgu->prepare("INSERT INTO AlisversiDetaylar (Alisveris_ID, Urun_ID, Urun_Miktari, BirimFiyat,"
                       " ToplamFiyat, indirim_tutari, indirim_ID) "
                       "VALUES (?, ?, ?, ?, ?, ?, ?)");
        sorgu->addBindValue(ID);
        sorgu->addBindValue(urunID);
        sorgu->addBindValue(miktar);
        sorgu->addBindValue(birimFiyat);
        sorgu->addBindValue(toplamFiyat);
        sorgu->addBindValue(indirim_tutar);
        sorgu->addBindValue(indirimID);

        if (!sorgu->exec())
        {
            QMessageBox::information(this, "Hata", "Detay eklenemedi: " + sorgu->lastError().text());
            return;
        }

        sorgu->prepare("UPDATE Urunler SET Miktar_Stokta = Miktar_Stokta - ? WHERE Urun_ID = ?");
        sorgu->addBindValue(miktar);
        sorgu->addBindValue(urunID);

        if (!sorgu->exec())
        {
            QMessageBox::information(this, "Hata", "Miktar Gunecellmedi: " + sorgu->lastError().text());
            return;
        }

        if (MikatStokta - miktar <= 10 && MikatStokta - miktar >= 1)
        {
            QString mesaj = tr("Ürün (") + urunAdi + tr(") yakında stoklarımızda kalmayacak");
            notificationPopup->addNotification(mesaj);
            QIcon Notiicon(":/noti icons/icon/Notif icon/002-active.png");
            ui->noti_button->setIcon(Notiicon);
        }
        else if (MikatStokta - miktar <= 0)
        {
            QString mesaj = tr("Ürün (") + urunAdi + tr(") stoklarımızda kalmamıştır");
            notificationPopup->addNotification(mesaj);
            QIcon Notiicon(":/noti icons/icon/Notif icon/002-active.png");
            ui->noti_button->setIcon(Notiicon);
        }
    }

    // if odeme is done
    analizler(0);
    istatistikler();
    raporlar();
    indirimRapolar();
    depoRaporlar();
    // indirimleri();
    Pie_Chart();

    model->clear();
    model->setHorizontalHeaderLabels({tr("Ürün"), tr("Miktar"), tr("Fiyat")});
    model->setRowCount(0);
    ui->urunler_list->setColumnWidth(1, 70);
    ui->urunler_list->setColumnWidth(2, 100);

    Toplam = 0;
    TT = tr("Toplam Tutar: ") + QString::number(Toplam, 'f', 2) + " TL";
    ui->total_label->setText(TT);

    if (Ekran->isVisible())
    {
        Ekran->Total_Yazdir(TT);
    }
}

////////////////////

////Magaza Fonkisyonlar:////
void MainWindow::on_kategori_urunler_activated(int index)
{
    Q_UNUSED(index);//
    ui->lineEdit_urun_arayin->clear();
    UrunFiltreModel->setSourceModel(Urunler_model);
    UrunFiltreModel->setFilterKeyColumn(6);

    if (ui->kategori_urunler->currentText() == "Kategori seçiniz")
    {
        ui->urunler_table->setModel(Urunler_model);
        return;
    }

    UrunFiltreModel->setFilterFixedString(ui->kategori_urunler->currentText());
    ui->urunler_table->setModel(UrunFiltreModel);
}

void MainWindow::on_lineEdit_urun_arayin_textChanged(const QString &arg1)
{
    if (ui->kategori_urunler->currentText() != "Kategori seçiniz")
    {
        FiltreModelSeachBar->setSourceModel(UrunFiltreModel);
        FiltreModelSeachBar->setFilterKeyColumn(1);

        FiltreModelSeachBar->setFilterFixedString(arg1);
        ui->urunler_table->setModel(FiltreModelSeachBar);
    }
    else
    {
        FiltreModelSeachBar->setSourceModel(Urunler_model);
        FiltreModelSeachBar->setFilterKeyColumn(1);

        FiltreModelSeachBar->setFilterFixedString(arg1);
        ui->urunler_table->setModel(FiltreModelSeachBar);
    }
}

void MainWindow::on_urunler_table_clicked(const QModelIndex &index) { urunler_table_index = index.row(); }

void MainWindow::on_urun_ekle_clicked()
{
    connect(YeniUrun_Ekrani, &yeniUrun::UrunuEkledi, this, &MainWindow::UrunlerGuncelleme);
    YeniUrun_Ekrani->exec();
}

void MainWindow::on_uruun_sil_clicked()
{
    int indexo = urunler_table_index;

    QModelIndex proxyIndex;
    if (ui->urunler_table->model() == UrunFiltreModel)
    {
        proxyIndex = UrunFiltreModel->index(urunler_table_index, 4);
        indexo = UrunFiltreModel->mapToSource(proxyIndex).row();
    }

    if (ui->urunler_table->model() == FiltreModelSeachBar)
    {
        proxyIndex = FiltreModelSeachBar->index(urunler_table_index, 4);
        if (ui->kategori_urunler->currentText() != "Kategori seçiniz")
        {
            QModelIndex index2 = FiltreModelSeachBar->mapToSource(proxyIndex);
            indexo = UrunFiltreModel->mapToSource(index2).row();
        }
        else
        {
            indexo = FiltreModelSeachBar->mapToSource(proxyIndex).row();
        }
    }

    if (urunler_table_index >= 0 && urunler_table_index < Urunler_model->rowCount())
    {
        QStandardItem *item = Urunler_model->item(indexo, 0);
        QString urunID = item->text();

        sorgu->prepare("delete from Urunler where Urun_ID = ?");
        sorgu->addBindValue(urunID);

        if (!sorgu->exec())
        {
            QMessageBox::information(this, "Hata", "urun silemedi: " + sorgu->lastError().text());
            urunler_table_index = -1;
            return;
        }

        UrunlerGuncelleme();
        urunler_table_index = -1;
    }
}

void MainWindow::on_urun_duzenle_clicked()
{
    connect(UrunEdit_Ekrani, &yeniUrun::UrunuEkledi, this, &MainWindow::UrunlerGuncelleme);

    int indexo = urunler_table_index;
    QModelIndex proxyIndex;
    if (ui->urunler_table->model() == UrunFiltreModel)
    {
        proxyIndex = UrunFiltreModel->index(urunler_table_index, 4);
        indexo = UrunFiltreModel->mapToSource(proxyIndex).row();
    }

    if (ui->urunler_table->model() == FiltreModelSeachBar)
    {
        proxyIndex = FiltreModelSeachBar->index(urunler_table_index, 4);
        if (ui->kategori_urunler->currentText() != "Kategori seçiniz")
        {
            QModelIndex index2 = FiltreModelSeachBar->mapToSource(proxyIndex);
            indexo = UrunFiltreModel->mapToSource(index2).row();
        }
        else
        {
            indexo = FiltreModelSeachBar->mapToSource(proxyIndex).row();
        }
    }

    if (urunler_table_index >= 0 && urunler_table_index < Urunler_model->rowCount())
    {
        UrunEdit_Ekrani->UID = Urunler_model->item(indexo, 0)->text();
        UrunEdit_Ekrani->UAd = Urunler_model->item(indexo, 1)->text();
        UrunEdit_Ekrani->UBarkod = Urunler_model->item(indexo, 2)->text();
        UrunEdit_Ekrani->UFiyat = Urunler_model->item(indexo, 3)->text();
        UrunEdit_Ekrani->UMaliyet = Urunler_model->item(indexo, 4)->text();
        UrunEdit_Ekrani->UStok = Urunler_model->item(indexo, 5)->text();

        QString KategorAd = Urunler_model->item(indexo, 6)->text();
        int KategorID = 0;
        for (int in = 0; in < Kategoriler.length(); ++in)
        {
            if (KategorAd == Kategoriler[in].name)
            {
                KategorID = Kategoriler[in].id.toInt();
            }
        }
        UrunEdit_Ekrani->UKategori = KategorID;
        UrunEdit_Ekrani->urunBilgiGuncelleme();
        UrunEdit_Ekrani->exec();
        urunler_table_index = -1;
    }
    else
    {
        UrunEdit_Ekrani->exec();
    }
}

////////////////////

////Raporlar ve analizler Fonkisyonlar:////
void MainWindow::on_satisR_button_clicked() { ui->stackedWidget_2->setCurrentIndex(0); }

void MainWindow::on_indirimR_button_clicked() { ui->stackedWidget_2->setCurrentIndex(1); }

void MainWindow::on_EnvanterR_button_clicked() { ui->stackedWidget_2->setCurrentIndex(2); }

void MainWindow::on_radioButton_7gun_clicked()
{
    analizler(7);
}

void MainWindow::on_radioButton_30gun_clicked()
{
    analizler(30);
}

void MainWindow::on_radioButton_tum_clicked()
{
    analizler(0);
}

void MainWindow::on_Depo_durumu_activated(int index)
{
    Q_UNUSED(index);//
    DepoFiltreModel->setSourceModel(Depo_model);
    DepoFiltreModel->setFilterKeyColumn(3);
    //qDebug()<<ui->Depo_durumu->currentText();
    if (ui->Depo_durumu->currentText() == "Depo durumu")
    {
        ui->depo->setModel(Depo_model);
        return;
    }

    DepoFiltreModel->setFilterFixedString(ui->Depo_durumu->currentText());
    ui->depo->setModel(DepoFiltreModel);
}

void MainWindow::on_sutungrafik_guncelleme_clicked()
{
    bar_chart();
}

////////////////////

////indirimler Fonkisyonlar:////
void MainWindow::on_indirimi_ekle_clicked()
{
    connect(YeniIndirim_Ekrani, &indirimler::IndirimEkledi, this, &MainWindow::IndirimlerGuncelleme);
    YeniIndirim_Ekrani->exec();
}

void MainWindow::on_indirimi_sil_clicked()
{
    int indexo = indirim_tablo_index;
    QModelIndex proxyIndex;
    if (ui->indirimler_tablo->model() == indirimlerFiltreModel)
    {
        proxyIndex = indirimlerFiltreModel->index(indirim_tablo_index, 4);
        indexo = indirimlerFiltreModel->mapToSource(proxyIndex).row();
    }

    if (indirim_tablo_index >= 0 && indirim_tablo_index < indirimler_model->rowCount())
    {
        QStandardItem *item = indirimler_model->item(indexo, 0);
        QString indirimID = item->text();

        sorgu->prepare("delete from indirimler where indirim_ID = ?");
        sorgu->addBindValue(indirimID);

        if (!sorgu->exec())
        {
            QMessageBox::information(this, "Hata", "indirim silemedi: " + sorgu->lastError().text());
            indirim_tablo_index = -1;
            return;
        }

        IndirimlerGuncelleme();
        indirim_tablo_index = -1;
    }
}

void MainWindow::on_indirimi_duzenleyin_clicked()
{
    connect(IndirimEdit_Ekrani, &indirimler::IndirimEkledi, this, &MainWindow::IndirimlerGuncelleme);

    int indexo = indirim_tablo_index;
    QModelIndex proxyIndex;
    if (ui->indirimler_tablo->model() == indirimlerFiltreModel)
    {
        proxyIndex = indirimlerFiltreModel->index(indirim_tablo_index, 4);
        indexo = indirimlerFiltreModel->mapToSource(proxyIndex).row();
    }

    if (indirim_tablo_index >= 0 && indirim_tablo_index < indirimler_model->rowCount())
    {

        IndirimEdit_Ekrani->indirim_id = indirimler_model->item(indexo, 0)->text();
        IndirimEdit_Ekrani->indirim_ad = indirimler_model->item(indexo, 1)->text();
        IndirimEdit_Ekrani->indirim_yuzdesi = indirimler_model->item(indexo, 2)->text();
        IndirimEdit_Ekrani->Strt_date = indirimler_model->item(indexo, 3)->text();
        IndirimEdit_Ekrani->End_date = indirimler_model->item(indexo, 4)->text();

        QString sorguString = R"(
                        SELECT
                            CASE
                                WHEN urun_id IS NOT NULL THEN 'urun'
                                WHEN kategori_id IS NOT NULL THEN 'kategori'
                                ELSE 'Both columns are NULL or an error occurred'
                            END AS result,
                            COALESCE(urun_id, kategori_id) AS value
                        FROM indirimler
                        WHERE indirim_ID = ?;
                                )";
        sorgu->prepare(sorguString);
        sorgu->addBindValue(indirimler_model->item(indexo, 0)->text());
        int indirim_uygu;
        // urun = 1
        // kategori = 2
        int val;
        if (sorgu->exec() && sorgu->next())
        {
            if(sorgu->value("result").toString() == "urun"){
                indirim_uygu = 1;
            }else{
                indirim_uygu = 2;
            }
            val = sorgu->value("value").toInt();

        }
        else
        {
            QMessageBox::information(this, "Hata", "sorguString in indimir guncelleme"
                                                       + indirimler_model->item(indexo,6)->text());
            return;
        }

        if(indirim_uygu == 1){//urun
            IndirimEdit_Ekrani->indirim_tipi = 0;

            sorgu->prepare("SELECT * FROM Urunler WHERE Urun_ID = ?");
            sorgu->addBindValue(val);//indirimler_model->item(indexo, 6)->text()
            QString urun_barkod;
            if (sorgu->exec() && sorgu->next())
            {
                urun_barkod = sorgu->value(2).toString();
            }
            else
            {
                QMessageBox::information(this, "Hata", "Ürün bulunamadı: " + indirimler_model->item(indexo,6)->text());
                return;
            }
            IndirimEdit_Ekrani->urun_numara = urun_barkod;
        }
        else if(indirim_uygu == 2){//kategori
            IndirimEdit_Ekrani->indirim_tipi = 1;
            IndirimEdit_Ekrani->IKategori = val;
        }

        IndirimEdit_Ekrani->indirimBilgiGuncelleme();
        IndirimEdit_Ekrani->exec();
        indirim_tablo_index = -1;
    }
    else
    {
        IndirimEdit_Ekrani->exec();
    }
}

void MainWindow::on_indirim_durumu_activated(int index)
{
    Q_UNUSED(index);//
    indirimlerFiltreModel->setSourceModel(indirimler_model);
    indirimlerFiltreModel->setFilterKeyColumn(5);

    if (ui->indirim_durumu->currentText() == "İndirim durumunu")
    {
        ui->indirimler_tablo->setModel(indirimler_model);
        return;
    }
    else if(ui->indirim_durumu->currentText() == "Aktif indirimler"){

        QRegularExpression regex("mevcut|bitecek", QRegularExpression::CaseInsensitiveOption);
        indirimlerFiltreModel->setFilterRegularExpression(regex);

        //indirimlerFiltreModel->setFilterFixedString("mevcut");
        ui->indirimler_tablo->setModel(indirimlerFiltreModel);
        return;
    }
    else if(ui->indirim_durumu->currentText() == "İndirimler sona erecek"){
        indirimlerFiltreModel->setFilterFixedString("bitecek");
        ui->indirimler_tablo->setModel(indirimlerFiltreModel);
        return;
    }
    else if(ui->indirim_durumu->currentText() == "İndirimler başlamadı"){
        indirimlerFiltreModel->setFilterFixedString("baslamadi");
        ui->indirimler_tablo->setModel(indirimlerFiltreModel);
        return;
    }
    else if(ui->indirim_durumu->currentText() == "İndirimler sona erdi"){
        indirimlerFiltreModel->setFilterFixedString("bitti");
        ui->indirimler_tablo->setModel(indirimlerFiltreModel);
        return;
    }

}

void MainWindow::on_indirimler_tablo_clicked(const QModelIndex &index) {indirim_tablo_index = index.row();}

////////////////////

////Kontrol Paneli Fonkisyonlar:////
void MainWindow::on_hesap_durumu_activated(int index)
{
    Q_UNUSED(index);//
    proxyModel->setSourceModel(Hesaplar_model);
    proxyModel->setFilterKeyColumn(4);

    if (ui->hesap_durumu->currentText() == "Aktif Hesaplar")
    {
        proxyModel->setFilterFixedString("aktif");
        ui->tableView_hesaplar->setModel(proxyModel);
    }
    else if (ui->hesap_durumu->currentText() == "Pasif Hesaplar")
    {
        proxyModel->setFilterFixedString("pasif");
        ui->tableView_hesaplar->setModel(proxyModel);
    }
    else
    {
        ui->tableView_hesaplar->setModel(Hesaplar_model);
    }
}

void MainWindow::on_tableView_hesaplar_clicked(const QModelIndex &index)
{
    hesaplar_tablo_index = index.row();
}

void MainWindow::on_pushButton_aktif_clicked()
{
    int indexo = hesaplar_tablo_index;

    if (ui->tableView_hesaplar->model() == proxyModel)
    {
        QModelIndex proxyIndex = proxyModel->index(hesaplar_tablo_index, 4);
        indexo = proxyModel->mapToSource(proxyIndex).row();
    }

    if (hesaplar_tablo_index >= 0 && hesaplar_tablo_index < Hesaplar_model->rowCount())
    {
        QStandardItem *elemetn = Hesaplar_model->item(indexo, 4);
        QString durumu = elemetn->text();
        qDebug() << durumu; //
        if (durumu == "aktif")
        {
            hesaplar_tablo_index = -1;
            return;
        }

        QStandardItem *elemetn2 = Hesaplar_model->item(indexo, 0);
        QString Kullaniciad = elemetn2->text();
        qDebug() << Kullaniciad; //

        sorgu->prepare("UPDATE Kullanicilar SET durum = 'aktif' WHERE Kullanici_Adi = ?");
        sorgu->addBindValue(Kullaniciad);

        if (!sorgu->exec())
        {
            QMessageBox::information(this, "Hata", "durum Gunecellmedi: " + sorgu->lastError().text());
            hesaplar_tablo_index = -1;
            return;
        }

        elemetn->setText("aktif");
        elemetn->setBackground(QColor(Qt::green));

        hesaplar_tablo_index = -1;
    }
}

void MainWindow::on_pushButton_pasif_clicked()
{
    int indexo = hesaplar_tablo_index;

    if (ui->tableView_hesaplar->model() == proxyModel)
    {
        QModelIndex proxyIndex = proxyModel->index(hesaplar_tablo_index, 4);
        indexo = proxyModel->mapToSource(proxyIndex).row();
    }

    if (hesaplar_tablo_index >= 0 && hesaplar_tablo_index < Hesaplar_model->rowCount())
    {
        QStandardItem *elemetn = Hesaplar_model->item(indexo, 4);
        QString durumu = elemetn->text();
        qDebug() << durumu; //
        if (durumu == "pasif")
        {
            hesaplar_tablo_index = -1;
            return;
        }

        QStandardItem *elemetn2 = Hesaplar_model->item(indexo, 0);
        QString Kullaniciad = elemetn2->text();
        qDebug() << Kullaniciad; //

        sorgu->prepare("UPDATE Kullanicilar SET durum = 'pasif' WHERE Kullanici_Adi = ?");
        sorgu->addBindValue(Kullaniciad);

        if (!sorgu->exec())
        {
            QMessageBox::information(this, "Hata", "durum P Gunecellmedi: " + sorgu->lastError().text());
            hesaplar_tablo_index = -1;
            return;
        }

        elemetn->setText("pasif");
        elemetn->setBackground(QColor(Qt::red));

        hesaplar_tablo_index = -1;
    }
}

////////////////////

////Ayarlar Fonkisyonlar:////
void MainWindow::on_dil_comboBox_activated(int index)
{
    Q_UNUSED(index);//

    QString Dosya = "setting.txt";
    QFile file(Dosya);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QString Message = "Dosya (setting.txt) yazmak için açılamıyor.\nLütfen dosyayı kontrol edip tekrar deneyin.";
        QMessageBox::warning(this, "Bilgi", Message);
        return;
    }
    QTextStream veriler(&file);

    QTranslator trans;
    if(ui->dil_comboBox->currentText() == "ingilizce"){



        veriler << "English";
    }

    if(ui->dil_comboBox->currentText() == "Türkçe"){
        veriler << "Turkish";

    }

    ui->label_settingDurumu->setText(tr("Dili değiştirmek için uygulamayı yeniden başlatmalısınız"));
    ui->label_settingDurumu->setStyleSheet("color: green");
    file.close();

}

////////////////////


void MainWindow::on_update_clicked()
{
    QString username = ui->lineEdit_username->text();
    QString AdSoyad = ui->lineEdit_AdSoyad->text();
    QString telefon = ui->lineEdit_num->text();
    QString email = ui->lineEdit_Email->text();

    if(username.isEmpty() || AdSoyad.isEmpty()|| telefon.isEmpty()|| email.isEmpty() ){
        ui->label_mesajDurumu->setText(tr("Lütfen tüm alanları doldurun"));
        ui->label_mesajDurumu->setStyleSheet("color: red");
        return;
    }


    sorgu->prepare("UPDATE Kullanicilar SET Telefon = ? , Ad_Soyad = ?, Email = ? "
                         "WHERE Kullanici_Adi = ? ");
    sorgu->addBindValue(telefon);
    sorgu->addBindValue(AdSoyad);
    sorgu->addBindValue(email);
    sorgu->addBindValue(username);
    if (!sorgu->exec())
    {
        qDebug() << "Error update in on_update_clicked(): " << sorgu->lastError();
        return;
    }

    ui->label_mesajDurumu->setText(tr("Veriler başarıyla güncellendi"));
    ui->label_mesajDurumu->setStyleSheet("color: green");
}


void MainWindow::on_changePass_clicked()
{
    QString username = ui->lineEdit_username->text();
    QString yeniSifre = ui->lineEdit_newsifre->text();
    QString eskiSifre = ui->lineEdit_eskisifre->text();

    QString OldstoredHash = SifrelemeSHA256Hash(eskiSifre);
    QString NewstoredHash = SifrelemeSHA256Hash(yeniSifre);

    sorgu->prepare("select * from Kullanicilar where Kullanici_Adi = ? and Sifre = ?");
    sorgu->addBindValue(username);
    sorgu->addBindValue(OldstoredHash);
    sorgu->exec();

    if (sorgu->next())
    {
        sorgu->prepare("UPDATE Kullanicilar SET Sifre = ? WHERE Kullanici_Adi = ?  ");
        sorgu->addBindValue(NewstoredHash);
        sorgu->addBindValue(username);
        if (!sorgu->exec())
        {
            qDebug() << "Error update in on_update_clicked(): " << sorgu->lastError();
            return;
        }

        ui->label_mesajDurumu->setText(tr("Şifre başarıyla güncellendi"));
        ui->label_mesajDurumu->setStyleSheet("color: green");
    }
    else{
        ui->label_mesajDurumu->setText(tr("Mevcut şifre yanlış"));
        ui->label_mesajDurumu->setStyleSheet("color: red");
    }


}

