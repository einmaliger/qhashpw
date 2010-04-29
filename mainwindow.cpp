#include <QtCore/QStringList>
#include <QtGui/QTableWidgetItem>

#include "mainwindow.h"

MainWindow::MainWindow(QList<Account> &a, QWidget *parent)
    : QMainWindow(parent)
{
    tab = new QTableWidget(a.size(), 4);

    QStringList headers;
    headers << tr("Site") << tr("User") << tr("Password") << tr("Note");
    tab->setHorizontalHeaderLabels(headers);

    for(int i = 0; i < a.size(); ++i)
    {
        QTableWidgetItem *it;

        it = new QTableWidgetItem(a[i].site());
        tab->setItem(i, 0, it);

        it = new QTableWidgetItem(a[i].user());
        tab->setItem(i, 1, it);

        QString s;
        for(int j = 0; j < a[i].max(); ++j)
            s += "*";
        it = new QTableWidgetItem(s);
        tab->setItem(i, 2, it);

        it = new QTableWidgetItem(a[i].note());
        tab->setItem(i, 3, it);
    }
    setCentralWidget(tab);
}

MainWindow::~MainWindow()
{
}
