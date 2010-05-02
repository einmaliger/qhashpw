#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QStatusBar>
#include <QtGui/QTableWidgetItem>

#include "hashpw.h"
#include "mainwindow.h"

MainWindow::MainWindow(QList<Account> &a, const QString &accessCode, QWidget *parent)
    : QMainWindow(parent), accessCode(accessCode)
{
    tab = new QTableWidget(a.size(), 4);

    QStringList headers;
    headers << tr("Site") << tr("User") << tr("Password") << tr("Note");
    tab->setHorizontalHeaderLabels(headers);

    connect(tab, SIGNAL(cellEntered(int,int)), SLOT(cellEntered(int,int)));

    all = a;
    filter();

    setCentralWidget(tab);

    lock = new QCheckBox("Locked");
    lock->setChecked(true);
    connect(lock, SIGNAL(stateChanged(int)),SLOT(lockToggled(int)));

    statusBar()->addPermanentWidget(lock);
}

MainWindow::~MainWindow()
{
}

void MainWindow::updateTable()
{
    currentlyVisiblePW = -1;
    tab->clearContents();
    tab->setRowCount(filtered.size());

    int i = 0;

    foreach(Account *a, filtered)
    {
        QTableWidgetItem *it;

        it = new QTableWidgetItem(a->site());
        tab->setItem(i, 0, it);

        it = new QTableWidgetItem(a->user());
        tab->setItem(i, 1, it);

        QString s;
        for(int j = 0; j < a->max(); ++j)
            s += "*";
        it = new QTableWidgetItem(s);
        tab->setItem(i, 2, it);

        it = new QTableWidgetItem(a->note());
        tab->setItem(i, 3, it);

        ++i;
    }
}

void MainWindow::cellEntered(int row, int column)
{
    if(column != 2 || mainPW.isEmpty()) return;

    hideVisiblePW();

    Account *a = filtered[row];
    char *pw = new char[a->max()+1];
    QByteArray desc = (a->site()+a->user()).toLocal8Bit();
    QByteArray mainPW = this->mainPW.toLocal8Bit();
    getpw(mainPW.constData(), desc.constData(), a->num(), a->min(), a->max(), a->flags(), pw);

    QTableWidgetItem *it = tab->item(row, 2);
    it->setText(pw);
    currentlyVisiblePW = row;
    delete pw;
    QTimer::singleShot(10000, this, SLOT(hideVisiblePW()));
}

void MainWindow::filter(const QString &phrase)
{
    filtered.clear();
    for(int i = 0; i < all.size(); ++i)
    {
        if(all[i].site().contains(phrase, Qt::CaseInsensitive))
            filtered.append(&all[i]);
    }
    updateTable();
}

void MainWindow::hideVisiblePW()
{
    if(currentlyVisiblePW == -1) return;

    QTableWidgetItem *it = tab->item(currentlyVisiblePW, 2);

    QString s;
    for(int j = 0; j < filtered[currentlyVisiblePW]->max(); ++j)
        s += "*";
    it->setText(s);

    currentlyVisiblePW = -1;
}

void MainWindow::lockToggled(int state)
{
    if(state == Qt::Unchecked)
    {
        QString password = QInputDialog::getText(
                this, tr("Main Password"),
                tr("Enter main password to unlock"),
                QLineEdit::Password
                );
        QByteArray b = password.toLocal8Bit();
        char code[11];
        getpw(b.constData(), "", 1, 10, 10, FLAGS_ALNUM, code);

        if(accessCode != code)
        {
            QMessageBox(
                    QMessageBox::Critical,
                    tr("Password Error"),
                    tr("Password not correct"),
                    QMessageBox::Ok
                    ).exec();
            lock->setChecked(true);
            return;
        }

        mainPW = password;
    }
    tab->setMouseTracking(!state);
}
