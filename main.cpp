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
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtCore>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include "accountset.h"
#include "mainwindow.h"
#include "tokenizer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile f("accounts.txt");
    f.open(QIODevice::ReadOnly);
    Tokenizer *t = new Tokenizer(&f);
    if(t->error() != Tokenizer::NO_ERROR)
        QMessageBox(QMessageBox::Critical, QObject::tr("File error"), QObject::tr("The input file could not be opened"), QMessageBox::Ok).exec();

    AccountSet accounts;

    if(!accounts.readFrom(t)) QMessageBox(QMessageBox::Information, QObject::tr("Load result"), accounts.errorMsg(), QMessageBox::Ok).exec();

    MainWindow w(&accounts);
    w.show();

    return a.exec();
}
