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

#include "accountset.h"

AccountSet::AccountSet()
{
}

const Account AccountSet::at(int i)  const
{
    Account a(*filtered_[i]);

    a.fillAccount(defaultAccount_);
    return a;
}

void AccountSet::filter(const QString &searchPhrase)
{
    filtered_.clear();
    for(int i = 0; i < all_.size(); ++i)
    {
        if(all_[i].site().contains(searchPhrase, Qt::CaseInsensitive) ||
           all_[i].note().contains(searchPhrase, Qt::CaseInsensitive))
            filtered_.append(&all_[i]);
    }

    emit filterChanged();
}

bool AccountSet::readFrom(Tokenizer *t)
{
    QString s;

    all_.clear();

    bool noError = defaultAccount_.readFrom(t);

    s += defaultAccount_.errorMsg();

    while(noError)
    {
        Account a;
        if(t->error() == Tokenizer::EOF_ERROR) break;
        noError = a.readFrom(t);
        s += a.errorMsg();
        all_.append(a);
    }

    errorMsg_ = s;

    filter("");

    return noError;
}

int AccountSet::rowCount() const
{
    return filtered_.count();
}
