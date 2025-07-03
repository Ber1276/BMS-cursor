#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "include/BookManager.h"
#include "include/User.h"
#include "include/BorrowManager.h"
#include "include/PermissionManager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_clicked();
    void on_navBook_clicked();
    void on_navBorrow_clicked();
    void on_navUser_clicked();
    void on_navImport_clicked();
    void refreshBookTable(class QTableWidget *table);
    void onAddBook(class QTableWidget *table);
    void onEditBook(class QTableWidget *table);
    void onDeleteBook(class QTableWidget *table);
    void refreshBorrowTable(class QTableWidget *table);
    void onBorrowBook(class QTableWidget *table);
    void onReturnBook(class QTableWidget *table);
    void onRenewBook(class QTableWidget *table);
    void refreshUserTable(class QTableWidget *table);
    void onAddUser(class QTableWidget *table);
    void onEditUser(class QTableWidget *table);
    void onDeleteUser(class QTableWidget *table);

private:
    Ui::Widget *ui;
    BookManager bookManager;
    UserManager userManager;
    BorrowManager *borrowManager;
    PermissionManager *permissionManager;
    void setupCustomUi();
};
#endif // WIDGET_H
