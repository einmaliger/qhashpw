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
#include <QtGui/QFormLayout>
#include <QtGui/QInputDialog>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QTableWidget>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QTreeWidget>

#include "accountsetview.h"
#include "hashpw.h"

AccountSetView::AccountSetView(AccountSet *as, const QString &filename)
    : QStackedWidget(), accounts_(as), filename_(filename), isLocked_(true)
{
    // Table/List view
    listView = new QTableWidget(as->rowCount(),4);
    QStringList headers;
    headers << tr("Site") << tr("User") << tr("Password") << tr("Note");
    listView->setHorizontalHeaderLabels(headers);
    listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);

    addWidget(listView);

    // Detail view
    treeView = new QWidget;
    QHBoxLayout *lay = new QHBoxLayout;
    tree = new QTreeWidget;
    tree->setHeaderLabel(tr("Accounts"));
    tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    tree->setSelectionMode(QAbstractItemView::SingleSelection);
    lay->addWidget(tree);
    QWidget *detail = createDetailView();
    lay->addWidget(detail);
    treeView->setLayout(lay);
    addWidget(treeView);

    // Signal/Slots
    connect(listView, SIGNAL(cellEntered(int,int)), SLOT(cellEntered(int,int)));
    connect(tree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
                  SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(detailInfoShow, SIGNAL(clicked()), SLOT(detailInfoShowClicked()));

    connect(accounts_, SIGNAL(filterChanged()), SLOT(updateTable()));
    connect(accounts_, SIGNAL(filterChanged()), SLOT(updateTree()));

    filter("");

    setCurrentWidget(treeView);
}

AccountSetView::~AccountSetView()
{
    delete accounts_;
}

bool AccountSetView::isListView()
{
    return currentWidget() == dynamic_cast<QWidget*>(listView);
}

bool AccountSetView::isTreeView()
{
    return currentWidget() == treeView;
}

QString AccountSetView::blindedPassword(const Account &a) const
{
    QString s;
    for(int j = 0; j < a.max(); ++j)
        s += "*";
    return s;
}

void AccountSetView::copyCurrentPassword() const
{
    QTableWidget *t = const_cast<QTableWidget*>(listView);

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
        Account a = accounts_->at(listView->row(w));
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

    listView->item(row, column)->setText(getPassword(accounts_->at(row)));
    currentlyVisiblePW = row;
    QTimer::singleShot(10000, this, SLOT(hideVisiblePW()));
}

QWidget *AccountSetView::createDetailView()
{
    QTabWidget *d = new QTabWidget;

    // Page 1
    QWidget *w = new QWidget;
    QVBoxLayout *lmain = new QVBoxLayout;
    QFormLayout *l = new QFormLayout;
    detailInfoSite = new QLabel;
    l->addRow(tr("Site:"), detailInfoSite);
    detailInfoUser = new QLabel;
    l->addRow(tr("User:"), detailInfoUser);
    detailInfoPassword = new QLabel;
    l->addRow(tr("Password:"), detailInfoPassword);
    detailInfoShow = new QPushButton(tr("Show"));
    detailInfoShow->setEnabled(!isLocked_);
    l->addRow("", detailInfoShow);
    lmain->addLayout(l);
    lmain->addStretch();
    w->setLayout(lmain);
    d->addTab(w, tr("Info"));
    return d;
}

void AccountSetView::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
    Q_ASSERT(current != 0);

    hideVisiblePW();

    const Account a = accounts_->at(current->data(1, Qt::UserRole).toInt());
    detailInfoSite->setText(a.site());
    detailInfoUser->setText(a.user());
    detailInfoPassword->setText(blindedPassword(a));
    detailInfoShow->setEnabled(!isLocked_);
}

void AccountSetView::detailInfoShowClicked()
{
    if(tree->currentItem() == 0) return;

    int row = tree->currentItem()->data(1, Qt::UserRole).toInt();

    cellEntered(row, 2);

    detailInfoShow->setDown(true);

    detailInfoPassword->setText(getPassword(accounts_->at(row)));
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

    QTableWidgetItem *it = listView->item(currentlyVisiblePW, 2);

    QString s = blindedPassword(accounts_->at(currentlyVisiblePW));
    it->setText(s);
    detailInfoPassword->setText(s);
    detailInfoShow->setDown(false);

    currentlyVisiblePW = -1;
}

void AccountSetView::switchToList()
{
    setCurrentWidget(listView);
}

void AccountSetView::switchToTree()
{
    setCurrentWidget(treeView);
}

void AccountSetView::toggleLock(bool newstate)
{
    listView->setMouseTracking(false);
    detailInfoShow->setEnabled(false);

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
            listView->setMouseTracking(true);
            detailInfoShow->setEnabled(true);
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
    listView->clearContents();  // note: will delete the items
    listView->setRowCount(accounts_->rowCount());

    for(int i = 0; i < accounts_->rowCount(); ++i)
    {
        Account a = accounts_->at(i);

        QTableWidgetItem *it;

        it = new QTableWidgetItem(a.site());
        listView->setItem(i, 0, it);

        it = new QTableWidgetItem(a.user());
        listView->setItem(i, 1, it);

        QString s;
        for(int j = 0; j < a.max(); ++j)
            s += "*";
        it = new QTableWidgetItem(s);
        listView->setItem(i, 2, it);

        it = new QTableWidgetItem(a.note());
        listView->setItem(i, 3, it);
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
