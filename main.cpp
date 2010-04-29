#include <QtCore>
#include <QtGui/QApplication>
#include "account.h"
#include "mainwindow.h"
#include "tokenizer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QFile f("accounts.txt");
    f.open(QIODevice::ReadOnly);
    Tokenizer *t = new Tokenizer(&f);
    if(t->error() != Tokenizer::NO_ERROR)
        w.addInfo(QObject::tr("There was an error opening the file"));

    Account defAccount;

    bool tryOneMore = defAccount.readFrom(t);

    w.addInfo(defAccount.errorMsg());

    QList<Account> all;
    while(tryOneMore)
    {
        Account a;
        tryOneMore = a.readFrom(t);
        w.addInfo(a.errorMsg());
        all.append(a);
    }

    return a.exec();
}
