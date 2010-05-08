/*
 * Copyright 2010 (c) Sascha Mueller <mailbox@saschamueller.com>
 *
 * This file is part of qhashpw.
 *
 * qhashpw is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * qhashpw is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with qhashpw.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QInputDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QStatusBar>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QToolBar>

#include "hashpw.h"
#include "mainwindow.h"

MainWindow::MainWindow(AccountSet *accounts, QWidget *parent)
    : QMainWindow(parent), accounts_(accounts)
{
    QToolBar *searchBar = new QToolBar();

    searchPhrase = new QLineEdit;
    searchBar->addWidget(searchPhrase);
    searchBar->addAction(tr("Filter"), this, SLOT(filter()));
    connect(searchPhrase, SIGNAL(returnPressed()), SLOT(filter()));
    connect(accounts_, SIGNAL(filterChanged()), SLOT(updateTable()));

    this->addToolBar(Qt::TopToolBarArea, searchBar);

    tab = new QTableWidget(accounts->rowCount(), 5);

    QStringList headers;
    headers << tr("Site") << tr("User") << tr("Password") << tr("Note") << tr("");
    tab->setHorizontalHeaderLabels(headers);
    //tab->setSelectionBehavior(QAbstractItemView::SelectRows);
    tab->setSelectionMode(QAbstractItemView::NoSelection);

    connect(tab, SIGNAL(cellClicked(int,int)), SLOT(cellClicked(int,int)));
    connect(tab, SIGNAL(cellEntered(int,int)), SLOT(cellEntered(int,int)));

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
    tab->clearContents();  // note: will delete the items
    tab->setRowCount(accounts_->rowCount());

    for(int i = 0; i < accounts_->rowCount(); ++i)
    {
        Account a = accounts_->at(i);

        QTableWidgetItem *it;

        it = new QTableWidgetItem(a.site());
        tab->setItem(i, 0, it);

        it = new QTableWidgetItem(a.user());
        tab->setItem(i, 1, it);

        QString s;
        for(int j = 0; j < a.max(); ++j)
            s += "*";
        it = new QTableWidgetItem(s);
        tab->setItem(i, 2, it);

        it = new QTableWidgetItem(a.note());
        tab->setItem(i, 3, it);

        it = new QTableWidgetItem(tr("Copy"));
        tab->setItem(i, 4, it);
    }
}

void MainWindow::cellClicked(int row, int column)
{
    if(column != 4 || mainPW.isEmpty()) return;

    QApplication::clipboard()->setText(getPassword(accounts_->at(row)));
}

void MainWindow::cellEntered(int row, int column)
{
    if(column != 2 || mainPW.isEmpty() || currentlyVisiblePW == row) return;

    hideVisiblePW();

    QTableWidgetItem *it = tab->item(row, 2);
    it->setText(getPassword(accounts_->at(row)));
    currentlyVisiblePW = row;
    QTimer::singleShot(10000, this, SLOT(hideVisiblePW()));
}

QString MainWindow::getPassword(const Account &a) const
{
    char *pw = new char[a.max()+1];
    QByteArray desc = (a.site()+a.user()).toLocal8Bit();
    QByteArray mainPW = this->mainPW.toLocal8Bit();
    getpw(mainPW.constData(), desc.constData(), a.num(), a.min(), a.max(), a.flags(), pw);
    QString result = pw;
    delete pw;
    return result;
}

void MainWindow::filter()
{
    accounts_->filter(searchPhrase->text());
}

void MainWindow::hideVisiblePW()
{
    if(currentlyVisiblePW == -1) return;

    QTableWidgetItem *it = tab->item(currentlyVisiblePW, 2);

    QString s;
    for(int j = 0; j < accounts_->at(currentlyVisiblePW).max(); ++j)
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

        if(accounts_->accessCode() != code)
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
