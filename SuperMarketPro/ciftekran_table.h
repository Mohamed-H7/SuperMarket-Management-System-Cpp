#ifndef CIFTEKRAN_TABLE_H
#define CIFTEKRAN_TABLE_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui
{
    class CiftEkran_Table;
}

class CiftEkran_Table : public QDialog
{
    Q_OBJECT

public:
    explicit CiftEkran_Table(QStandardItemModel *tablo_model, QWidget *parent = nullptr);
    ~CiftEkran_Table();

    void Total_Yazdir(QString a);

private:
    Ui::CiftEkran_Table *ui;
};

#endif // CIFTEKRAN_TABLE_H
