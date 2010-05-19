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

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <QString>

#include "tokenizer.h"

class DefaultAccount;

class Account : public QObject
{
public:
    // Special value to show that a field has not been set
    static const int INVALID_INT_FIELD;

    Account();
    Account(const Account &a);
    Account operator=(const Account &a);

    // Fill in the Account structure with data read
    // from the given Tokenizer, which must currently be
    // at an opening '{' bracket of an assignment block
    // Returns 1 if successful or 0 if it failed
    // If there are no more tokens remaining, t->error
    // will be set to EOF_ERROR and false will be returned
    // the next time.
    // errorMsg will contain textual descriptions of errors
    // def is used for the version information
    // If def is 0, the calling object is assumed to
    // become the default account and be of class DefaultAccount
    virtual bool readFrom(Tokenizer *t, const DefaultAccount *def);

    // For every field that is set in default, but
    // not in this, copy the value from default
    void fillAccount(const Account &defaultAccount);

    inline QString category() const { return category_; }
    inline QString site() const { return site_; }
    inline QString user() const { return user_; }
    inline QString note() const { return note_; }
    inline QString salt() const { return salt_; }
    inline int algo() const { return algo_; }
    inline int flags() const { return flags_; }
    inline int min() const { return min_; }
    inline int max() const { return max_; }
    inline int num() const { return num_; }

    inline QString errorMsg()
    {
        QString e = errorMsg_;
        errorMsg_ = QString(); // Nullify
        return e;
    }

protected:
    bool doAlgoAssignment(const Tokenizer *t, const QString &val);
    bool doFlagAssignment(const Tokenizer *t, const QString &val);
    bool forceChar(Tokenizer *t, char c, const QString &errorMsg);
    void raiseError(const Tokenizer *t, const QString &msg);
    void raiseTokenizerError(const Tokenizer *t);
    void raiseWarning(const Tokenizer *t, const QString &msg);

    QString category_, site_, user_, note_, salt_;
    int algo_, flags_, min_, max_, num_;

    QString errorMsg_;
};

class DefaultAccount: public Account
{
    friend class Account;

public:
    DefaultAccount();

    DefaultAccount(const DefaultAccount &a);
    DefaultAccount operator=(const DefaultAccount &a);

    inline QString author() const { return author_; }
    inline QString currentCategory() const { return currentCategory_; }
    inline int version() const { return version_; }

    void setCurrentCategory(const QString &c)
    { currentCategory_ = c; }

protected:
    QString author_, currentCategory_;
    int version_;
};

#endif // ACCOUNT_H
