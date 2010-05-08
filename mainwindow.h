#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QList>
#include <QtGui/QCheckBox>
#include <QtGui/QMainWindow>
#include <QtGui/QTableWidget>

#include "accountset.h"

class QLineEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(AccountSet *accounts, QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateTable();

private slots:
    void cellClicked(int row, int column);
    void cellEntered(int row, int column);
    void filter();
    QString getPassword(const Account &a) const;
    void hideVisiblePW();
    void lockToggled(int state);

private:
    QTableWidget *tab;
    QCheckBox *lock;
    QLineEdit *searchPhrase;

    AccountSet *accounts_;

    QString mainPW;
    int currentlyVisiblePW;     // row of password that is currently visible (or -1)
};

#endif // MAINWINDOW_H
