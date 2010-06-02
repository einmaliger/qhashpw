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

#ifndef ACCOUNTSET_H
#define ACCOUNTSET_H

#include "account.h"
#include "tokenizer.h"

class AccountSet: public QObject
{
    Q_OBJECT

public:
    AccountSet();

    QString accessCode() const {return defaultAccount_.note();}

    // Constructs a read-only Account structure
    // (utilizing defaultAccount)
    // from the filtered list of accounts
    const Account at(int i) const;

    const Account operator[](int i) const
    {
        return at(i);
    }

    const DefaultAccount defaultAccount() const
    {
        return defaultAccount_;
    }

    inline QString errorMsg()
    {
        QString e = errorMsg_;
        errorMsg_ = QString(); // Nullify
        return e;
    }

    void filter(const QString &searchPhrase);

    bool readFrom(Tokenizer *t);

    int rowCount() const;
private:
    DefaultAccount defaultAccount_;

    QList<Account> all_;
    QList<Account*> filtered_;

    QString errorMsg_;
    QString filename_;

signals:
    void filterChanged();
};

#endif // ACCOUNTSET_H
