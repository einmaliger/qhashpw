#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <QFile>
#include <QString>

class Tokenizer: public QObject
{
public:
    enum Error {
        NO_ERROR = 0,
        FILE_OPEN_ERROR,        // File could not be opened for input
        EOF_ERROR,              // nextToken called, but no more tokens in file
        BUF_OVERRUN_ERROR,      // Buffer overrun (token > 1 MB)
        QUOTED_EOF_ERROR,       // End of file reached in quoted string
        FORCE_CHAR_ERROR        // current token is not the expected char for forceCharToken
    };

    enum Type {
        TT_NOTHING,
        TT_NUMBER,
        TT_STRING,
        TT_CHAR
    };

    inline Error error() const { return error_;}
    inline Type tokT() const { return tokT_; }

    union {
        int i;
        QString *s;
        char c;
    } tok;

    // Line number of the current token
    inline int lineno() const { return lineno_;}

    // Initializes tokenizer with the given file
    Tokenizer(QFile *filename);

    // Closes the file and cleans up
    ~Tokenizer();

    // Advance to the next token
    // Returns true on success
    bool next();

    // Advance to the next token if the curren token is 'c'
    bool forceCharToken(char c);

    // Print a short description about the current token
    QString currentTokenDesc() const;

private:
    Error error_;
    QFile *f_;
    int lineno_;
    Type tokT_;
};

#endif // TOKENIZER_H
