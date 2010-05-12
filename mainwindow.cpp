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

#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtGui/QInputDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>

#include "accountsetview.h"
#include "mainwindow.h"
#include "tokenizer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QToolBar *searchBar = new QToolBar();

    searchPhrase = new QLineEdit;
    searchBar->addWidget(searchPhrase);
    searchBar->addAction(tr("Filter"), this, SLOT(filter()));
    connect(searchPhrase, SIGNAL(returnPressed()), SLOT(filter()));

    this->addToolBar(Qt::TopToolBarArea, searchBar);

    center = new MyTabWidget;

    setCentralWidget(center);

    lock = new QCheckBox("Locked");
    lock->setChecked(true);
    connect(lock, SIGNAL(stateChanged(int)),SLOT(lockToggled(int)));

    statusBar()->addPermanentWidget(lock);
}

MainWindow::~MainWindow()
{
}

void MainWindow::addAccountSet(const QString &filename)
{
    QFileInfo fi(filename);
    QFile f(fi.absoluteFilePath());
    f.open(QIODevice::ReadOnly);
    Tokenizer *t = new Tokenizer(&f); // deleted at end of function
    if(t->error() != Tokenizer::NO_ERROR)
        QMessageBox(QMessageBox::Critical, QObject::tr("File error"), QObject::tr("The input file could not be opened"), QMessageBox::Ok).exec();

    AccountSet *accounts = new AccountSet; // deleted by AccountSetView or in this function

    if(accounts->readFrom(t))
    {
        AccountSetView *asv = new AccountSetView(accounts); // transfers possession of accounts to asv!

        connect(this, SIGNAL(mainPWEntered(QString)), asv, SLOT(setMainPassword(QString)));
        connect(this, SIGNAL(filterChanged(QString)), asv, SLOT(filter(QString)));

        center->addTab(asv, fi.fileName());
    }
     else
    {
        QMessageBox(QMessageBox::Information, QObject::tr("Load result"), accounts->errorMsg(), QMessageBox::Ok).exec();
        delete accounts;
    }

    delete t;
}

void MainWindow::filter()
{
    emit filterChanged(searchPhrase->text());
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

        emit mainPWEntered(password);
    }
}
