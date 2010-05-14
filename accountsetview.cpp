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

#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QTableWidgetItem>

#include "accountsetview.h"
#include "hashpw.h"

AccountSetView::AccountSetView(AccountSet *as, const QString &filename)
    : QTableWidget(as->rowCount(), 4), accounts_(as), filename_(filename), isLocked_(true)
{
    QStringList headers;
    headers << tr("Site") << tr("User") << tr("Password") << tr("Note");
    setHorizontalHeaderLabels(headers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    connect(this, SIGNAL(cellEntered(int,int)), SLOT(cellEntered(int,int)));

    connect(accounts_, SIGNAL(filterChanged()), SLOT(updateTable()));

    filter("");
}

AccountSetView::~AccountSetView()
{
    delete accounts_;
}

void AccountSetView::copyCurrentPassword() const
{
    // At most one row, which is columnCount() cells,
    // can be selected
    Q_ASSERT(selectedIndexes().size() <= columnCount());

    if(selectedIndexes().size() == 0)
        QMessageBox(
                QMessageBox::Critical,
                tr("Nothing selected"),
                tr("You must select an account first"),
                QMessageBox::Ok
                ).exec();
    else
    {
        // Unfortunately selectedItems is not const, so we need to hack a bit here
        const QTableWidgetItem *w = const_cast<AccountSetView*>(this)->selectedItems()[0];
        Account a = accounts_->at(row(w));
        QApplication::clipboard()->setText(getPassword(a));
        QMessageBox(QMessageBox::Information,
                    tr("Success"),
                    tr("The password for\n%1@%2\nwas copied to the clipboard")
                    .arg(a.user())
                    .arg(a.site()),
                    QMessageBox::Ok)
        .exec();
    }
}

void AccountSetView::cellEntered(int row, int column)
{
    if(column != 2 || isLocked_ || currentlyVisiblePW == row) return;

    hideVisiblePW();

    item(row, column)->setText(getPassword(accounts_->at(row)));
    currentlyVisiblePW = row;
    QTimer::singleShot(10000, this, SLOT(hideVisiblePW()));
}

void AccountSetView::filter(const QString &searchPhrase)
{
    accounts_->filter(searchPhrase);
    // will trigger accounts::filterChanged, which will call updateTable
}

QString AccountSetView::getPassword(const Account &a) const
{
    QByteArray mainPW = mainPW_.toLocal8Bit();
    QByteArray salt = a.salt().toLocal8Bit();
    QByteArray desc = (a.site() + a.user()).toLocal8Bit();
    struct PasswordOptions opt;

    opt.mainPW = mainPW.constData();
    opt.salt = salt.constData();
    opt.descr = desc.constData();
    opt.num = a.num();
    opt.min = a.min();
    opt.max = a.max();
    opt.flags = a.flags();
    opt.hash = a.algo();

    char *pw = new char[a.max()+1];
    getpw2(&opt, pw);
    QString result = pw;
    delete pw;
    return result;
}

void AccountSetView::hideVisiblePW()
{
    if(currentlyVisiblePW == -1) return;

    QTableWidgetItem *it = item(currentlyVisiblePW, 2);

    QString s;
    for(int j = 0; j < accounts_->at(currentlyVisiblePW).max(); ++j)
        s += "*";
    it->setText(s);

    currentlyVisiblePW = -1;
}

void AccountSetView::toggleLock(bool newstate)
{
    setMouseTracking(false);

    if(isLocked_ == true && newstate == false)
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
        }
         else
        {
            setMouseTracking(true);
            mainPW_ = password;
            isLocked_ = false;

            currentlyVisiblePW = -1;
        }
    }
     else isLocked_ = newstate;

    emit lockStateChanged();
}

void AccountSetView::updateTable()
{
    currentlyVisiblePW = -1;
    clearContents();  // note: will delete the items
    setRowCount(accounts_->rowCount());

    for(int i = 0; i < accounts_->rowCount(); ++i)
    {
        Account a = accounts_->at(i);

        QTableWidgetItem *it;

        it = new QTableWidgetItem(a.site());
        setItem(i, 0, it);

        it = new QTableWidgetItem(a.user());
        setItem(i, 1, it);

        QString s;
        for(int j = 0; j < a.max(); ++j)
            s += "*";
        it = new QTableWidgetItem(s);
        setItem(i, 2, it);

        it = new QTableWidgetItem(a.note());
        setItem(i, 3, it);
    }
}
