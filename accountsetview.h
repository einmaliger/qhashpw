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

#include <QTableWidget>

#include "accountset.h"

class AccountSetView : public QTableWidget
{
    Q_OBJECT

public:
    AccountSetView(AccountSet *as); // transfers possession of as!
    ~AccountSetView();  // will delete accounts_!

    AccountSet *accounts() { return accounts_; }
    bool isLocked() { return isLocked_; }

public slots:
    void hideVisiblePW();
    void toggleLock(bool newstate);

private:
    const QString &mainPW() const { return mainPW_; }

private slots:
    void cellClicked(int row, int column);
    void cellEntered(int row, int column);
    void filter(const QString &searchPhrase);
    QString getPassword(const Account &a) const;
    void updateTable();

private:
    AccountSet *accounts_;
    bool isLocked_;
    int currentlyVisiblePW;     // row of password that is currently visible (or -1)
    QString mainPW_;

signals:
    void lockStateChanged();
};

#endif // ACCOUNTSETVIEW_H