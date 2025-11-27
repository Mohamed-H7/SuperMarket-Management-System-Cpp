#include "notificationpopup.h"
#include "ui_notificationpopup.h"

NotificationPopup::NotificationPopup(QWidget *parent)
    : QWidget(parent, Qt::Popup), ui(new Ui::NotificationPopup)
{
    ui->setupUi(this);
}

NotificationPopup::~NotificationPopup()
{
    delete ui;
}

void NotificationPopup::addNotification(const QString &text)
{
    ui->bildirimler_list->addItem(text);
}

void NotificationPopup::on_bildirimler_list_itemDoubleClicked(QListWidgetItem *item)
{
    delete item;
}
