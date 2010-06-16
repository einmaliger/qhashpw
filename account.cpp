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

const int Account::INVALID_INT_FIELD = -1;

Account::Account()
: algo_(HASH_RIPEMD160), flags_(INVALID_INT_FIELD), min_(INVALID_INT_FIELD),
max_(INVALID_INT_FIELD), num_(INVALID_INT_FIELD)
{
}

Account::Account(const Account &a)
: QObject(), category_(a.category()), site_(a.site()), user_(a.user()),
  note_(a.note()), salt_(a.salt()), algo_(a.algo()), flags_(a.flags()),
  min_(a.min()), max_(a.max()), num_(a.num())
{
}

Account Account::operator=(const Account &a)
{
    return Account(a);
}

bool Account::readFrom(Tokenizer *t, const DefaultAccount *def)
{
    int version = (def == 0)?INVALID_INT_FIELD:def->version();

    // internal flag that enforces that an option
    // can only occur in the default account
    static const int DEFONLY = 0x80;
    static const int VERSIONMASK = 0x7F;

    // version identifies the minimum verison
    struct {const QString key; QString *sval; int *ival; int version;} var[] =
    {
       {"category", &category_, NULL, 2},
        {"site", &site_, NULL, 1},
        {"user", &user_, NULL, 1},
        {"note", &note_, NULL, 1},
        {"salt", &salt_, NULL, 2},
        {"flag", reinterpret_cast<QString*>(&flags_), &flags_, 1},
        {"algo", reinterpret_cast<QString*>(&algo_), &algo_, 2},
        {"min", NULL, &min_, 1},
        {"max", NULL, &max_, 1},
        {"num", NULL, &num_, 1},
        {"version", NULL, &version, 2 | DEFONLY},
        {"", NULL, NULL, INVALID_INT_FIELD}
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
        {
            raiseWarning(t, tr("Unknown field \"%1\" - ignored!").arg(key));
            goto endOfAssignment;
        }

        // We have an assignment to a known key (variable)
        // First determine if it is valid here
        if(var[i].version & DEFONLY)
        {
            if(def != 0)
            {
                raiseWarning(t, tr("Field \"%1\" not allowed here - ignored!").arg(var[i].key));
                goto endOfAssignment;
            }

        }
        else
        {
            if(def != 0 && (var[i].version & VERSIONMASK) > def->version())
            {
                raiseError(t, tr("Field \"%1\" not supported in this version").arg(var[i].key));
                goto endOfAssignment;
            }
        }

        // Okay, the assignment is valid
        if(t->tokT() == Tokenizer::TT_STRING && var[i].sval)
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
            else if(key == "algo")
            {
                if(flags_ != INVALID_INT_FIELD)
                    goto errorDoubleAssign;
                else
                    if(!doAlgoAssignment(t, *t->tok.s))
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
            if(key == "version")
            {
                Q_ASSERT(def == 0);
                dynamic_cast<DefaultAccount*>(this)->version_ = version;
            }
        }
        else raiseError(t, tr("Wrong datatype for field %1").arg(key));

endOfAssignment:
        t->next();
        if(t->tokT() == Tokenizer::TT_CHAR && t->tok.c == ',')
            t->next();
    }

    if(category_.isEmpty() && def != 0 && def->version() == 1)
        category_ = def->currentCategory();

    // enforce the closing '}' and advance to the next token
    // a comment may now be allowed
    return forceChar(t, '}', tr("Closing '}' expected"), true);
}

void Account::fillAccount(const Account &defaultAccount)
{
    if(user_.isNull()) user_ = defaultAccount.user();
    if(site_.isNull()) site_ = defaultAccount.site();
    // Do not inherit note
    if(salt_.isNull()) salt_ = defaultAccount.salt();
    if(algo_ == INVALID_INT_FIELD) algo_ = defaultAccount.algo();
    if(flags_ == INVALID_INT_FIELD) flags_ = defaultAccount.flags();
    if(min_ == INVALID_INT_FIELD) min_ = defaultAccount.min();
    if(max_ == INVALID_INT_FIELD) max_ = defaultAccount.max();
    if(num_ == INVALID_INT_FIELD) num_ = defaultAccount.num();
}

