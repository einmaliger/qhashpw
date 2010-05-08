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

#include "account.h"
#include "hashpw.h"

const int Account::INVALID_INT_FIELD = 100;

Account::Account()
: flags_(INVALID_INT_FIELD), min_(INVALID_INT_FIELD), max_(INVALID_INT_FIELD),
num_(INVALID_INT_FIELD)
{
}

Account::Account(const Account &a)
: QObject(), site_(a.site()), user_(a.user()), note_(a.note()),
  flags_(a.flags()), min_(a.min()), max_(a.max()),
  num_(a.num())
{
}

Account Account::operator=(const Account &a)
{
    return Account(a);
}

bool Account::readFrom(Tokenizer *t)
{
    struct {const QString key; QString *sval; int *ival;} var[] =
    {
        {"site", &site_, NULL},
        {"user", &user_, NULL},
        {"note", &note_, NULL},
        {"flag", reinterpret_cast<QString*>(&flags_), &flags_},
        {"min", NULL, &min_},
        {"max", NULL, &max_},
        {"num", NULL, &num_},
        {"", NULL, NULL}
    };

    if(t->error() == Tokenizer::EOF_ERROR) return false;

    if(!forceChar(t, '{', tr("Expected start of assignment block")))
        return false;

    if(t->tokT() != Tokenizer::TT_STRING)
    {
        raiseError(t, tr("Expected a string (that describes a key)"));
        return false;
    }

    QString key;

    while(!(t->tokT() == Tokenizer::TT_CHAR && t->tok.c == '}'))
    {
        key = *t->tok.s;
        Q_ASSERT(!key.isNull());

        t->next();

        if(!forceChar(t, ':', tr("Expected ':'")))
            return false;

        if(t->tokT() == Tokenizer::TT_CHAR)
        {
            raiseError(t, tr("Expected a number of string as right part of assignment"));
            return false;
        }

        // Now we have either a TT_NUMBER, or a TT_STRING
        // Search our list of fields to find out what type it SHOULD be

        int i;

        for(i = 0; var[i].key != "" && var[i].key != key; ++i);

        if(var[i].key == "")
            raiseWarning(t, tr("Unknown field \"%1\" - ignoring!").arg(key));
        else if(t->tokT() == Tokenizer::TT_STRING && var[i].sval)
        {
            // String assignment
            if(key == "flag")
            {
                if(flags_ != INVALID_INT_FIELD)
                    goto errorDoubleAssign;
                else
                 if(!doFlagAssignment(t, *t->tok.s))
                    return false;
            }
             else
            {
                 if(!var[i].sval->isNull())
                     goto errorDoubleAssign;
                 else
                     *var[i].sval = *t->tok.s;
            }

        }
         else if(t->tokT() == Tokenizer::TT_NUMBER && var[i].ival)
        {
            // number assignment
            if(*var[i].ival != INVALID_INT_FIELD)
            {
errorDoubleAssign:
                raiseError(t, tr("Component %1 was already set for account %2")
                           .arg(key)
                           .arg(site_.isNull()?tr("<unnamed account>"):site_));
                return false;
            }
            *var[i].ival = t->tok.i;
        }
         else raiseError(t, tr("Wrong datatype for field %1").arg(key));

        t->next();
        if(t->tokT() == Tokenizer::TT_CHAR && t->tok.c == ',')
            t->next();
    }

    return forceChar(t, '}', tr("Closing '}' expected"));
}

void Account::fillAccount(const Account &defaultAccount)
{
    if(user_.isNull()) user_ = defaultAccount.user_;
    if(site_.isNull()) site_ = defaultAccount.site_;
    // Do not inherit note
    if(flags_ == INVALID_INT_FIELD) flags_ = defaultAccount.flags_;
    if(min_ == INVALID_INT_FIELD) min_ = defaultAccount.min_;
    if(max_ == INVALID_INT_FIELD) max_ = defaultAccount.max_;
    if(num_ == INVALID_INT_FIELD) num_ = defaultAccount.num_;
}

bool Account::doFlagAssignment(const Tokenizer *t, const QString &val)
{
    if(val == "print")
        flags_ = FLAGS_PRINT;
    else if(val == "alpha")
        flags_ = FLAGS_ALPHA;
    else if(val == "alnum")
        flags_ = FLAGS_ALNUM;
    else if(val == "lower")
        flags_ = FLAGS_LOWER;
    else
    {
        raiseError(t, tr("I don't understand the flag description \"%1\"")
                   .arg(val));
        return false;
    }
    return true;
}

bool Account::forceChar(Tokenizer *t, char c, const QString &errorMsg)
{
    if(!t->forceCharToken(c))
    {
        if(t->error() == Tokenizer::FORCE_CHAR_ERROR)
            raiseError(t, errorMsg);
        else
            raiseTokenizerError(t);
        return false;
    }
     else return true;
}

void Account::raiseError(const Tokenizer *t, const QString &msg)
{
    errorMsg_.append(
            tr("Error in line %1: %2\n")
            .arg(t->lineno())
            .arg(msg));
}

void Account::raiseTokenizerError(const Tokenizer *t)
{
    QString s;
    switch(t->error())
    {
        case Tokenizer::NO_ERROR:
            s = tr("No error - this is probably a bug in the program, sorry");
            break;
        case Tokenizer::FILE_OPEN_ERROR:
            s = tr("The input file could not be opened");
            break;
        case Tokenizer::EOF_ERROR:
            s = tr("Unexpected end of file");
            break;
        case Tokenizer::BUF_OVERRUN_ERROR:
            s = tr("Buffer overrun (some token is extremely large)");
            break;
        case Tokenizer::QUOTED_EOF_ERROR:
            s = tr("End of file reached while parsing quoted string (closing \" missing?)");
            break;
        case Tokenizer::FORCE_CHAR_ERROR:
            s = tr("Expected a certain char - this is probably a bug in the program, sorry");
            break;
        default:
            Q_ASSERT(false);
    }
    errorMsg_.append(tr("Tokenizer said: %1").arg(s));
}

void Account::raiseWarning(const Tokenizer *t, const QString &msg)
{
    errorMsg_.append(
            tr("Warning in line %1: %2\n")
            .arg(t->lineno())
            .arg(msg));
}
