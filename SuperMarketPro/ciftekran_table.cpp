#include "ciftekran_table.h"
#include "ui_ciftekran_table.h"
#include <QTableWidget>

CiftEkran_Table::CiftEkran_Table(QStandardItemModel *tablo_model, QWidget *parent)
    : QDialog(parent), ui(new Ui::CiftEkran_Table)
{
    ui->setupUi(this);

    ui->tableView->setModel(tablo_model);
    ui->tableView->setColumnWidth(1, 70);
    ui->tableView->setColumnWidth(2, 100);
}

CiftEkran_Table::~CiftEkran_Table()
{
    delete ui;
}

void CiftEkran_Table::Total_Yazdir(QString a)
{
    ui->tableView->setColumnWidth(1, 70);
    ui->tableView->setColumnWidth(2, 100);
    ui->total_label2->setText(a);
}
