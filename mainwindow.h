#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QList>
#include <QtGui/QMainWindow>
#include <QtGui/QTableWidget>

#include "account.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QList<Account> &a, QWidget *parent = 0);
    ~MainWindow();

    void addInfo(const QString &info);

private:
    QTableWidget *tab;
};

#endif // MAINWINDOW_H
