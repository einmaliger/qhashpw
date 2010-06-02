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
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtGui/QApplication> // for qApp
#include <QtGui/QFileDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>

#include "accountsetview.h"
#include "mainwindow.h"
#include "tokenizer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowFilePath(QString());

    QSettings cfg;

    // Create actions
    QAction *openAction = new QAction(tr("&Open..."), this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, SIGNAL(triggered()), SLOT(open()));

    lockAction = new QAction(tr("Locked"), this);
    lockAction->setCheckable(true);
    // checked, enabled will be set in updateCurrentSet

    int maxRecentFiles = cfg.value("maxRecentFiles", 4).toInt();
    for(int i = 0; i < maxRecentFiles; ++i)
    {
        recentFileActions.append(new QAction(this));
        recentFileActions.last()->setVisible(false);
        connect(recentFileActions.last(), SIGNAL(triggered()),
                SLOT(openRecentFile()));
    }

    QAction *exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, SIGNAL(triggered()), SLOT(close()));

    toClipboardAction = new QAction(QIcon(":/img/toclipboard.svgz"), tr("Copy"), this);
    toClipboardAction->setShortcut(QKeySequence::Copy);
    toClipboardAction->setToolTip(tr("Copy the selected password to the clipboard"));
    // enabled will be set in updateCurrentSet

    viewActions = new QActionGroup(this);
    toTreeViewAction = new QAction(QIcon(":/img/view_list_tree.svgz"), tr("Tree View"), viewActions);
    toListViewAction = new QAction(QIcon(":/img/view_list_text.svgz"), tr("List View"), viewActions);
    // enabled will be set in updateCurrentSet

    QAction *aboutAction = new QAction(tr("&About"), this);

    QAction *aboutQtAction = new QAction(tr("About &Qt"), this);
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    // Create tool bars
    QToolBar *mainToolBar = new QToolBar;
    mainToolBar->setMovable(false);
    mainToolBar->setFloatable(false);
    mainToolBar->setIconSize(QSize(QStyle::PM_SmallIconSize,QStyle::PM_SmallIconSize));
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mainToolBar->addAction(lockAction);
    mainToolBar->addAction(toClipboardAction);
    mainToolBar->addSeparator();
    mainToolBar->addAction(toTreeViewAction);
    mainToolBar->addAction(toListViewAction);

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

    updateCurrentSet();
    connect(center, SIGNAL(currentChanged(int)), SLOT(updateCurrentSet(int)));
    connect(lockAction, SIGNAL(toggled(bool)), SLOT(lockActionToggled(bool)));
    connect(toClipboardAction, SIGNAL(triggered()),SLOT(toClipboardActionTriggered()));
    connect(viewActions, SIGNAL(triggered(QAction*)), SLOT(viewActionTriggered(QAction *)));

    // Menus
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(lockAction);
    separatorAction = fileMenu->addSeparator();
    for(int i = 0; i < maxRecentFiles; ++i)
        fileMenu->addAction(recentFileActions[i]);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    updateRecentFileActions();

    QMenu *accountMenu = menuBar()->addMenu(tr("&Account"));
    accountMenu->addAction(toClipboardAction);

    menuBar()->addSeparator();
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}

MainWindow::~MainWindow()
{
}

void MainWindow::addAccountSet(const QString &filename)
{
    QFileInfo fi(filename);
    QFile f(fi.absoluteFilePath());
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    Tokenizer *t = new Tokenizer(&f); // deleted at end of function
    if(t->error() != Tokenizer::NO_ERROR)
        QMessageBox(QMessageBox::Critical, tr("File error"), tr("The input file could not be opened"), QMessageBox::Ok).exec();

    AccountSet *accounts = new AccountSet; // deleted by AccountSetView or in this function

    if(accounts->readFrom(t))
    {
        AccountSetView *asv = new AccountSetView(accounts, filename); // transfers possession of accounts to asv!

        connect(this, SIGNAL(filterChanged(QString)), asv, SLOT(filter(QString)));
        connect(asv, SIGNAL(lockStateChanged()), SLOT(updateCurrentSet()));

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

void MainWindow::open()
{
    QString filename = QFileDialog::getOpenFileName(this);
    if (!filename.isEmpty()) addAccountSet(filename);
}

// From Qt example
void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        addAccountSet(action->data().toString());
}

void MainWindow::toClipboardActionTriggered()
{
    AccountSetView *s = center->currentSet();
    Q_ASSERT(s != 0);
    Q_ASSERT(!s->isLocked());
    s->copyCurrentPassword();
}

void MainWindow::updateCurrentSet(int)
{
    bool enabled = center->count() > 0 && center->currentSet() != 0;
    bool locked = !enabled || center->currentSet()->isLocked();

    QString currentFilename = enabled?center->currentSet()->filename():QString();

    setWindowFilePath(currentFilename);

    if(!currentFilename.isEmpty())
    {
        // Taken from Qt example
        QSettings settings;
        QStringList files = settings.value("recentFileList").toStringList();
        files.removeAll(currentFilename);
        files.prepend(currentFilename);
        while(files.size() > 4) files.removeLast();
        settings.setValue("recentFileList", files);
        updateRecentFileActions();
    }

    lockAction->setChecked(locked);
    lockAction->setIcon(locked?QIcon(":/img/locked.svg"):QIcon(":/img/unlocked.svg"));
    lockAction->setIconText(locked?tr("Locked"):tr("Unlocked"));
    lockAction->setEnabled(enabled);

    toClipboardAction->setEnabled(!locked);
    viewActions->setEnabled(enabled);
    toTreeViewAction->setChecked(enabled && center->currentSet()->isTreeView());
    toTreeViewAction->setChecked(enabled && center->currentSet()->isListView());
}

// Taken from Qt example, slightly modified
void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), recentFileActions.size());

    int i;
    for (i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        recentFileActions[i]->setText(text);
        recentFileActions[i]->setData(files[i]);
        recentFileActions[i]->setVisible(true);
    }
    for (; i < recentFileActions.size(); ++i)
        recentFileActions[i]->setVisible(false);

    separatorAction->setVisible(numRecentFiles > 0);
}

void MainWindow::viewActionTriggered(QAction *a)
{
    AccountSetView *s = center->currentSet();
    Q_ASSERT(s != 0);
    if(a == toTreeViewAction)
        s->switchToTree();
    else
        s->switchToList();

}
