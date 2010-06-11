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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QList>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QMainWindow>
#include <QtGui/QTableWidget>

#include "accountset.h"
#include "mytabwidget.h"

class QLineEdit;
class QMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void addAccountSet(const QString &filename);

private slots:
    void filter();
    void lockActionToggled(bool state);
    void open();
    void openRecentFile();
    void toClipboardActionTriggered();
    void viewActionTriggered(QAction *);
    void updateCurrentSet(int unused = -1);

private:
    void updateRecentFileActions();
    MyTabWidget *center;
    QLineEdit *searchPhrase;

    QAction *lockAction;
    QList<QAction*> recentFileActions;
    QAction *toClipboardAction;
    QAction *toTreeViewAction;
    QAction *toListViewAction;
    QActionGroup *viewActions;
    QAction *separatorAction; // for recentFilesMenu

signals:
    void filterChanged(QString filter);
    void mainPWEntered(QString mainPW);
};

#endif // MAINWINDOW_H
