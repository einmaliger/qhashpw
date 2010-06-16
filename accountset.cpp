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

    bool noError = defaultAccount_.readFrom(t, 0);

    s += defaultAccount_.errorMsg();

    while(noError)
    {
        if(t->tokT() == Tokenizer::TT_COMMENT)
        {
            QString s(*t->tok.s);
            t->next();
            if(defaultAccount_.version() == 1 &&
               s.startsWith("##"))
            {
                s = s.remove('#').trimmed().toLower();
                s[0] = s[0].toUpper();
                defaultAccount_.setCurrentCategory(s);
            }
        }
        Account a;
        if(t->error() == Tokenizer::EOF_ERROR) break;
        noError = a.readFrom(t, &defaultAccount_);
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

void AccountSet::saveTo(QTextStream &f)
{
    f << tr("# File created automatically by (TODO: Insert program and version number)\n")
      << "\n"
      << "# Default account\n";

    defaultAccount_.saveTo(f, defaultAccount_.version());
    f << "\n";

    QString currentCat = "";

    foreach(Account a, all_)
    {
        QString newCat = a.category();
        if(newCat != currentCat && defaultAccount_.version() == 1)
        {
            f << "####################  " << newCat << "  ####################\n\n";
            currentCat = newCat;
        }
        a.saveTo(f, defaultAccount_.version());
        f << "\n";
    }
}
