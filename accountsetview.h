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

#ifndef ACCOUNTSETVIEW_H
#define ACCOUNTSETVIEW_H

#include <QtCore/QString>
#include <QtGui/QStackedWidget>

#include "accountset.h"

class QLabel;
class QPushButton;
class QTableWidget;
class QTreeWidget;
class QTreeWidgetItem;

class AccountSetView : public QStackedWidget
{
    Q_OBJECT

public:
    AccountSetView(AccountSet *as, const QString &filename = QString()); // transfers possession of as!
    ~AccountSetView();  // will delete accounts_!

    AccountSet *accounts() { return accounts_; }
    QString filename() const { return filename_; }
    bool isLocked() { return isLocked_; }
    bool isListView();
    bool isTreeView();
    void setFilename(const QString &n)
    { filename_ = n; }

public slots:
    void copyCurrentPassword() const;
    void hideVisiblePW();
    void switchToList();
    void switchToTree();
    void toggleLock(bool newstate);

private:
    QString blindedPassword(const Account &a) const;
    const QString &mainPW() const { return mainPW_; }

private slots:
    void cellEntered(int row, int column);
    QWidget *createDetailView();
    void currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
    void detailInfoShowClicked();
    void filter(const QString &searchPhrase);
    QString getPassword(const Account &a) const;
    void updateTable();
    void updateTree();

private:
    QTableWidget *listView;
    QWidget *treeView;
    QLabel *detailInfoSite, *detailInfoUser, *detailInfoPassword;
    QPushButton *detailInfoShow;
    QTreeWidget *tree;
    AccountSet *accounts_;
    QString filename_;
    bool isLocked_;
    int currentlyVisiblePW;     // row of password that is currently visible (or -1)
    QString mainPW_;

signals:
    void lockStateChanged();
};

#endif // ACCOUNTSETVIEW_H
