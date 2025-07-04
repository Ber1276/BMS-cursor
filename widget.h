#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QInputDialog>
#include <QFrame>
#include <QDateTime>
#include <QFileDialog>
#include <QThread>
#include <QProgressDialog>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QCoreApplication>
#include <QApplication>
#include <QDir>
#include <QDebug>
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
    void onImportBooks(QTableWidget *table);

private:
    Ui::Widget *ui;
    BookManager bookManager;
    UserManager userManager;
    BorrowManager *borrowManager;
    PermissionManager *permissionManager;
    
    // 全局UI组件
    QStackedWidget *mainStack;
    QPushButton *btnBook;
    QPushButton *btnBorrow;
    QPushButton *btnUser;
    
    // 登录状态管理
    bool isLoggedIn = false;
    QString currentUser;
    Role currentUserRole = USER;
    
    // 用户管理相关方法
    void setupCustomUi();
    bool showGlobalLoginDialog();
    bool showGlobalRegisterDialog();
    bool loginUser(const QString &username, const QString &password);
    bool registerUser(const QString &username, const QString &password, Role role);
    void logoutUser();
    bool checkPermissionAndNavigate(int targetPage, Role requiredRole = USER);
    void switchToPage(int pageIndex);
    void updateLoginStatus();
    void saveUserData();
    void loadUserData();
    
    // 权限检查方法
    bool hasPermission(Role requiredRole);
    void showPermissionDeniedDialog();
    
    // 页面索引常量
    static const int BOOK_PAGE = 0;
    static const int BORROW_PAGE = 1;
    static const int USER_PAGE = 2;
};
#endif // WIDGET_H
