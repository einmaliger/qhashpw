#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QList>
#include <QtGui/QCheckBox>
#include <QtGui/QMainWindow>
#include <QtGui/QTableWidget>

#include "account.h"

class QLineEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QList<Account> &a, const QString &accessCode, QWidget *parent = 0);
    ~MainWindow();

    void updateTable();

private slots:
    void cellClicked(int row, int column);
    void cellEntered(int row, int column);
    void filter();
    QString getPassword(const Account *a) const;
    void hideVisiblePW();
    void lockToggled(int state);

private:
    QTableWidget *tab;
    QList<Account> all;
    QList<Account*> filtered;
    QCheckBox *lock;
    QLineEdit *searchPhrase;

    QString accessCode;
    QString mainPW;
    int currentlyVisiblePW;     // row of password that is currently visible (or -1)
};

#endif // MAINWINDOW_H
