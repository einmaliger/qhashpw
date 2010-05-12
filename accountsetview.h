#ifndef ACCOUNTSETVIEW_H
#define ACCOUNTSETVIEW_H

#include <QTableWidget>

#include "accountset.h"

class AccountSetView : public QTableWidget
{
    Q_OBJECT

public:
    AccountSetView(AccountSet *as); // transfers possession of as!
    ~AccountSetView();  // will delete accounts_!

    AccountSet *accounts() { return accounts_; }

public slots:
    void hideVisiblePW();
    void setMainPassword(const QString &mainPW);

private:
    const QString &mainPW() const { return mainPW_; }

private slots:
    void cellClicked(int row, int column);
    void cellEntered(int row, int column);
    void filter(const QString &searchPhrase);
    QString getPassword(const Account &a) const;
    void updateTable();

private:
    AccountSet *accounts_;
    int currentlyVisiblePW;     // row of password that is currently visible (or -1)
    QString mainPW_;
};

#endif // ACCOUNTSETVIEW_H
