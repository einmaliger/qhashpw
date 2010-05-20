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

#include <QtCore/QHash>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QBoxLayout>
#include <QtGui/QClipboard>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QTableWidget>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QTreeWidget>

#include "accountsetview.h"
#include "hashpw.h"

AccountSetView::AccountSetView(AccountSet *as, const QString &filename)
    : QStackedWidget(), accounts_(as), filename_(filename), isLocked_(true)
{
    // Table/List view
    tab = new QTableWidget(as->rowCount(),4);
    QStringList headers;
    headers << tr("Site") << tr("User") << tr("Password") << tr("Note");
    tab->setHorizontalHeaderLabels(headers);
    tab->setSelectionBehavior(QAbstractItemView::SelectRows);
    tab->setSelectionMode(QAbstractItemView::SingleSelection);

    addWidget(tab);

    // Detail view
    QWidget *detailWidget = new QWidget;
    QHBoxLayout *lay = new QHBoxLayout;
    tree = new QTreeWidget;
    lay->addWidget(tree);
    detailWidget->setLayout(lay);
    addWidget(detailWidget);

    // Signal/Slots
    connect(tab, SIGNAL(cellEntered(int,int)), SLOT(cellEntered(int,int)));

    connect(accounts_, SIGNAL(filterChanged()), SLOT(updateTable()));
    connect(accounts_, SIGNAL(filterChanged()), SLOT(updateTree()));

    filter("");

    setCurrentWidget(detailWidget);
}

AccountSetView::~AccountSetView()
{
    delete accounts_;
}

void AccountSetView::copyCurrentPassword() const
{
    QTableWidget *t = const_cast<QTableWidget*>(tab);

    // At most one row, which is columnCount() cells,
    // can be selected
    Q_ASSERT(t->selectedItems().size() <= t->columnCount());

    if(t->selectedItems().size() == 0)
        QMessageBox(
                QMessageBox::Critical,
                tr("Nothing selected"),
                tr("You must select an account first"),
                QMessageBox::Ok
                ).exec();
    else
    {
        // Unfortunately selectedItems is not const, so we need to hack a bit here
        const QTableWidgetItem *w = t->selectedItems()[0];
        Account a = accounts_->at(tab->row(w));
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

    tab->item(row, column)->setText(getPassword(accounts_->at(row)));
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

    QTableWidgetItem *it = tab->item(currentlyVisiblePW, 2);

    QString s;
    for(int j = 0; j < accounts_->at(currentlyVisiblePW).max(); ++j)
        s += "*";
    it->setText(s);

    currentlyVisiblePW = -1;
}

void AccountSetView::toggleLock(bool newstate)
{
    tab->setMouseTracking(false);

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
            tab->setMouseTracking(true);
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
    }
}

void AccountSetView::updateTree()
{
    tree->clear();

    QHash<const QString&,QTreeWidgetItem*> cats; // categories

    for(int i = 0; i < accounts_->rowCount(); ++i)
    {
        Account a = accounts_->at(i);

        QTreeWidgetItem *parent;

        if(!cats.contains(a.category()))
        {
            parent = new QTreeWidgetItem(tree);
            parent->setExpanded(true);
            parent->setText(0, a.category().isEmpty()?tr("General"):a.category());
            cats[a.category()] = parent;
        }
         else parent = cats[a.category()];

        QTreeWidgetItem *it = new QTreeWidgetItem(parent);
        it->setText(0, a.site());
        it->setData(1, Qt::UserRole, i);

        // should the "user" also be displayed in the list?
        it->setData(2, Qt::UserRole, 0);

        // See if there are other entries with the same "site" as
        // the current one and if so, append the username to make
        // the list entries unique
        for(int i = 0; i < parent->childCount(); ++i)
        {
            QTreeWidgetItem *b = parent->child(i);

            if(b == it) continue;

            Account bAccount = accounts_->at(b->data(1, Qt::UserRole).toInt());

            if(bAccount.site() == a.site() && b->data(2, Qt::UserRole).toInt() == 0)
            {
                b->setText(0, tr("%1 (%2)").arg(bAccount.site()).arg(bAccount.user()));
                b->setData(2, Qt::UserRole, 1);
            }

            if(bAccount.site() == a.site())
            {
                it->setText(0, tr("%1 (%2)").arg(a.site()).arg(a.user()));
                it->setData(2, Qt::UserRole, 1);
                break;
            }
        }
    }
}
