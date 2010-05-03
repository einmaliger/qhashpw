#include <cctype>

#include "tokenizer.h"

Tokenizer::Tokenizer(QFile *f)
: error_(NO_ERROR), f_(NULL), lineno_(1), tokT_(TT_NOTHING)
{
    if(!f->isReadable())
    {
        error_ = FILE_OPEN_ERROR;
        return;
    }

    f_ = f;

    next();
}

Tokenizer::~Tokenizer()
{
    if(tokT_ == TT_STRING) delete tok.s;
}

QString Tokenizer::currentTokenDesc() const
{
    switch(tokT_)
    {
      case TT_NOTHING:
        return tr("<nothing>");
      case TT_STRING:
        return tr("<string, \"%1\">").arg(*tok.s);
      case TT_NUMBER:
        return tr("<number, %1>").arg(tok.i);
      case TT_CHAR:
        return tr("<char, '%1'>").arg(
                isprint(tok.c)?
                (isspace(tok.c)?
                 ' ':tok.c):'.');
    }
}

bool Tokenizer::next()
{
    char c;                    // The current character
    bool incomment = false;    // Are we in a comment?
    const int MAXBUF = 2<<20;  // Maximum buffer size. A megabyte string is probably not useful

    // De-initialize the old token
    if(tokT_ == TT_STRING) delete tok.s;

    // Skip all whitespace and comments
    do if(!f_->getChar(&c)) c = 0;
      else if(c == '\n')
      {
          lineno_++;
          incomment = false;
      }
      else if(c == '#') incomment = true;
    while(!f_->atEnd() && (incomment || isspace(c)));

    if(f_->atEnd())
    {
        error_ = EOF_ERROR;
        return false;
    }

    bool isQuoted = (c == '"');

    // If neither alphanumeric nor quotation mark, it's a char token
    if(!isalnum(c) && !isQuoted)
    {
        tokT_ = TT_CHAR;
        tok.c = c;
        return true;
    }

    // Parse whatever comes now as a string (either quoted or unquoted)
    int isNumber = isdigit(c);     // is it a number?

    tok.s = new QString("");
    tokT_ = TT_STRING;

    do
    {
        // if we are processing a quoted string, and this is
        // a quote character (first or second one), don't
        // add it to the string buffer
        if(isQuoted && c == '"') goto skipCharacter;

        // See if there is still room in the string buffer
        if(tok.s->length() >= MAXBUF)
        {
            error_ = BUF_OVERRUN_ERROR;
            delete tok.s;
            tokT_ = TT_NOTHING;
            return false;
        }

        // Add the current character to the string buffer
        *tok.s += c;

        // Quoted strings may contain newlines
        if(c == '\n') lineno_++;

    skipCharacter:
        char tmp;
        c = f_->getChar(&tmp) ? tmp : 0;
    } while(!f_->atEnd() && ((isQuoted && c != '"') || isalnum(c)));

    // if we did not read a quoted string, we have read one
    // character too much
    if(!isQuoted) f_->ungetChar(c);

    if(isNumber)
    {
        int i = tok.s->toInt();
        delete tok.s;
        tokT_ = TT_NUMBER;
        tok.i = i;
    }

    return true;
}

bool Tokenizer::forceCharToken(char c)
{
    if(tokT_ == TT_CHAR && tok.c == c)
    {
        next();
        return true;
    }
    error_ = FORCE_CHAR_ERROR;
    return 0;
}
