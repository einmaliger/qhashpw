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
#include <QtGui/QCheckBox>
#include <QtGui/QMainWindow>
#include <QtGui/QTableWidget>

#include "accountset.h"
#include "mytabwidget.h"

class QLineEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void addAccountSet(const QString &filename);

private slots:
    void filter();
    void lockToggled(int state);

private:
    MyTabWidget *center;
    QCheckBox *lock;
    QLineEdit *searchPhrase;

    AccountSet *accounts_;

    QString mainPW;

signals:
    void filterChanged(QString filter);
    void mainPWEntered(QString mainPW);
};

#endif // MAINWINDOW_H
