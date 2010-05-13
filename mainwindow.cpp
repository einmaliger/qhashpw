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
    // Create actions
    lockAction = new QAction(this); // icon and string will be set in updateLockAction
    lockAction->setCheckable(true);

    toClipboardAction = new QAction(QIcon("img/toclipboard.svgz"), tr("Copy"), this);
    toClipboardAction->setToolTip(tr("Copy the selected password to the clipboard"));

    // Create tool bars
    QToolBar *mainToolBar = new QToolBar;
    mainToolBar->setIconSize(QSize(QStyle::PM_SmallIconSize,QStyle::PM_SmallIconSize));
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mainToolBar->addAction(lockAction);
    mainToolBar->addAction(toClipboardAction);

    addToolBar(Qt::TopToolBarArea, mainToolBar);
    addToolBarBreak();

    QToolBar *searchBar = new QToolBar;
    searchPhrase = new QLineEdit;
    searchBar->addWidget(searchPhrase);
    searchBar->addAction(tr("Filter"), this, SLOT(filter()));
    connect(searchPhrase, SIGNAL(returnPressed()), SLOT(filter()));

    addToolBar(Qt::TopToolBarArea, searchBar);

    center = new MyTabWidget;

    setCentralWidget(center);

    updateLockAction();
    connect(center, SIGNAL(currentChanged(int)), SLOT(updateLockAction(int)));
    connect(lockAction, SIGNAL(toggled(bool)), SLOT(lockActionToggled(bool)));
    connect(toClipboardAction, SIGNAL(triggered()),SLOT(toClipboardActionTriggered()));

    //lock = new QCheckBox("Locked");
    //lock->setChecked(true);
    //connect(lock, SIGNAL(stateChanged(int)),SLOT(lockToggled(int)));

    //statusBar()->addPermanentWidget(lock);
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
        QMessageBox(QMessageBox::Critical, tr("File error"), tr("The input file could not be opened"), QMessageBox::Ok).exec();

    AccountSet *accounts = new AccountSet; // deleted by AccountSetView or in this function

    if(accounts->readFrom(t))
    {
        AccountSetView *asv = new AccountSetView(accounts); // transfers possession of accounts to asv!

        connect(this, SIGNAL(filterChanged(QString)), asv, SLOT(filter(QString)));
        connect(asv, SIGNAL(lockStateChanged()), SLOT(updateLockAction()));

        center->addTab(asv, fi.fileName());
    }
     else
    {
        QMessageBox(QMessageBox::Information, tr("Load result"), accounts->errorMsg(), QMessageBox::Ok).exec();
        delete accounts;
    }

    delete t;
}

void MainWindow::filter()
{
    emit filterChanged(searchPhrase->text());
}

void MainWindow::lockActionToggled(bool state)
{
    Q_ASSERT(center->currentSet() != 0);
    // Let the current AccountSetView handle this
    center->currentSet()->toggleLock(state);
}

void MainWindow::toClipboardActionTriggered()
{
    AccountSetView *s = center->currentSet();
    Q_ASSERT(s != 0);
    Q_ASSERT(!s->isLocked());
    s->copyCurrentPassword();
}

void MainWindow::updateLockAction(int)
{
    bool enabled = center->count() > 0 && center->currentSet() != 0;
    bool locked = !enabled || center->currentSet()->isLocked();

    lockAction->setChecked(locked);
    lockAction->setIcon(locked?QIcon("img/locked.svg"):QIcon("img/unlocked.svg"));
    lockAction->setText(locked?tr("Locked"):tr("Unlocked"));
    lockAction->setIconText(locked?tr("Locked"):tr("Unlocked"));
    lockAction->setEnabled(enabled);

    toClipboardAction->setEnabled(!locked);
}
