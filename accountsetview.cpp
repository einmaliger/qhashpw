#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QMessageBox>
#include <QtGui/QTableWidgetItem>

#include "accountsetview.h"
#include "hashpw.h"

AccountSetView::AccountSetView(AccountSet *as)
    : QTableWidget(as->rowCount(), 5), accounts_(as)
{
    QStringList headers;
    headers << tr("Site") << tr("User") << tr("Password") << tr("Note") << tr("");
    setHorizontalHeaderLabels(headers);
    //setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::NoSelection);

    connect(this, SIGNAL(cellClicked(int,int)), SLOT(cellClicked(int,int)));
    connect(this, SIGNAL(cellEntered(int,int)), SLOT(cellEntered(int,int)));

    connect(accounts_, SIGNAL(filterChanged()), SLOT(updateTable()));

    filter("");
}

AccountSetView::~AccountSetView()
{
    delete accounts_;
}

void AccountSetView::cellClicked(int row, int column)
{
    if(column != 4 || mainPW().isEmpty()) return;

    QApplication::clipboard()->setText(getPassword(accounts_->at(row)));
}

void AccountSetView::cellEntered(int row, int column)
{
    if(column != 2 || mainPW().isEmpty() || currentlyVisiblePW == row) return;

    hideVisiblePW();

    item(row, column)->setText(getPassword(accounts_->at(row)));
    currentlyVisiblePW = row;
    QTimer::singleShot(10000, this, SLOT(hideVisiblePW()));
}

void AccountSetView::filter(const QString &searchPhrase)
{
    accounts_->filter(searchPhrase);
    // will trigger accounts::filterChanged, which will call updateTable
}

QString AccountSetView::getPassword(const Account &a) const
{
    char *pw = new char[a.max()+1];
    QByteArray desc = (a.site()+a.user()).toLocal8Bit();
    QByteArray mainPW = mainPW_.toLocal8Bit();
    getpw(mainPW.constData(), desc.constData(), a.num(), a.min(), a.max(), a.flags(), pw);
    QString result = pw;
    delete pw;
    return result;
}

void AccountSetView::hideVisiblePW()
{
    if(currentlyVisiblePW == -1) return;

    QTableWidgetItem *it = item(currentlyVisiblePW, 2);

    QString s;
    for(int j = 0; j < accounts_->at(currentlyVisiblePW).max(); ++j)
        s += "*";
    it->setText(s);

    currentlyVisiblePW = -1;
}

void AccountSetView::setMainPassword(const QString &password)
{
    setMouseTracking(false);

    QByteArray b = password.toLocal8Bit();
    char code[11];
    getpw(b.constData(), "", 1, 10, 10, FLAGS_ALNUM, code);

    if(accounts_->accessCode() != code)
    {
        QMessageBox(
                QMessageBox::Critical,
                tr("Password Error"),
                tr("Password not correct"),
                QMessageBox::Ok
                ).exec();
        return;
    }

    setMouseTracking(true);
    mainPW_ = password;

    currentlyVisiblePW = -1;
}


void AccountSetView::updateTable()
{
    currentlyVisiblePW = -1;
    clearContents();  // note: will delete the items
    setRowCount(accounts_->rowCount());

    for(int i = 0; i < accounts_->rowCount(); ++i)
    {
        Account a = accounts_->at(i);

        QTableWidgetItem *it;

        it = new QTableWidgetItem(a.site());
        setItem(i, 0, it);

        it = new QTableWidgetItem(a.user());
        setItem(i, 1, it);

        QString s;
        for(int j = 0; j < a.max(); ++j)
            s += "*";
        it = new QTableWidgetItem(s);
        setItem(i, 2, it);

        it = new QTableWidgetItem(a.note());
        setItem(i, 3, it);

        it = new QTableWidgetItem(tr("Copy"));
        setItem(i, 4, it);
    }
}
