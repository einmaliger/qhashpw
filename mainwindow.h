#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QList>
#include <QtGui/QCheckBox>
#include <QtGui/QMainWindow>
#include <QtGui/QTableWidget>

#include "account.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QList<Account> &a, const QString &accessCode, QWidget *parent = 0);
    ~MainWindow();

    void updateTable();

private slots:
    void filter(const QString &phrase = "");
    void lockToggled(int state);

private:
    QTableWidget *tab;
    QList<Account> all;
    QList<Account*> filtered;
    QCheckBox *lock;

    QString accessCode;
};

#endif // MAINWINDOW_H
