#ifndef NOTIFICATIONPOPUP_H
#define NOTIFICATIONPOPUP_H

#include "qlistwidget.h"
#include <QWidget>

namespace Ui
{
    class NotificationPopup;
}

class NotificationPopup : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationPopup(QWidget *parent = nullptr);
    ~NotificationPopup();

    void addNotification(const QString &text);

private slots:
    void on_bildirimler_list_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::NotificationPopup *ui;
};

#endif // NOTIFICATIONPOPUP_H