bool Account::doAlgoAssignment(const Tokenizer *t, const QString &val)
{
    if(val == "ripemd160")
        algo_ = HASH_RIPEMD160;
    else if(val == "sha1")
        algo_ = HASH_SHA1;
    else if(val == "dss1")
        algo_ = HASH_DSS1;
    else if(val == "md5")
        algo_ = HASH_MD5;
    else
    {
        raiseError(t, tr("Invalid algorithm description \"%1\"")
                   .arg(val));
        return false;
    }
    return true;

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

bool Account::forceChar(Tokenizer *t, char c, const QString &errorMsg, bool commentIsToken)
{
    if(!t->forceCharToken(c, commentIsToken))
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

void Account::saveTo(QTextStream &f, int version) const
{
    AccountSaver *saver = new AccountSaver(f, this, version);
    saver->exec();
    delete saver;
}

AccountSaver::AccountSaver(QTextStream &out, const Account *a, int version)
: a(a), firstAssignment(true), out(out), version(version)
{}

void AccountSaver::exec()
{
    const DefaultAccount *def = qobject_cast<const DefaultAccount*>(a);

    QString v;

    out << "{\n";

    if(def && version == 2)
    {
        QString s = QString(
                "\tversion = \"%1\",\n"
                "\tauthor = \"%2\"")
               .arg(def->version())
               .arg(def->author());
        firstAssignment = false;
        out << s;
    }

    writeString(1, "site", a->site());
    writeString(1, "user", a->user());
    writeString(2, "category", a->category());
    writeString(1, "note", a->note());

    v.clear();
    switch(a->flags())
    {
        case FLAGS_PRINT: v = "print"; break;
        case FLAGS_ALPHA: v = "alpha"; break;
        case FLAGS_ALNUM: v = "alnum"; break;
        case FLAGS_LOWER: v = "lower"; break;
    }
    writeString(1, "flag", v);

    writeNumber(1, "min", a->min());
    writeNumber(1, "max", a->max());
    writeNumber(1, "num", a->num());

    v.clear();
    switch(a->algo())
    {
        case HASH_RIPEMD160: v = "ripemd160"; break;
        case HASH_SHA1:      v = "sha1"; break;
        case HASH_DSS1:      v = "dss1"; break;
        case HASH_MD5:       v = "md5"; break;
    }
    writeString(2, "algo", v);

    writeString(2, "salt", a->salt());

    out << "\n}\n";
}

void AccountSaver::rawWrite(const QString &key, const QString &value)
{
    if(!value.isNull())
    {
        if(!firstAssignment)
            out << ",\n";

        out << "\t" << key << ": " << value;
        firstAssignment = false;
    }
}

void AccountSaver::writeNumber(int ver, const QString &key, int value)
{
    if(version < ver) return;
    if(value == Account::INVALID_INT_FIELD) return;
    QString v;
    v.setNum(value);
    rawWrite(key, v);
}

void AccountSaver::writeString(int ver, const QString &key, const QString &value)
{
    if(version < ver) return;
    if(value.isNull()) return;

    bool needParen = value.isEmpty() || !value[0].isLetter();

    if(!needParen)
    {
        foreach(QChar c, value)
            if(!(c.isLetterOrNumber())) needParen = true;
    }

    QString val = needParen?"\""+value+"\"":value;
    rawWrite(key, val);
}

DefaultAccount::DefaultAccount()
: Account(), version_(1)
{
}

DefaultAccount::DefaultAccount(const DefaultAccount &a)
: Account(a), author_(a.author()), currentCategory_(a.currentCategory()),
  version_(a.version())
{
}

DefaultAccount DefaultAccount::operator=(const DefaultAccount &a)
{
    return DefaultAccount(a);
}
