#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QCryptographicHash>

namespace Ui
{
    class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QSqlDatabase, QWidget *parent = nullptr);
    ~login();

private slots:
    void on_kapat_button_clicked();

    void on_pushButton_clicked();

    void on_kaydol_toolButton_clicked();

    void on_giris_toolButton_clicked();

    void on_kaydon_pushButton_clicked();

private:
    Ui::login *ui;

    QSqlQuery *sorgu_LO;
};

#endif // LOGIN_H
