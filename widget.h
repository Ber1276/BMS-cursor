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
#include "include/MyQueue.h"
#include "include/MyStack.h"
#include <QListView>
#include <QStandardItem>

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
    void saveAllData(); // 新增：保存所有持久化数据（含队列）
    void loadAllData(); // 新增：加载所有持久化数据（含队列）

private slots:
    void refreshBookTable(class QTableWidget *table);
    void refreshBookTable(QTableWidget *table, int fieldIndex, const QString &keyword);
    void onAddBook(class QTableWidget *table);
    void onEditBook(class QTableWidget *table);
    void onDeleteBook(class QTableWidget *table);
    void refreshBorrowTable(class QTableWidget *table);
    void refreshBorrowTable(QTableWidget *table, int fieldIndex, const QString &keyword);
    void onBorrowBook(class QTableWidget *table);
    void onReturnBook(class QTableWidget *table);
    void onRenewBook(class QTableWidget *table);
    void refreshUserTable(QTableWidget *table);
    void refreshUserTable(QTableWidget *table, const QString &keyword);
    void onAddUser(class QTableWidget *table);
    void onEditUser(class QTableWidget *table);
    void onDeleteUser(class QTableWidget *table);
    void onImportBooks(QTableWidget *table);
    void refreshBorrowPageTable(class QTableWidget *table, int pageNum, int pageSize);
    void refreshBorrowPageTable(QTableWidget *table, int fieldIndex, const QString &keyword, int pageNum, int pageSize);
    
    // 表格排序槽函数
    void onBookTableHeaderClicked(int logicalIndex);
    void onBorrowTableHeaderClicked(int logicalIndex);
    void onBorrowPageTableHeaderClicked(int logicalIndex);
    // 搜索历史函数
    void onHistoryItemSelected(const QModelIndex& index);

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
    QPushButton *btnBorrowBookPage;
    QLineEdit *borrowPage_searchEdit;
    QListView* historyView;
    QStandardItemModel* historyModel;
    QWidget* historyPopup;
    QTimer* inputTimer;
    
    // 登录状态管理
    bool isLoggedIn = false;
    QString currentUser;
    Role currentUserRole = USER;
    
    // 表格排序状态管理
    struct TableSortState {
        int lastSortedColumn = -1;
        bool ascending = true;
    };
    TableSortState bookTableSortState;
    TableSortState borrowTableSortState;
    TableSortState borrowPageTableSortState;

    // 搜索状态管理
    int bookTableLastFieldIndex = 0;
    QString bookTableLastKeyword;
    int borrowTableLastFieldIndex = 0;
    QString borrowTableLastKeyword;
    
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
    void saveBookAndBorrowData();
    void loadBookAndBorrowData();
    void refreshBorrowDataFromFile();
    
    // 权限检查方法
    bool hasPermission(Role requiredRole);
    void showPermissionDeniedDialog();
    void updateBorrowPageTitle();
    
    // 页面索引常量
    static const int BOOK_PAGE = 0;
    static const int BORROW_PAGE = 1;
    static const int USER_PAGE = 2;
    static const int BORROW_BOOK_PAGE = 3;

    //分页控件
    QPushButton *btnFirst, *btnPrev, *btnNext, *btnLast;
    QLabel *lblPageInfo;
    QComboBox *cmbPageSize;

    int currentBorrowPage = 1;
    int totalBorrowPage = 1;
    const int DEFAULT_PAGE_SIZE = 20;

    MyStack<QString> searchHistory;  // 用于保存搜索记录的栈
    const int MAX_HISTORY = 10;     // 最大保存记录数

    //更新页码信息
    void updatePageInfo(int pageNum, int pageSize, int totalResults);

    void handleBorrowPageBorrowClicked(const QString &isbn, const QString &title);

    // 搜索历史
    void setupSearchHistoryPopup();
    void showSearchHistory();
    void hideSearchHistory();
    void updateSearchHistory();
    void onFocusChanged(QWidget* old, QWidget* now);
};
#endif // WIDGET_H
