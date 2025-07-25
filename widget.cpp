#include "widget.h"
#include "./ui_widget.h"
#include <QMessageBox>
#include "include/BookManager.h"
#include "include/User.h"
#include "include/BorrowManager.h"
#include "include/PermissionManager.h"
#include "include/MyQueue.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QFrame>
#include <QTableWidget>
#include <QHeaderView>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QDateTime>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QThread>
#include <QProgressDialog>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QCoreApplication>
#include <QApplication>
#include <QDir>
#include <QTimer>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , borrowManager(nullptr)
    , permissionManager(nullptr)
    , mainStack(nullptr)
    , btnBook(nullptr)
    , btnBorrow(nullptr)
    , btnUser(nullptr)
    , btnBorrowBookPage(nullptr)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    
    // 初始化管理器
    borrowManager = new BorrowManager(&bookManager, &userManager);
    permissionManager = new PermissionManager(&userManager);
    
    // 加载用户数据
    loadUserData();
    
    // 加载图书和借阅记录数据
    loadBookAndBorrowData();
    
    // 设置UI
    setupCustomUi();
    
    // 默认显示借阅图书页面（无需登录）
    switchToPage(BORROW_BOOK_PAGE);
    
    loadAllData(); // 自动加载所有持久化数据
}

Widget::~Widget()
{
    // 保存用户数据
    saveUserData();
    
    // 保存图书和借阅记录数据
    saveBookAndBorrowData();
    
    delete borrowManager;
    delete permissionManager;
    delete ui;
    
    saveAllData(); // 自动保存所有持久化数据
}

//设置UI
void Widget::setupCustomUi()
{
    // 右上角窗口控制按钮
    QWidget *titleBar = new QWidget(this);
    titleBar->setObjectName("titleBar");
    titleBar->setFixedHeight(36);
    titleBar->setStyleSheet("background: transparent;");
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(0, 0, 8, 0);
    titleLayout->addStretch();

    //登录按钮
    QPushButton *btnLogin = new QPushButton("登录", titleBar);
    btnLogin->setObjectName("btnLogin");
    btnLogin->setStyleSheet(R"(
        QPushButton {
            background: #4CAF50;
            color: white;
            border: none;
            padding: 4px 12px;
            border-radius: 4px;
            font-size: 12px;
            margin-right: 10px;
        }
        QPushButton:hover {
            background: #388E3C;
        }
        QPushButton:pressed {
            background: #2E7D32;
        }
    )");
    btnLogin->setVisible(!isLoggedIn); // 未登录时显示，登录后隐藏
    titleLayout->addWidget(btnLogin);

    // 添加登录状态显示
    QLabel *loginStatusLabel = new QLabel("未登录", titleBar);
    loginStatusLabel->setObjectName("loginStatusLabel");
    loginStatusLabel->setStyleSheet("color: #666; font-size: 12px; margin-right: 10px;");
    titleLayout->addWidget(loginStatusLabel);
    
    // 添加登出按钮
    QPushButton *btnLogout = new QPushButton("登出", titleBar);
    btnLogout->setObjectName("btnLogout");
    btnLogout->setStyleSheet(R"(
        QPushButton {
            background: #ff9800;
            color: white;
            border: none;
            padding: 4px 8px;
            border-radius: 4px;
            font-size: 11px;
            margin-right: 10px;
        }
        QPushButton:hover {
            background: #f57c00;
        }
        QPushButton:pressed {
            background: #e65100;
        }
    )");
    btnLogout->setVisible(false); // 初始隐藏
    titleLayout->addWidget(btnLogout);
    
    QPushButton *btnMin = new QPushButton("-", titleBar);
    QPushButton *btnMax = new QPushButton("□", titleBar);
    QPushButton *btnClose = new QPushButton("×", titleBar);
    
    QString winBtnStyle = R"(
    QPushButton {
        background: transparent;
        color: #888;
        font-size: 18px;
        border: none;
        width: 32px;
        height: 32px;
        border-radius: 6px;
    }
    QPushButton:hover {
        background: #e1e2e6;
        color: #222;
    }
    QPushButton:pressed {
        background: #b2bec3;
    }
    )";
    btnMin->setStyleSheet(winBtnStyle);
    btnMax->setStyleSheet(winBtnStyle);
    btnClose->setStyleSheet(winBtnStyle + "QPushButton:hover{background:#e57373;color:white;}");
    titleLayout->addWidget(btnMin);
    titleLayout->addWidget(btnMax);
    titleLayout->addWidget(btnClose);

    // 左侧导航栏
    QFrame *navBar = new QFrame(this);
    navBar->setFixedWidth(120);
    navBar->setStyleSheet("background:#222; border-top-left-radius:16px; border-bottom-left-radius:16px;");
    QVBoxLayout *navLayout = new QVBoxLayout(navBar);
    navLayout->setSpacing(20);
    navLayout->setContentsMargins(0,40,0,40);
    
    btnBook = new QPushButton("图书管理", navBar);
    btnBorrow = new QPushButton("借阅管理", navBar);
    btnUser = new QPushButton("用户管理", navBar);
    btnBorrowBookPage = new QPushButton("借阅图书", navBar);
    
    for (auto btn : {btnBook, btnBorrow, btnUser, btnBorrowBookPage}) {
        btn->setFixedHeight(40);
        btn->setStyleSheet("QPushButton{color:white;background:transparent;border:none;font-size:16px;} QPushButton:hover{background:#444;}");
        navLayout->addWidget(btn);
    }
    navLayout->addStretch();
    
    // 主内容区
    mainStack = new QStackedWidget(this);
    
    // 图书管理页
    QWidget *bookPage = new QWidget(this);
    QVBoxLayout *bookLayout = new QVBoxLayout(bookPage);
    QTableWidget *bookTable = new QTableWidget(bookPage);
    bookTable->setColumnCount(5);
    QStringList headers;
    headers << "ISBN" << "书名" << "作者" << "出版社" << "出版年份";
    bookTable->setHorizontalHeaderLabels(headers); //设置表格的水平表头标签
    bookTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //让所有列自动拉伸以填满整个表格控件的宽度
    bookTable->setEditTriggers(QAbstractItemView::NoEditTriggers); //禁用用户对表格内容的编辑功能
    bookTable->setSelectionBehavior(QAbstractItemView::SelectRows); //点击任意单元格时，整行被选中
    bookTable->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行
    
    //搜索栏
    QHBoxLayout *bookSearchLayout = new QHBoxLayout();
    bookSearchLayout->setSpacing(10);
    bookSearchLayout->setContentsMargins(15, 15, 15, 15);
    
    QComboBox *bookFieldCombo = new QComboBox(bookPage);
    bookFieldCombo->addItem("ISBN");
    bookFieldCombo->addItem("书名");
    bookFieldCombo->addItem("作者");
    bookFieldCombo->addItem("出版社");
    bookFieldCombo->addItem("出版年份");
    bookFieldCombo->setFixedWidth(120);
    
    QLineEdit *bookSearchEdit = new QLineEdit(bookPage);
    bookSearchEdit->setPlaceholderText("输入关键字搜索");
    bookSearchEdit->setMinimumWidth(200);
    
    QPushButton *btnSearchBook = new QPushButton("搜索", bookPage);
    btnSearchBook->setFixedWidth(80);
    
    // 设置搜索框样式
    QString searchStyle = R"(
        QComboBox {
            background-color: #ffffff;
            border: 2px solid #e1e2e6;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 14px;
            color: #2c3e50;
            font-weight: 500;
        }
        QComboBox:focus {
            border-color: #3498db;
            background-color: #ffffff;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #7f8c8d;
            margin-right: 8px;
        }
        QComboBox QAbstractItemView {
            background-color: #ffffff;
            border: 2px solid #e1e2e6;
            border-radius: 8px;
            selection-background-color: #3498db;
            color: #2c3e50;
            padding: 8px;
        }
        QLineEdit {
            background-color: #ffffff;
            border: 2px solid #e1e2e6;
            border-radius: 8px;
            padding: 10px 15px;
            font-size: 14px;
            color: #2c3e50;
            selection-background-color: #3498db;
        }
        QLineEdit:focus {
            border-color: #3498db;
            background-color: #ffffff;
        }
        QLineEdit::placeholder {
            color: #95a5a6;
            font-style: italic;
        }
        QPushButton {
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 15px;
            font-size: 14px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #2980b9;
        }
        QPushButton:pressed {
            background-color: #21618c;
        }
    )";
    
    bookFieldCombo->setStyleSheet(searchStyle);
    bookSearchEdit->setStyleSheet(searchStyle);
    btnSearchBook->setStyleSheet(searchStyle);
    
    bookSearchLayout->addWidget(bookFieldCombo);
    bookSearchLayout->addWidget(bookSearchEdit);
    bookSearchLayout->addWidget(btnSearchBook);
    bookSearchLayout->addStretch();
    bookLayout->insertLayout(0, bookSearchLayout); // 插入到最上方
    // 操作按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("添加图书", bookPage);
    QPushButton *btnEdit = new QPushButton("修改图书", bookPage);
    QPushButton *btnDelete = new QPushButton("删除图书", bookPage);
    QPushButton *btnImportBooks = new QPushButton("批量导入", bookPage);
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnEdit);
    btnLayout->addWidget(btnDelete);
    btnLayout->addWidget(btnImportBooks);
    btnLayout->addStretch();
    bookLayout->addLayout(btnLayout);
    bookLayout->addWidget(bookTable);
    mainStack->addWidget(bookPage);
    
    // 借阅管理页
    QWidget *borrowPage = new QWidget(this);
    QVBoxLayout *borrowLayout = new QVBoxLayout(borrowPage);
    
    // 添加页面标题
    QLabel *borrowTitleLabel = new QLabel("借阅管理", borrowPage);
    borrowTitleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333; margin: 10px;");
    borrowTitleLabel->setAlignment(Qt::AlignCenter);
    borrowLayout->addWidget(borrowTitleLabel);
    // 借阅记录表
    QTableWidget *borrowTable = new QTableWidget(borrowPage);
    borrowTable->setColumnCount(7);
    QStringList borrowHeaders;
    borrowHeaders << "记录ID" << "ISBN" << "用户名" << "借阅日期" << "到期日期" << "归还日期" << "状态";
    borrowTable->setHorizontalHeaderLabels(borrowHeaders);
    borrowTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    borrowTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    borrowTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    borrowTable->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // 搜索框
    QHBoxLayout *borrowRecord_searchLayout = new QHBoxLayout();
    borrowRecord_searchLayout->setSpacing(10);
    borrowRecord_searchLayout->setContentsMargins(15, 15, 15, 15);

    QComboBox *borrowRecordFieldCombo = new QComboBox(borrowPage);
    borrowRecordFieldCombo->addItem("记录Id");
    borrowRecordFieldCombo->addItem("ISBN");
    borrowRecordFieldCombo->addItem("用户名");
    borrowRecordFieldCombo->addItem("借阅日期");
    borrowRecordFieldCombo->addItem("到期时间");
    borrowRecordFieldCombo->addItem("状态");
    borrowRecordFieldCombo->setFixedWidth(120);

    QLineEdit *borrowRecord_searchEdit = new QLineEdit(borrowPage);
    borrowRecord_searchEdit->setPlaceholderText("请输入记录Id、ISBN、用户名等搜索");
    borrowRecord_searchEdit->setMinimumWidth(200);

    QPushButton *borrowRecord_searchBtn = new QPushButton("搜索", borrowPage);
    borrowRecord_searchBtn->setFixedWidth(80);

    borrowRecordFieldCombo->setStyleSheet(searchStyle);
    borrowRecord_searchEdit->setStyleSheet(searchStyle);
    borrowRecord_searchBtn->setStyleSheet(searchStyle);

    borrowRecord_searchLayout->addWidget(borrowRecordFieldCombo);
    borrowRecord_searchLayout->addWidget(borrowRecord_searchEdit);
    borrowRecord_searchLayout->addWidget(borrowRecord_searchBtn);
    borrowRecord_searchLayout->addStretch();
    borrowLayout->insertLayout(0,borrowRecord_searchLayout);

    // 操作按钮
    QHBoxLayout *borrowBtnLayout = new QHBoxLayout();
    QPushButton *btnBorrowBook = new QPushButton("借书", borrowPage);
    QPushButton *btnReturnBook = new QPushButton("还书", borrowPage);
    QPushButton *btnRenewBook = new QPushButton("续借", borrowPage);
    QPushButton *btnRefreshBorrow = new QPushButton("刷新记录", borrowPage);
    borrowBtnLayout->addWidget(btnBorrowBook);
    borrowBtnLayout->addWidget(btnReturnBook);
    borrowBtnLayout->addWidget(btnRenewBook);
    borrowBtnLayout->addWidget(btnRefreshBorrow);
    borrowBtnLayout->addStretch();
    borrowLayout->addLayout(borrowBtnLayout);
    borrowLayout->addWidget(borrowTable);
    mainStack->addWidget(borrowPage);
    
    // 用户管理页
    QWidget *userPage = new QWidget(this);
    QVBoxLayout *userLayout = new QVBoxLayout(userPage);
    QTableWidget *userTable = new QTableWidget(userPage);
    userTable->setColumnCount(2);
    QStringList userHeaders;
    userHeaders << "用户名" << "角色";
    userTable->setHorizontalHeaderLabels(userHeaders);
    userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    userTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTable->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // 操作按钮
    QHBoxLayout *userBtnLayout = new QHBoxLayout();
    QPushButton *btnAddUser = new QPushButton("添加用户", userPage);
    QPushButton *btnEditUser = new QPushButton("修改用户", userPage);
    QPushButton *btnDeleteUser = new QPushButton("删除用户", userPage);
    QPushButton *btnRefreshUser = new QPushButton("刷新用户", userPage);
    // 搜索框
    QHBoxLayout *userSearchLayout = new QHBoxLayout();
    userSearchLayout->setSpacing(10);
    userSearchLayout->setContentsMargins(15, 15, 15, 15);
    
    QLineEdit *userSearchEdit = new QLineEdit(userPage);
    userSearchEdit->setPlaceholderText("输入用户名关键字搜索");
    userSearchEdit->setMinimumWidth(200);
    
    QPushButton *btnSearchUser = new QPushButton("搜索", userPage);
    btnSearchUser->setFixedWidth(80);

    userSearchEdit->setStyleSheet(searchStyle);
    btnSearchUser->setStyleSheet(searchStyle);
    
    userSearchLayout->addWidget(userSearchEdit);
    userSearchLayout->addWidget(btnSearchUser);
    userSearchLayout->addStretch();
    userLayout->insertLayout(0, userSearchLayout); // 插入到最上方
    userBtnLayout->addWidget(btnAddUser);
    userBtnLayout->addWidget(btnEditUser);
    userBtnLayout->addWidget(btnDeleteUser);
    userBtnLayout->addWidget(btnRefreshUser);
    userBtnLayout->addStretch();
    userLayout->addLayout(userBtnLayout);
    userLayout->addWidget(userTable);
    mainStack->addWidget(userPage);

    // 借阅图书页
    QWidget *borrowPage_widget = new QWidget(this);
    QVBoxLayout *borrowPage_layout = new QVBoxLayout(borrowPage_widget);

    // 搜索区
    // 初始化搜索历史弹窗
    QHBoxLayout *borrowPage_searchLayout = new QHBoxLayout();
    borrowPage_searchLayout->setSpacing(10);
    borrowPage_searchLayout->setContentsMargins(15, 15, 15, 15);

    QComboBox *borrowPageFieldCombo = new QComboBox(borrowPage_widget);
    borrowPageFieldCombo->addItem("ISBN");
    borrowPageFieldCombo->addItem("书名");
    borrowPageFieldCombo->addItem("作者");
    borrowPageFieldCombo->addItem("出版社");
    borrowPageFieldCombo->addItem("出版年份");
    borrowPageFieldCombo->setFixedWidth(120);
    
    borrowPage_searchEdit = new QLineEdit(borrowPage_widget);
    borrowPage_searchEdit->setPlaceholderText("请输入书名、作者或ISBN搜索");
    borrowPage_searchEdit->setMinimumWidth(200);
    
    QPushButton *borrowPage_searchBtn = new QPushButton("搜索", borrowPage_widget);
    borrowPage_searchBtn->setFixedWidth(80);
    
    borrowPageFieldCombo->setStyleSheet(searchStyle);
    borrowPage_searchEdit->setStyleSheet(searchStyle);
    borrowPage_searchBtn->setStyleSheet(searchStyle);
    
    borrowPage_searchLayout->addWidget(borrowPageFieldCombo);
    borrowPage_searchLayout->addWidget(borrowPage_searchEdit);
    borrowPage_searchLayout->addWidget(borrowPage_searchBtn);
    borrowPage_searchLayout->addStretch();
    borrowPage_layout->insertLayout(0,borrowPage_searchLayout);


    setupSearchHistoryPopup();
    historyPopup->hide();

    // 表格
    QTableWidget *borrowPage_table = new QTableWidget(borrowPage_widget);
    borrowPage_table->setColumnCount(6);
    QStringList borrowPage_headers;
    borrowPage_headers << "ISBN" << "书名" << "作者" << "出版社" << "出版年份" << "操作";
    borrowPage_table->setHorizontalHeaderLabels(borrowPage_headers);
    borrowPage_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    borrowPage_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    borrowPage_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    borrowPage_table->setSelectionMode(QAbstractItemView::SingleSelection);
    borrowPage_layout->addWidget(borrowPage_table);

    //分页控件
    btnFirst = new QPushButton("首页", this);
    btnPrev = new QPushButton("上一页", this);
    btnNext = new QPushButton("下一页", this);
    btnLast = new QPushButton("末页", this);

    // 分页按钮样式，适配深浅色模式
    static const QString PAGE_BUTTON_STYLE = R"(
        QPushButton {
            background: palette(button);
            color: palette(button-text);
            border: 1px solid #b0b0b0;
            border-radius: 6px;
            padding: 4px 8px;
            font-size: 14px;
            font-weight: 500;
            min-width: 40px;
            min-height: 14px;
        }
        QPushButton:hover {
            background: palette(highlight);
            color: palette(highlighted-text);
            border-color: #3498db;
        }
        QPushButton:pressed {
            background: #e1e2e6;
        }
        QPushButton:disabled {
            background: #f0f0f0;
            color: #b0b0b0;
            border-color: #e0e0e0;
        }
    )";
    btnFirst->setStyleSheet(PAGE_BUTTON_STYLE);
    btnPrev->setStyleSheet(PAGE_BUTTON_STYLE);
    btnNext->setStyleSheet(PAGE_BUTTON_STYLE);
    btnLast->setStyleSheet(PAGE_BUTTON_STYLE);

    lblPageInfo = new QLabel("第 1/1 页", this);
    QLabel *pageSizeLabel = new QLabel("每页数量:");

    QString labelStyle = "color: palette(window-text); font-size: 14px; font-weight: 500; padding: 0 6px;background: palette(base);border-radius: 6px;";
    lblPageInfo->setStyleSheet(labelStyle);
    pageSizeLabel->setStyleSheet(labelStyle);

    cmbPageSize = new QComboBox(this);
    cmbPageSize->addItems({"10", "20", "50", "100"});
    cmbPageSize->setCurrentText(QString::number(DEFAULT_PAGE_SIZE));
    QString comboStyle = R"(
        QComboBox {
            color: palette(window-text);
            background: palette(base);
            border: 1px solid palette(mid);
            border-radius: 6px;
            padding: 4px 8px;
            font-size: 14px;
            font-weight: 500;
            min-width: 40px;
            min-height: 14px;
        }
        QComboBox QAbstractItemView {
            color: palette(window-text);
            background: palette(base);
            selection-background-color: palette(highlight);
            selection-color: palette(highlighted-text);
        }
    )";
    cmbPageSize->setStyleSheet(comboStyle);

    // 按钮布局
    QHBoxLayout *pageBtnLayout = new QHBoxLayout();
    pageBtnLayout->addWidget(btnFirst);
    pageBtnLayout->addWidget(btnPrev);
    pageBtnLayout->addWidget(lblPageInfo);
    pageBtnLayout->addWidget(btnNext);
    pageBtnLayout->addWidget(btnLast);
    pageBtnLayout->addStretch();
    pageBtnLayout->addWidget(pageSizeLabel);
    pageBtnLayout->addWidget(cmbPageSize);
    borrowPage_layout->addLayout(pageBtnLayout);

    mainStack->addWidget(borrowPage_widget);
    
    // 主内容区背景色
    mainStack->setStyleSheet("background:#f5f6fa; border-top-right-radius:16px; border-bottom-right-radius:16px;");
    
    // 各页面背景色
    bookPage->setStyleSheet("background:#f5f6fa;");
    borrowPage->setStyleSheet("background:#f5f6fa;");
    userPage->setStyleSheet("background:#f5f6fa;");
    borrowPage_widget->setStyleSheet("background:#f5f6fa;");
    
    // 总体布局
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(navBar);
    mainLayout->addWidget(mainStack);
    
    // 嵌入到外层垂直布局
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->setSpacing(0);
    outerLayout->addWidget(titleBar);
    outerLayout->addLayout(mainLayout);
    setLayout(outerLayout);
    
    // 导航按钮信号槽 - 使用全局权限检查
    connect(btnBook, &QPushButton::clicked, this ,[this](){ checkPermissionAndNavigate(BOOK_PAGE, ADMIN); });
    connect(btnBorrow, &QPushButton::clicked, this,[this]{ checkPermissionAndNavigate(BORROW_PAGE, USER); });
    connect(btnUser, &QPushButton::clicked, this,[this]{ checkPermissionAndNavigate(USER_PAGE, ADMIN); });
    connect(btnBorrowBookPage, &QPushButton::clicked, this,[this]{ switchToPage(BORROW_BOOK_PAGE); });
    
    // 图书管理功能信号槽 - 增删改操作需要管理员权限
    connect(btnAdd, &QPushButton::clicked, this,[this, bookTable]{
        if (hasPermission(ADMIN)) {
            onAddBook(bookTable); 
        } else {
            QMessageBox::warning(this, "权限不足", "只有管理员才能添加图书。");
        }
    });
    connect(btnEdit, &QPushButton::clicked, this,[this, bookTable]{
        if (hasPermission(ADMIN)) {
            onEditBook(bookTable); 
        } else {
            QMessageBox::warning(this, "权限不足", "只有管理员才能修改图书。");
        }
    });
    connect(btnDelete, &QPushButton::clicked, this,[this, bookTable]{
        if (hasPermission(ADMIN)) {
            onDeleteBook(bookTable); 
        } else {
            QMessageBox::warning(this, "权限不足", "只有管理员才能删除图书。");
        }
    });
    connect(btnImportBooks, &QPushButton::clicked, this,[this, bookTable]{
        if (hasPermission(ADMIN)) {
            onImportBooks(bookTable); 
        } else {
            QMessageBox::warning(this, "权限不足", "只有管理员才能批量导入图书。");
        }
    });
    // 搜索功能
    connect(btnSearchBook, &QPushButton::clicked, this, [=]{ // 搜索按钮点击事件
        refreshBookTable(bookTable, bookFieldCombo->currentIndex(), bookSearchEdit->text());
    });
    connect(bookSearchEdit, &QLineEdit::returnPressed, this, [=]{ // 回车键触发搜索
        refreshBookTable(bookTable, bookFieldCombo->currentIndex(), bookSearchEdit->text());
    });
    
    // 图书表格排序连接
    connect(bookTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &Widget::onBookTableHeaderClicked);
    
    // 借阅管理功能信号槽 - 需要登录，普通用户只能操作自己的记录
    connect(btnBorrowBook, &QPushButton::clicked, this,[this, borrowTable]{
        if (isLoggedIn) {
            onBorrowBook(borrowTable); 
        } else {
            QMessageBox::warning(this, "未登录", "请先登录后再进行借书操作。");
        }
    });
    connect(btnReturnBook, &QPushButton::clicked, this,[this, borrowTable]{
        if (isLoggedIn) {
            onReturnBook(borrowTable); 
        } else {
            QMessageBox::warning(this, "未登录", "请先登录后再进行还书操作。");
        }
    });
    connect(btnRenewBook, &QPushButton::clicked, this,[this, borrowTable]{
        if (isLoggedIn) {
            onRenewBook(borrowTable); 
        } else {
            QMessageBox::warning(this, "未登录", "请先登录后再进行续借操作。");
        }
    });

    // 刷新借阅记录按钮
    connect(btnRefreshBorrow, &QPushButton::clicked, this,[this, borrowTable, borrowRecordFieldCombo, borrowRecord_searchEdit]{
        if (isLoggedIn) {
            // 从文件重新加载数据以确保数据一致性
            refreshBorrowDataFromFile();
            // 刷新表格显示
            refreshBorrowTable(borrowTable, borrowRecordFieldCombo->currentIndex(), borrowRecord_searchEdit->text());
            QMessageBox::information(this, "刷新成功", "借阅记录已从文件重新加载并刷新！");
        } else {
            QMessageBox::warning(this, "未登录", "请先登录后再查看借阅记录。");
        }
    });

    // 搜索借阅记录
    connect(borrowRecord_searchBtn, &QPushButton::clicked, this, [=]{ // 搜索按钮点击事件
        refreshBorrowTable(borrowTable, borrowRecordFieldCombo->currentIndex(), borrowRecord_searchEdit->text());
    });
    connect(borrowRecord_searchEdit, &QLineEdit::returnPressed, this, [=]{ // 回车键触发搜索
        refreshBorrowTable(borrowTable, borrowRecordFieldCombo->currentIndex(), borrowRecord_searchEdit->text());
    });
    
    // 借阅表格排序连接
    connect(borrowTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &Widget::onBorrowTableHeaderClicked);
    
    // 用户管理功能信号槽
    connect(btnAddUser, &QPushButton::clicked, this,[this, userTable]{ onAddUser(userTable); });
    connect(btnEditUser, &QPushButton::clicked, this,[this, userTable]{ onEditUser(userTable); });
    connect(btnDeleteUser, &QPushButton::clicked, this,[this, userTable]{ onDeleteUser(userTable); });
    connect(btnRefreshUser, &QPushButton::clicked, this,[this, userTable]{ refreshUserTable(userTable); });
    // 搜索用户
    connect(btnSearchUser, &QPushButton::clicked, this, [=]{
        refreshUserTable(userTable, userSearchEdit->text().trimmed());
    });
    connect(userSearchEdit, &QLineEdit::returnPressed, this, [=]{
        refreshUserTable(userTable, userSearchEdit->text().trimmed());
    });

    // 借阅图书信号槽
    connect(borrowPage_searchBtn, &QPushButton::clicked, this, [=]{ //搜索按钮
        searchHistory.push(borrowPage_searchEdit->text());
        updateSearchHistory();
        currentBorrowPage = 1;
        refreshBorrowPageTable(borrowPage_table, borrowPageFieldCombo->currentIndex(), borrowPage_searchEdit->text(), currentBorrowPage, cmbPageSize->currentText().toInt());
    });
    connect(borrowPage_searchEdit, &QLineEdit::returnPressed, this, [=]{ //搜索框
        searchHistory.push(borrowPage_searchEdit->text());
        updateSearchHistory();
        currentBorrowPage = 1;
        refreshBorrowPageTable(borrowPage_table, borrowPageFieldCombo->currentIndex(), borrowPage_searchEdit->text(), currentBorrowPage, cmbPageSize->currentText().toInt());
    });
    // 搜索框聚焦时显示历史
    connect(qApp, &QApplication::focusChanged, this, &Widget::onFocusChanged);

    //分页控制信号槽
    connect(btnFirst, &QPushButton::clicked, this, [=](){ //首页
        currentBorrowPage = 1;
        refreshBorrowPageTable(borrowPage_table, borrowPageFieldCombo->currentIndex(), borrowPage_searchEdit->text(), currentBorrowPage, cmbPageSize->currentText().toInt());
    });

    connect(btnPrev, &QPushButton::clicked, this, [=](){ //上一页
        if (currentBorrowPage > 1) {
            --currentBorrowPage;
            refreshBorrowPageTable(borrowPage_table, borrowPageFieldCombo->currentIndex(), borrowPage_searchEdit->text(), currentBorrowPage, cmbPageSize->currentText().toInt());
        }
    });

    connect(btnNext, &QPushButton::clicked, this, [=](){ //下一页
        if (currentBorrowPage < totalBorrowPage) {
            ++currentBorrowPage;
            refreshBorrowPageTable(borrowPage_table, borrowPageFieldCombo->currentIndex(), borrowPage_searchEdit->text(), currentBorrowPage, cmbPageSize->currentText().toInt());
        }
    });

    connect(btnLast, &QPushButton::clicked, this, [=](){ //末页
        int pageSize = cmbPageSize->currentText().toInt();
        currentBorrowPage = totalBorrowPage;
        refreshBorrowPageTable(borrowPage_table, borrowPageFieldCombo->currentIndex(), borrowPage_searchEdit->text(), currentBorrowPage, pageSize);
    });

    connect(cmbPageSize, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](){ //页数
        currentBorrowPage = 1;
        refreshBorrowPageTable(borrowPage_table, borrowPageFieldCombo->currentIndex(), borrowPage_searchEdit->text(), currentBorrowPage, cmbPageSize->currentText().toInt());
    });
    
    // 借阅图书页面表格排序连接
    connect(borrowPage_table->horizontalHeader(), &QHeaderView::sectionClicked, this, &Widget::onBorrowPageTableHeaderClicked);
    
    // 窗口控制信号槽
    connect(btnMin, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(btnMax, &QPushButton::clicked, this,[this, btnMax]{
        if (isMaximized()) {
            showNormal();
            btnMax->setText("□");
        } else {
            showMaximized();
            btnMax->setText("❐");
        }
    });
    connect(btnClose, &QPushButton::clicked, this, &QWidget::close);
    connect(btnLogout, &QPushButton::clicked, this, &Widget::logoutUser);
    connect(btnLogin, &QPushButton::clicked, this, [this, btnLogin]{
        if (showGlobalLoginDialog()) {
            updateLoginStatus();
        }
    });

    // 优化按钮样式
    QString btnStyle = R"(
    QPushButton {
        color: #222;
        background: #e1e2e6;
        border-radius: 8px;
        font-size: 15px;
        padding: 6px 18px;
    }
    QPushButton:hover {
        background: #d0d1d6;
    }
    )";
    btnAdd->setStyleSheet(btnStyle);
    btnEdit->setStyleSheet(btnStyle);
    btnDelete->setStyleSheet(btnStyle);
    btnImportBooks->setStyleSheet(btnStyle);
    btnBorrowBook->setStyleSheet(btnStyle);
    btnReturnBook->setStyleSheet(btnStyle);
    btnRenewBook->setStyleSheet(btnStyle);
    btnRefreshBorrow->setStyleSheet(btnStyle);
    btnAddUser->setStyleSheet(btnStyle);
    btnEditUser->setStyleSheet(btnStyle);
    btnDeleteUser->setStyleSheet(btnStyle);
    btnRefreshUser->setStyleSheet(btnStyle);

    QString tableStyle = R"(
    QTableWidget {
        background: #fff;
        border-radius: 8px;
        border: none;
    }
    QHeaderView::section {
        background: #e1e2e6;
        color: #222;
        font-weight: bold;
        border: none;
        height: 32px;
    }
    QTableWidget::item {
        color: #222;
    }
    QTableWidget::item:selected {
        background: #b2bec3;
        color: #222;
    }
    )";
    bookTable->setStyleSheet(tableStyle);
    borrowTable->setStyleSheet(tableStyle);
    userTable->setStyleSheet(tableStyle);
    borrowPage_table->setStyleSheet(tableStyle);

    QString navBtnStyle = R"(
    QPushButton {
        color: white;
        background: transparent;
        border: none;
        font-size: 16px;
        border-radius: 8px;
    }
    QPushButton:hover {
        background: #333;
    }
    )";
    btnBook->setStyleSheet(navBtnStyle);
    btnBorrow->setStyleSheet(navBtnStyle);
    btnUser->setStyleSheet(navBtnStyle);
    btnBorrowBookPage->setStyleSheet(navBtnStyle);

    // 初始刷新
    refreshBookTable(bookTable);
    // refreshBorrowTable(borrowTable);
    refreshUserTable(userTable);
    refreshBorrowPageTable(borrowPage_table, currentBorrowPage, DEFAULT_PAGE_SIZE);
    
    // 更新登录状态显示
    updateLoginStatus();
}

// 全局登录对话框
bool Widget::showGlobalLoginDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("用户登录");
    dialog.setFixedSize(380, 320);
    dialog.setModal(true);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 25, 30, 25);
    
    // 设置对话框整体样式
    dialog.setStyleSheet(R"(
        QDialog {
            background: #fff;
            color: #222;
            border: 1px solid #e0e0e0;
        }
        QLabel {
            color: #222;
            font-size: 15px;
        }
        QLineEdit {
            background: #fff;
            border: 1.5px solid #e0e0e0;
            border-radius: 6px;
            padding: 12px 16px;
            font-size: 15px;
            color: #222;
            min-height: 28px;
        }
        QLineEdit:focus {
            border-color: #1976d2;
            background: #fff;
        }
        QLineEdit::placeholder {
            color: #b0b0b0;
            font-style: normal;
        }
        QComboBox {
            background: #fff;
            border: 1.5px solid #e0e0e0;
            border-radius: 6px;
            padding: 12px 16px;
            font-size: 15px;
            color: #222;
            min-width: 120px;
            min-height: 28px;
        }
        QComboBox:focus {
            border-color: #1976d2;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #888;
            margin-right: 8px;
        }
        QComboBox QAbstractItemView {
            background: #fff;
            border: 1.5px solid #e0e0e0;
            border-radius: 6px;
            selection-background-color: #e3f2fd;
            color: #222;
            padding: 8px;
        }
    )");
    
    // 标题区域
    QLabel *titleLabel = new QLabel("用户登录", &dialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        font-size: 22px;
        font-weight: bold;
        color: #222;
        margin: 0 0 18px 0;
        padding: 0;
        background: transparent;
    )");
    mainLayout->addWidget(titleLabel);
    
    // 表单区域
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(18);
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    QLineEdit *usernameEdit = new QLineEdit(&dialog);
    usernameEdit->setPlaceholderText("请输入用户名");
    QLineEdit *passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    
    // 设置标签样式
    QLabel *usernameLabel = new QLabel("用户名:", &dialog);
    QLabel *passwordLabel = new QLabel("密码:", &dialog);
    QString labelStyle = R"(
        font-weight: 500;
        color: #444;
        font-size: 15px;
        padding: 4px 0;
    )";
    usernameLabel->setStyleSheet(labelStyle);
    passwordLabel->setStyleSheet(labelStyle);
    
    formLayout->addRow(usernameLabel, usernameEdit);
    formLayout->addRow(passwordLabel, passwordEdit);
    mainLayout->addLayout(formLayout);
    
    // 添加一些间距
    mainLayout->addSpacing(10);
    
    // 按钮布局
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);
    
    QPushButton *btnLogin = new QPushButton("登录", &dialog);
    QPushButton *btnRegister = new QPushButton("注册", &dialog);
    QPushButton *btnCancel = new QPushButton("取消", &dialog);
    
    // 设置按钮固定大小
    btnLogin->setFixedSize(80, 36);
    btnRegister->setFixedSize(80, 36);
    btnCancel->setFixedSize(80, 36);
    
    btnLayout->addStretch();
    btnLayout->addWidget(btnLogin);
    btnLayout->addWidget(btnRegister);
    btnLayout->addWidget(btnCancel);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);
    
    // 设置按钮样式
    QString btnStyle = R"(
        QPushButton {
            background: #1976d2;
            color: #fff;
            border: none;
            border-radius: 6px;
            font-size: 15px;
            font-weight: 500;
            padding: 8px 0;
        }
        QPushButton:hover {
            background: #1565c0;
        }
        QPushButton:pressed {
            background: #0d47a1;
        }
    )";
    
    QString registerBtnStyle = btnStyle;
    QString cancelBtnStyle = R"(
        QPushButton {
            background: #e0e0e0;
            color: #666;
            border: none;
            border-radius: 6px;
            font-size: 15px;
            font-weight: 500;
            padding: 8px 0;
        }
        QPushButton:hover {
            background: #cccccc;
        }
        QPushButton:pressed {
            background: #bdbdbd;
        }
    )";
    
    btnLogin->setStyleSheet(btnStyle);
    btnRegister->setStyleSheet(registerBtnStyle);
    btnCancel->setStyleSheet(cancelBtnStyle);
    
    // 连接信号槽
    connect(btnLogin, &QPushButton::clicked, this,[this, usernameEdit, passwordEdit, &dialog]{
        QString username = usernameEdit->text().trimmed();
        QString password = passwordEdit->text();
        
        if (username.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(&dialog, "输入错误", "用户名和密码不能为空！");
            return;
        }
        
        if (loginUser(username, password)) {
            QMessageBox::information(&dialog, "登录成功", "欢迎，" + username + "！");
            dialog.accept();
        } else {
            QMessageBox::warning(&dialog, "登录失败", "用户名或密码错误！");
        }
    });
    
    connect(btnRegister, &QPushButton::clicked, this,[&]{
        dialog.reject();
        showGlobalRegisterDialog();
    });
    
    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    // 回车键登录
    connect(usernameEdit, &QLineEdit::returnPressed, this,[&]{ btnLogin->click(); });
    connect(passwordEdit, &QLineEdit::returnPressed, this,[&]{ btnLogin->click(); });
    
    int result = dialog.exec();
    return (result == QDialog::Accepted && isLoggedIn);
}

// 全局注册对话框
bool Widget::showGlobalRegisterDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("用户注册");
    dialog.setFixedSize(420, 420);
    dialog.setModal(true);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 25, 30, 25);
    
    // 设置对话框整体样式（与登录对话框保持一致）
    dialog.setStyleSheet(R"(
        QDialog {
            background: #fff;
            color: #222;
            border: 1px solid #e0e0e0;
        }
        QLabel {
            color: #222;
            font-size: 15px;
        }
        QLineEdit {
            background: #fff;
            border: 1.5px solid #e0e0e0;
            border-radius: 6px;
            padding: 12px 16px;
            font-size: 15px;
            color: #222;
            min-height: 28px;
        }
        QLineEdit:focus {
            border-color: #1976d2;
            background: #fff;
        }
        QLineEdit::placeholder {
            color: #b0b0b0;
            font-style: normal;
        }
        QComboBox {
            background: #fff;
            border: 1.5px solid #e0e0e0;
            border-radius: 6px;
            padding: 12px 16px;
            font-size: 15px;
            color: #222;
            min-width: 120px;
            min-height: 28px;
        }
        QComboBox:focus {
            border-color: #1976d2;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #888;
            margin-right: 8px;
        }
        QComboBox QAbstractItemView {
            background: #fff;
            border: 1.5px solid #e0e0e0;
            border-radius: 6px;
            selection-background-color: #e3f2fd;
            color: #222;
            padding: 8px;
        }
    )");
    
    // 标题区域
    QLabel *titleLabel = new QLabel("用户注册", &dialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        font-size: 22px;
        font-weight: bold;
        color: #222;
        margin: 0 0 18px 0;
        padding: 0;
        background: transparent;
    )");
    mainLayout->addWidget(titleLabel);
    
    // 表单区域
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(18);
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    QLineEdit *usernameEdit = new QLineEdit(&dialog);
    usernameEdit->setPlaceholderText("请输入用户名");
    QLineEdit *passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setPlaceholderText("请输入密码（至少6位）");
    QLineEdit *confirmPasswordEdit = new QLineEdit(&dialog);
    confirmPasswordEdit->setPlaceholderText("请再次输入密码");
    QComboBox *roleCombo = new QComboBox(&dialog);
    
    passwordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    roleCombo->addItem("普通用户", USER);
    roleCombo->addItem("管理员", ADMIN);
    
    // 设置标签样式
    QLabel *usernameLabel = new QLabel("用户名:", &dialog);
    QLabel *passwordLabel = new QLabel("密码:", &dialog);
    QLabel *confirmPasswordLabel = new QLabel("确认密码:", &dialog);
    QLabel *roleLabel = new QLabel("角色:", &dialog);
    
    QString labelStyle = R"(
        font-weight: 500;
        color: #444;
        font-size: 15px;
        padding: 4px 0;
    )";
    usernameLabel->setStyleSheet(labelStyle);
    passwordLabel->setStyleSheet(labelStyle);
    confirmPasswordLabel->setStyleSheet(labelStyle);
    roleLabel->setStyleSheet(labelStyle);
    
    formLayout->addRow(usernameLabel, usernameEdit);
    formLayout->addRow(passwordLabel, passwordEdit);
    formLayout->addRow(confirmPasswordLabel, confirmPasswordEdit);
    formLayout->addRow(roleLabel, roleCombo);
    mainLayout->addLayout(formLayout);
    
    // 添加一些间距
    mainLayout->addSpacing(10);
    
    // 按钮布局
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);
    
    QPushButton *btnRegister = new QPushButton("注册", &dialog);
    QPushButton *btnCancel = new QPushButton("取消", &dialog);
    
    // 设置按钮固定大小
    btnRegister->setFixedSize(80, 36);
    btnCancel->setFixedSize(80, 36);
    
    btnLayout->addStretch();
    btnLayout->addWidget(btnRegister);
    btnLayout->addWidget(btnCancel);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);
    
    // 设置按钮样式
    QString registerBtnStyle = R"(
        QPushButton {
            background: #1976d2;
            color: #fff;
            border: none;
            border-radius: 6px;
            font-size: 15px;
            font-weight: 500;
            padding: 8px 0;
        }
        QPushButton:hover {
            background: #1565c0;
        }
        QPushButton:pressed {
            background: #0d47a1;
        }
    )";
    
    QString cancelBtnStyle = R"(
        QPushButton {
            background: #e0e0e0;
            color: #666;
            border: none;
            border-radius: 6px;
            font-size: 15px;
            font-weight: 500;
            padding: 8px 0;
        }
        QPushButton:hover {
            background: #cccccc;
        }
        QPushButton:pressed {
            background: #bdbdbd;
        }
    )";
    
    btnRegister->setStyleSheet(registerBtnStyle);
    btnCancel->setStyleSheet(cancelBtnStyle);
    
    // 连接信号槽
    connect(btnRegister, &QPushButton::clicked, this,[&]{
        QString username = usernameEdit->text().trimmed();
        QString password = passwordEdit->text();
        QString confirmPassword = confirmPasswordEdit->text();
        Role role = static_cast<Role>(roleCombo->currentData().toInt());
        
        // 验证输入
        if (username.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
            QMessageBox::warning(&dialog, "输入错误", "所有字段都不能为空！");
            return;
        }
        
        if (password != confirmPassword) {
            QMessageBox::warning(&dialog, "输入错误", "两次输入的密码不一致！");
            return;
        }
        
        if (password.length() < 6) {
            QMessageBox::warning(&dialog, "输入错误", "密码长度至少6位！");
            return;
        }
        
        if (registerUser(username, password, role)) {
            QMessageBox::information(&dialog, "注册成功", "用户注册成功！请登录。");
            dialog.accept();
        } else {
            QMessageBox::warning(&dialog, "注册失败", "用户名已存在或注册失败！");
        }
    });
    
    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    // 回车键注册
    connect(usernameEdit, &QLineEdit::returnPressed, this,[&]{ passwordEdit->setFocus(); });
    connect(passwordEdit, &QLineEdit::returnPressed, this,[&]{ confirmPasswordEdit->setFocus(); });
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, this,[&]{ btnRegister->click(); });
    
    int result = dialog.exec();
    return (result == QDialog::Accepted);
}

// 权限检查和页面导航
bool Widget::checkPermissionAndNavigate(int targetPage, Role requiredRole)
{
    if (!isLoggedIn) {
        // 未登录，显示登录对话框
        if (showGlobalLoginDialog()) {
            // 登录成功，检查权限
            if (hasPermission(requiredRole)) {
                switchToPage(targetPage);
                return true;
            } else {
                showPermissionDeniedDialog();
                return false;
            }
        } else {
            // 用户取消登录，切换到借阅图书页面
            switchToPage(BORROW_BOOK_PAGE);
            return false;
        }
    } else {
        // 已登录，检查权限
        if (hasPermission(requiredRole)) {
            switchToPage(targetPage);
            return true;
        } else {
            showPermissionDeniedDialog();
            return false;
        }
    }
}

// 页面切换
void Widget::switchToPage(int pageIndex)
{
    if (mainStack && pageIndex >= 0 && pageIndex < mainStack->count()) {
        mainStack->setCurrentIndex(pageIndex);
        
        // 更新导航按钮状态
        btnBook->setStyleSheet("QPushButton{color:white;background:transparent;border:none;font-size:16px;} QPushButton:hover{background:#444;}");
        btnBorrow->setStyleSheet("QPushButton{color:white;background:transparent;border:none;font-size:16px;} QPushButton:hover{background:#444;}");
        btnUser->setStyleSheet("QPushButton{color:white;background:transparent;border:none;font-size:16px;} QPushButton:hover{background:#444;}");
        btnBorrowBookPage->setStyleSheet("QPushButton{color:white;background:transparent;border:none;font-size:16px;} QPushButton:hover{background:#444;}");

        switch (pageIndex) {
            case BOOK_PAGE:
                btnBook->setStyleSheet("QPushButton{color:white;background:#444;border:none;font-size:16px;} QPushButton:hover{background:#555;}");
                break;
            case BORROW_PAGE:
                btnBorrow->setStyleSheet("QPushButton{color:white;background:#444;border:none;font-size:16px;} QPushButton:hover{background:#555;}");
                updateBorrowPageTitle();
                // 自动刷新借阅记录表格
                {
                    QTableWidget* borrowTable = qobject_cast<QTableWidget*>(mainStack->widget(BORROW_PAGE)->findChild<QTableWidget*>());
                    if (borrowTable) {
                        refreshBorrowTable(borrowTable, borrowTableLastFieldIndex, borrowTableLastKeyword);
                    }
                }
                break;
            case USER_PAGE:
                btnUser->setStyleSheet("QPushButton{color:white;background:#444;border:none;font-size:16px;} QPushButton:hover{background:#555;}");
                // 自动刷新用户表格
                {
                    QTableWidget* userTable = qobject_cast<QTableWidget*>(mainStack->widget(USER_PAGE)->findChild<QTableWidget*>());
                    if (userTable) {
                        refreshUserTable(userTable, ""); // 默认刷新全部用户，可根据需要保存/传递上次搜索关键字
                    }
                }
                break;
            case BORROW_BOOK_PAGE:
                btnBorrowBookPage->setStyleSheet("QPushButton{color:white;background:#444;border:none;font-size:16px;} QPushButton:hover{background:#555;}");
                break;
        }
    }
}


// 权限检查
bool Widget::hasPermission(Role requiredRole)
{
    if (!isLoggedIn) return false;
    return static_cast<int>(currentUserRole) >= static_cast<int>(requiredRole);
}

// 显示权限不足对话框
void Widget::showPermissionDeniedDialog()
{
    QMessageBox::warning(this, "权限不足", 
        "您没有访问此功能的权限。\n"
        "借阅管理需要登录，用户管理需要管理员权限。");
}

// 更新借阅管理页面标题
void Widget::updateBorrowPageTitle()
{
    if (mainStack && mainStack->currentIndex() == BORROW_PAGE) {
        QWidget *borrowPage = mainStack->widget(BORROW_PAGE);
        if (borrowPage) {
            QList<QLabel*> labels = borrowPage->findChildren<QLabel*>();
            for (QLabel* label : labels) {
                if (label->text().startsWith("借阅管理")) {
                    QString title = "借阅管理";
                    if (isLoggedIn) {
                        if (hasPermission(ADMIN)) {
                            title += " (管理员模式 - 可管理所有记录)";
                        } else {
                            title += QString(" (用户：%1 - 仅显示个人记录)").arg(currentUser);
                        }
                    } else {
                        title += " (请登录)";
                    }
                    label->setText(title);
                    break;
                }
            }
        }
    }
}

// 更新登录状态显示
void Widget::updateLoginStatus()
{
    // 精确查找标题栏中的登录状态标签和登出按钮并更新
    QWidget *titleBar = findChild<QWidget*>("titleBar");
    if (titleBar) {
        QLabel *loginStatusLabel = titleBar->findChild<QLabel*>("loginStatusLabel");
        QPushButton *btnLogin = titleBar->findChild<QPushButton*>("btnLogin");
        QPushButton *btnLogout = titleBar->findChild<QPushButton*>("btnLogout");
        if (loginStatusLabel) {
            if (isLoggedIn) {
                QString roleText = (currentUserRole == ADMIN) ? "管理员" : "普通用户";
                loginStatusLabel->setText(QString("用户：%1 (%2)").arg(currentUser).arg(roleText));
                loginStatusLabel->setStyleSheet("color: #4CAF50; font-size: 12px; margin-right: 10px; font-weight: bold;");
            } else {
                loginStatusLabel->setText("未登录");
                loginStatusLabel->setStyleSheet("color: #666; font-size: 12px; margin-right: 10px;");
            }
        }
        if (btnLogout) {
            btnLogout->setVisible(isLoggedIn);
        }
        if (btnLogin) {
            btnLogin->setVisible(!isLoggedIn);
        }
    }
    
    // 如果当前在借阅管理页面，更新页面标题
    if (mainStack && mainStack->currentIndex() == BORROW_PAGE) {
        updateBorrowPageTitle();
    }
}

// 保存用户数据
void Widget::saveUserData()
{
    try {
        QString userDataPath = QCoreApplication::applicationDirPath() + "/users.json";
        userManager.saveToFile(userDataPath.toStdString());
    } catch (const std::exception &e) {
        qDebug() << "保存用户数据失败:" << e.what();
    }
}

// 加载用户数据
void Widget::loadUserData()
{
    try {
        QString userDataPath = QCoreApplication::applicationDirPath() + "/users.json";
        userManager.loadFromFile(userDataPath.toStdString());
    } catch (const std::exception &e) {
        qDebug() << "加载用户数据失败:" << e.what();
    }
}

// 用户登录
bool Widget::loginUser(const QString &username, const QString &password)
{
    if (username.isEmpty() || password.isEmpty()) return false;
    
    const User* user = userManager.findUser(username.toStdString());
    if (user && user->password == password.toStdString()) {
        isLoggedIn = true;
        currentUser = username;
        currentUserRole = user->role;
        updateLoginStatus();
        return true;
    }
    return false;
}

// 用户注册
bool Widget::registerUser(const QString &username, const QString &password, Role role)
{
    if (username.isEmpty() || password.isEmpty()) return false;
    
    // 检查用户名是否已存在
    if (userManager.findUser(username.toStdString())) {
        return false;
    }
    
    try {
        User user(username.toStdString(), password.toStdString(), role);
        userManager.addUser(user);
        saveUserData();
        return true;
    } catch (const std::exception &e) {
        qDebug() << "注册用户失败:" << e.what();
        return false;
    }
}

// 用户登出
void Widget::logoutUser()
{
    isLoggedIn = false;
    currentUser.clear();
    currentUserRole = USER;
    updateLoginStatus();
    
    // 如果当前在需要权限的页面，切换到借阅图书页面
    if (mainStack && mainStack->currentIndex() != BORROW_BOOK_PAGE) {
        switchToPage(BORROW_BOOK_PAGE);
    }
}

//刷新图书管理页面
void Widget::refreshBookTable(QTableWidget *table)
{
    table->setRowCount(0);
    const auto &books = bookManager.getAllBooks();
    for (size_t i = 0; i < books.getSize(); ++i) {
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(books[i].getIsbn())));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(books[i].getTitle())));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(books[i].getAuthor())));
        table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(books[i].getPublisher())));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(books[i].getPublishYear())));
    }
}

//按查询排序结果刷新页面
void Widget::refreshBookTable(QTableWidget *table, int fieldIndex, const QString &keyword)
{
    // 保存当前搜索条件以供按列排序时使用
    bookTableLastFieldIndex = fieldIndex;
    bookTableLastKeyword = keyword;
    table->setRowCount(0);
    MyVector<Book> result;
    //按字段调用查询函数
    QString key = keyword.trimmed();
    if (key.isEmpty()) {
        result = bookManager.getAllBooks();
    } else {
        std::string keyStr = key.toStdString();
        switch (fieldIndex) {
        case 0: //ISBN
            {
                Book* pBook = bookManager.findBookByIsbn(keyStr);
                if (pBook) {
                    result.add(*pBook); // 传递 Book 对象引用
                }
            }
            break;
        case 1: //书名
            result = bookManager.findBooksByTitle(keyStr);
            break;
        case 2: // 作者
            result = bookManager.findBooksByAuthor(keyStr);
            break;
        case 3: // 出版社
            result = bookManager.findBooksByPublisher(keyStr);
            break;
        case 4: // 出版年份
            {
                bool ok = false;
                int year = key.toInt(&ok);
                if (ok) {
                    result = bookManager.findBooksByYear(year);
                }
            }
            break;
        }
    }
    // 集成排序逻辑
    if (bookTableSortState.lastSortedColumn >= 0 && bookTableSortState.lastSortedColumn < 5) {
        SortBy sortBy;
        switch (bookTableSortState.lastSortedColumn) {
        case 0: sortBy = SortBy::ISBN; break;
        case 1: sortBy = SortBy::TITLE; break;
        case 2: sortBy = SortBy::AUTHOR; break;
        case 3: sortBy = SortBy::PUBLISHER; break;
        case 4: sortBy = SortBy::YEAR; break;
        default: sortBy = SortBy::TITLE; break;
        }
        SortOrder order = bookTableSortState.ascending ? SortOrder::ASCENDING : SortOrder::DESCENDING;
        result = bookManager.sortSearchResults(result, sortBy, order);
    }
    //展示
    for (size_t i = 0; i < result.getSize(); ++i) {
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(result[i].getIsbn())));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(result[i].getTitle())));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(result[i].getAuthor())));
        table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(result[i].getPublisher())));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(result[i].getPublishYear())));
    }
}

void Widget::onAddBook(QTableWidget *table)
{
    // 弹窗输入信息
    QDialog dialog(this);
    dialog.setWindowTitle("添加图书");
    QFormLayout form(&dialog);
    QLineEdit *isbnEdit = new QLineEdit(&dialog);
    QLineEdit *titleEdit = new QLineEdit(&dialog);
    QLineEdit *authorEdit = new QLineEdit(&dialog);
    QLineEdit *publisherEdit = new QLineEdit(&dialog);
    QLineEdit *yearEdit = new QLineEdit(&dialog);
    form.addRow("ISBN:", isbnEdit);
    form.addRow("书名:", titleEdit);
    form.addRow("作者:", authorEdit);
    form.addRow("出版社:", publisherEdit);
    form.addRow("出版年份:", yearEdit);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted) {
        QString isbn = isbnEdit->text().trimmed();
        QString title = titleEdit->text().trimmed();
        QString author = authorEdit->text().trimmed();
        QString publisher = publisherEdit->text().trimmed();
        QString yearStr = yearEdit->text().trimmed();
        if (isbn.isEmpty() || title.isEmpty() || author.isEmpty() || publisher.isEmpty() || yearStr.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "所有字段均不能为空！");
            return;
        }
        bool ok = false;
        int year = yearStr.toInt(&ok);
        if (!ok || year < 1000 || year > 2024) {
            QMessageBox::warning(this, "输入错误", "出版年份无效！");
            return;
        }
        try {
            Book book(isbn.toStdString(), title.toStdString(), author.toStdString(), publisher.toStdString(), year);
            bookManager.addBook(book);
            // 立即保存图书数据到文件，确保数据同步
            QString bookDataPath = QCoreApplication::applicationDirPath() + "/books.json";
            if (bookManager.saveToFile(bookDataPath)) {
                qDebug() << "图书数据已保存到文件:" << bookDataPath;
            } else {
                qDebug() << "图书数据保存失败:" << bookDataPath;
            }
            refreshBookTable(table);
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "添加失败", e.what());
        }
    }
}

void Widget::onEditBook(QTableWidget *table)
{
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "未选择", "请先选择要修改的图书行。");
        return;
    }
    QString oldIsbn = table->item(row, 0)->text();
    QString oldTitle = table->item(row, 1)->text();
    QString oldAuthor = table->item(row, 2)->text();
    QString oldPublisher = table->item(row, 3)->text();
    QString oldYear = table->item(row, 4)->text();
    QDialog dialog(this);
    dialog.setWindowTitle("修改图书");
    QFormLayout form(&dialog);
    QLineEdit *isbnEdit = new QLineEdit(oldIsbn, &dialog);
    QLineEdit *titleEdit = new QLineEdit(oldTitle, &dialog);
    QLineEdit *authorEdit = new QLineEdit(oldAuthor, &dialog);
    QLineEdit *publisherEdit = new QLineEdit(oldPublisher, &dialog);
    QLineEdit *yearEdit = new QLineEdit(oldYear, &dialog);
    form.addRow("ISBN:", isbnEdit);
    form.addRow("书名:", titleEdit);
    form.addRow("作者:", authorEdit);
    form.addRow("出版社:", publisherEdit);
    form.addRow("出版年份:", yearEdit);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted) {
        QString isbn = isbnEdit->text().trimmed();
        QString title = titleEdit->text().trimmed();
        QString author = authorEdit->text().trimmed();
        QString publisher = publisherEdit->text().trimmed();
        QString yearStr = yearEdit->text().trimmed();
        if (isbn.isEmpty() || title.isEmpty() || author.isEmpty() || publisher.isEmpty() || yearStr.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "所有字段均不能为空！");
            return;
        }
        bool ok = false;
        int year = yearStr.toInt(&ok);
        if (!ok || year < 1000 || year > 2024) {
            QMessageBox::warning(this, "输入错误", "出版年份无效！");
            return;
        }
        try {
            Book newBook(isbn.toStdString(), title.toStdString(), author.toStdString(), publisher.toStdString(), year);
            if (!bookManager.updateBook(oldIsbn.toStdString(), newBook)) {
                QMessageBox::warning(this, "修改失败", "未找到原图书或更新失败。");
                return;
            }
            // 立即保存图书数据到文件，确保数据同步
            QString bookDataPath = QCoreApplication::applicationDirPath() + "/books.json";
            if (bookManager.saveToFile(bookDataPath)) {
                qDebug() << "图书数据已保存到文件:" << bookDataPath;
            } else {
                qDebug() << "图书数据保存失败:" << bookDataPath;
            }
            refreshBookTable(table);
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "修改失败", e.what());
        }
    }
}

void Widget::onDeleteBook(QTableWidget *table)
{
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "未选择", "请先选择要删除的图书行。");
        return;
    }
    QString isbn = table->item(row, 0)->text();
    int ret = QMessageBox::question(this, "确认删除", QString("确定要删除图书：%1 吗？").arg(isbn), QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        if (!bookManager.removeBook(isbn.toStdString())) {
            QMessageBox::warning(this, "删除失败", "未找到该图书或删除失败。");
            return;
        }
        // 立即保存图书数据到文件，确保数据同步
        QString bookDataPath = QCoreApplication::applicationDirPath() + "/books.json";
        if (bookManager.saveToFile(bookDataPath)) {
            qDebug() << "图书数据已保存到文件:" << bookDataPath;
        } else {
            qDebug() << "图书数据保存失败:" << bookDataPath;
        }
        refreshBookTable(table);
    }
}


// 刷新借阅记录表格
void Widget::refreshBorrowTable(QTableWidget *table)
{
    table->setRowCount(0);
    
    if (!isLoggedIn) {
        QMessageBox::warning(this, "未登录", "请先登录后再查看借阅记录。");
        return;
    }
    
    // 根据用户权限显示不同的记录
    MyVector<BorrowRecord> records;
    if (hasPermission(ADMIN)) {
        // 管理员可以看到所有记录
        records = borrowManager->getAllBorrowRecords();
    } else {
        // 普通用户只能看到自己的记录
        records = borrowManager->getUserBorrowRecords(currentUser.toStdString());
    }
    
    for (size_t i = 0; i < records.getSize(); ++i) {
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(records[i].getRecordId())));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(records[i].getIsbn())));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(records[i].getUsername())));
        table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(records[i].getBorrowDateStr())));
        table->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(records[i].getDueDateStr())));
        table->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(records[i].getReturnDateStr())));
        table->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(records[i].getStatus())));
    }
}

// 刷新借阅记录表格，支持按字段查询和排序
void Widget::refreshBorrowTable(QTableWidget *table, int fieldIndex, const QString &keyword)
{
    // 保存当前搜索条件
    borrowTableLastFieldIndex = fieldIndex;
    borrowTableLastKeyword = keyword;
    table->setRowCount(0);

    if (!isLoggedIn) {
        QMessageBox::warning(this, "未登录", "请先登录后再查看借阅记录。");
        return;
    }

    // 根据用户权限显示不同的记录
    MyVector<BorrowRecord> records;
    if (hasPermission(ADMIN)) {
        // 管理员可以看到所有记录
        records = borrowManager->getAllBorrowRecords();
    } else {
        // 普通用户只能看到自己的记录
        records = borrowManager->getUserBorrowRecords(currentUser.toStdString());
    }
    MyVector<BorrowRecord> result;
    //按字段调用查询函数
    QString key = keyword.trimmed();
    if (key.isEmpty()) {
        result = records;
    } else {
        std::string keyStr = key.toStdString();
        switch (fieldIndex) {
        case 0: //记录ID
        {
            BorrowRecord *borrowRecord = borrowManager->findByRecordId(records,keyStr);
            if (borrowRecord) {
                result.add(*borrowRecord);
            }
        }
        break;
        case 1: // ISBN
            result = borrowManager->findByISBN(records,keyStr);
            break;
        case 2: // 用户名
            result = borrowManager->findByUsername(records,keyStr);
            break;
        case 3: // 借阅时间
            result = borrowManager->findByBorrowDate(records,keyStr);
            break;
        case 4: // 到期时间
            result = borrowManager->findByDueDate(records,keyStr);
            break;
        case 5: // 状态
            result = borrowManager->findByStatus(records,keyStr);
            break;
        }
    }
    // 集成排序逻辑
    if (borrowTableSortState.lastSortedColumn >= 0 && borrowTableSortState.lastSortedColumn < 7) {
        BorrowSortBy sortBy;
        switch (borrowTableSortState.lastSortedColumn) {
        case 0: sortBy = BorrowSortBy::RECORD_ID; break;
        case 1: sortBy = BorrowSortBy::ISBN; break;
        case 2: sortBy = BorrowSortBy::USERNAME; break;
        case 3: sortBy = BorrowSortBy::BORROW_DATE; break;
        case 4: sortBy = BorrowSortBy::DUE_DATE; break;
        case 5: sortBy = BorrowSortBy::RETURN_DATE; break;
        case 6: sortBy = BorrowSortBy::STATUS; break;
        default: sortBy = BorrowSortBy::RECORD_ID; break;
        }
        BorrowSortOrder order = borrowTableSortState.ascending ? BorrowSortOrder::ASCENDING : BorrowSortOrder::DESCENDING;
        result = borrowManager->sortSearchResults(result, sortBy, order);
    }
    // 展示结果
    for (size_t i = 0; i < result.getSize(); ++i) {
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(result[i].getRecordId())));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(result[i].getIsbn())));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(result[i].getUsername())));
        table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(result[i].getBorrowDateStr())));
        table->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(result[i].getDueDateStr())));
        table->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(result[i].getReturnDateStr())));
        table->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(result[i].getStatus())));
    }
}

void Widget::onBorrowBook(QTableWidget *table)
{
    if (!isLoggedIn) {
        QMessageBox::warning(this, "未登录", "请先登录后再进行借书操作。");
        return;
    }
    // 弹窗选择图书
    QDialog dialog(this);
    dialog.setWindowTitle("借书");
    QFormLayout form(&dialog);
    QComboBox *bookCombo = new QComboBox(&dialog);
    const auto &books = bookManager.getAllBooks();
    for (size_t i = 0; i < books.getSize(); ++i) {
        QString bookInfo = QString("%1 - %2").arg(QString::fromStdString(books[i].getIsbn())).arg(QString::fromStdString(books[i].getTitle()));
        bookCombo->addItem(bookInfo, QString::fromStdString(books[i].getIsbn()));
    }
    form.addRow("选择图书:", bookCombo);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted) {
        QString isbn = bookCombo->currentData().toString();
        try {
            borrowManager->borrowBook(isbn.toStdString(), currentUser.toStdString());
            borrowManager->saveToFile("borrow_records.json");
            borrowManager->saveWaitingQueues("waiting_queues.json");
            refreshBorrowTable(table);
            QMessageBox::information(this, "借书成功", "图书借阅成功！");
        } catch (const std::exception &e) {
            QMessageBox::information(this, "借书提示", e.what());
        }
    }
}

void Widget::onReturnBook(QTableWidget *table)
{
    if (!isLoggedIn) {
        QMessageBox::warning(this, "未登录", "请先登录后再进行还书操作。");
        return;
    }
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "未选择", "请先选择要归还的借阅记录。");
        return;
    }
    QString recordUsername = table->item(row, 2)->text();
    if (!hasPermission(ADMIN) && recordUsername != currentUser) {
        QMessageBox::warning(this, "权限不足", "您只能归还自己的借阅记录。");
        return;
    }
    QString recordId = table->item(row, 0)->text();
    try {
        bool ok = borrowManager->returnBookByRecordId(recordId.toStdString());
        borrowManager->saveToFile("borrow_records.json");
        borrowManager->saveWaitingQueues("waiting_queues.json");
        refreshBorrowTable(table);
        if (ok) {
            QMessageBox::information(this, "还书成功", "图书归还成功！");
        } else {
            QMessageBox::warning(this, "还书失败", "未找到借阅记录或已归还");
        }
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "还书失败", e.what());
    }
}

void Widget::onRenewBook(QTableWidget *table)
{
    if (!isLoggedIn) {
        QMessageBox::warning(this, "未登录", "请先登录后再进行续借操作。");
        return;
    }
    
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "未选择", "请先选择要续借的借阅记录。");
        return;
    }
    
    // 检查权限：普通用户只能续借自己的记录，管理员可以续借所有记录
    QString recordUsername = table->item(row, 2)->text();
    if (!hasPermission(ADMIN) && recordUsername != currentUser) {
        QMessageBox::warning(this, "权限不足", "您只能续借自己的借阅记录。");
        return;
    }
    
    int recordId = table->item(row, 0)->text().toInt();
    try {
        borrowManager->renewBook(recordId);
        // 立即保存借阅记录到文件，确保数据同步
        QString borrowDataPath = QCoreApplication::applicationDirPath() + "/borrow_records.json";
        if (borrowManager->saveToFile(borrowDataPath)) {
            qDebug() << "借阅记录已保存到文件:" << borrowDataPath;
        } else {
            qDebug() << "借阅记录保存失败:" << borrowDataPath;
        }
        refreshBorrowTable(table);
        QMessageBox::information(this, "续借成功", "图书续借成功！");
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "续借失败", e.what());
    }
}

//刷新用户表
void Widget::refreshUserTable(QTableWidget *table)
{
    table->setRowCount(0);
    const auto &users = userManager.getAllUsers();
    for (size_t i = 0; i < users.getSize(); ++i) {
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(users[i].username)));
        table->setItem(i, 1, new QTableWidgetItem(users[i].role == ADMIN ? "管理员" : "普通用户"));
    }
}

// 刷新用户表支持按用户名查询
void Widget::refreshUserTable(QTableWidget *table, const QString &keyword)
{
    table->setRowCount(0);
    MyVector<User> searchUserResult = keyword.isEmpty()? userManager.getAllUsers(): userManager.fuzzyFindUsers(keyword.toStdString());
    for (size_t i = 0; i < searchUserResult.getSize(); ++i) {
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(searchUserResult[i].username)));
        table->setItem(i, 1, new QTableWidgetItem(searchUserResult[i].role == ADMIN ? "管理员" : "普通用户"));
    }
}

// 添加用户
void Widget::onAddUser(QTableWidget *table)
{
    if (!hasPermission(ADMIN)) {
        QMessageBox::warning(this, "权限不足", "只有管理员才能添加用户。");
        return;
    }
    
    //开启添加用户对话框
    QDialog dialog(this);
    dialog.setWindowTitle("添加用户");
    QFormLayout form(&dialog);
    QLineEdit *usernameEdit = new QLineEdit(&dialog);
    QLineEdit *passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setEchoMode(QLineEdit::Password);
    QComboBox *roleCombo = new QComboBox(&dialog);
    roleCombo->addItem("普通用户", USER);
    roleCombo->addItem("管理员", ADMIN);
    form.addRow("用户名:", usernameEdit);
    form.addRow("密码:", passwordEdit);
    form.addRow("角色:", roleCombo);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted) { //确认后开始添加
        QString username = usernameEdit->text().trimmed();
        QString password = passwordEdit->text();
        Role role = static_cast<Role>(roleCombo->currentData().toInt());
        if (username.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "用户名和密码不能为空！");
            return;
        }
        if (userManager.findUser(username.toStdString())) {
            QMessageBox::warning(this, "添加失败", "用户名已存在！");
            return;
        }
        try {
            User user(username.toStdString(), password.toStdString(), role);
            userManager.addUser(user); //添加用户
            saveUserData(); //保存用户
            refreshUserTable(table); //刷新用户
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "添加失败", e.what());
        }
    }
}

//修改用户
void Widget::onEditUser(QTableWidget *table)
{
    if (!hasPermission(ADMIN)) {
        QMessageBox::warning(this, "权限不足", "只有管理员才能修改用户。");
        return;
    }
    
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "未选择", "请先选择要修改的用户行。");
        return;
    }
    QString oldUsername = table->item(row, 0)->text();
    const User* userPtr = userManager.findUser(oldUsername.toStdString());
    if (!userPtr) {
        QMessageBox::warning(this, "未找到", "未找到该用户，可能已被删除。");
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle("修改用户");
    QFormLayout form(&dialog);
    QLineEdit *usernameEdit = new QLineEdit(oldUsername, &dialog);
    QLineEdit *passwordEdit = new QLineEdit(QString::fromStdString(userPtr->password), &dialog);
    passwordEdit->setEchoMode(QLineEdit::Password);
    //qcombobox 下拉框
    QComboBox *roleCombo = new QComboBox(&dialog);
    roleCombo->addItem("普通用户", USER);
    roleCombo->addItem("管理员", ADMIN);
    roleCombo->setCurrentIndex(userPtr->role == ADMIN ? 1 : 0);
    form.addRow("用户名:", usernameEdit);
    form.addRow("密码:", passwordEdit);
    form.addRow("角色:", roleCombo);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted) {
        QString username = usernameEdit->text().trimmed();
        QString password = passwordEdit->text();
        Role role = static_cast<Role>(roleCombo->currentData().toInt());
        if (username.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "用户名和密码不能为空！");
            return;
        }
        // 用户名变更且新用户名已存在
        if (username != oldUsername && userManager.findUser(username.toStdString())) {
            QMessageBox::warning(this, "修改失败", "新用户名已存在！");
            return;
        }
        try {
            User newUser(username.toStdString(), password.toStdString(), role);
            if (!userManager.updateUser(oldUsername.toStdString(), newUser)) {
                QMessageBox::warning(this, "修改失败", "未找到原用户或更新失败。");
                return;
            }
            saveUserData();
            refreshUserTable(table);
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "修改失败", e.what());
        }
    }
}

//删除用户
void Widget::onDeleteUser(QTableWidget *table)
{
    if (!hasPermission(ADMIN)) {
        QMessageBox::warning(this, "权限不足", "只有管理员才能删除用户。");
        return;
    }
    
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "未选择", "请先选择要删除的用户行。");
        return;
    }
    QString username = table->item(row, 0)->text();
    
    // 不能删除当前登录用户
    if (username == currentUser) {
        QMessageBox::warning(this, "删除失败", "不能删除当前登录用户！");
        return;
    }
    //弹出确认框
    int ret = QMessageBox::question(this, "确认删除", QString("确定要删除用户：%1 吗？").arg(username), QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        if (!userManager.removeUser(username.toStdString())) {
            QMessageBox::warning(this, "删除失败", "未找到该用户或删除失败。");
            return;
        }
        saveUserData();
        refreshUserTable(table);
    }
}

// 导入书籍
void Widget::onImportBooks(QTableWidget *table)
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择书籍文件", "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "打开失败", "无法打开文件！");
        return;
    }

    // 统计总行数
    int totalLines = 0;
    while (!file.atEnd()) {
        file.readLine();
        ++totalLines;
    }
    //重置文件指针
    file.seek(0);

    QProgressDialog *progress = new QProgressDialog("正在导入书籍...", "取消", 0, totalLines, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);
    progress->setValue(0);

    // 创建进度条
    QFutureWatcher<int> *watcher = new QFutureWatcher<int>(this);
    connect(progress, &QProgressDialog::canceled, watcher, &QFutureWatcher<int>::cancel);

    auto importTask = [fileName, totalLines, this, progress, watcher]() -> int {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return 0;
        // 创建文本流
        QTextStream in(&file);
        int count = 0;
        // 计算进度步长
        //qmax 返回a和b中的较大值
        int progressStep = qMax(1, totalLines / 1000);
        while (!in.atEnd()) {
            if (watcher->isCanceled()) break;
            // 读取一行并去除空格
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;
            QString jsonStr = line;
            // 替换单引号为双引号
            jsonStr.replace("'", "\"");
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &err);
            if (err.error != QJsonParseError::NoError || !doc.isObject()) continue;
            QJsonObject obj = doc.object();
            QString isbn = obj.value("ISBN").toString();
            QString title = obj.value("书名").toString();
            QString author = obj.value("作者").toString();
            QString publisher = obj.value("出版社").toString();
            int year = obj.value("出版年限").toInt();
            if (isbn.isEmpty() || title.isEmpty() || author.isEmpty() || publisher.isEmpty() || year == 0) continue;
            try {
                Book book(isbn.toStdString(), title.toStdString(), author.toStdString(), publisher.toStdString(), year);
                bookManager.addBookNoRebuild(book);
            } catch (const std::exception &e) {
                // 重复或异常跳过
                qDebug() << "导入书籍失败:" << e.what();
            }
            ++count;
            //判断是否需要更新进度条
            if (count % progressStep == 0) {
                // 使用Qt的信号槽机制更新进度条
                QMetaObject::invokeMethod(progress, "setValue", Qt::QueuedConnection, Q_ARG(int, count));
            }
        }
        // 重建图书哈希表
        bookManager.rebuildBookHashTable();
        // 更新进度条
        QMetaObject::invokeMethod(progress, "setValue", Qt::QueuedConnection, Q_ARG(int, totalLines));
        return count;
    };

    // 连接信号槽
    // 导入完成后立即保存图书数据到文件，确保数据同步
    connect(watcher, &QFutureWatcher<int>::finished, this,[=]{
        progress->close();
        QString bookDataPath = QCoreApplication::applicationDirPath() + "/books.json";
        if (bookManager.saveToFile(bookDataPath)) {
            qDebug() << "导入后图书数据已保存到文件:" << bookDataPath;
        } else {
            qDebug() << "导入后图书数据保存失败:" << bookDataPath;
        }
        // 刷新图书表
        refreshBookTable(table);
        QMessageBox::information(this, "导入完成", QString("成功导入%1条书籍信息。\n(如有重复或异常已自动跳过)").arg(watcher->result()));
        watcher->deleteLater();
        progress->deleteLater();
    });

    watcher->setFuture(QtConcurrent::run(importTask));
    progress->exec();
}

static const QString BUTTON_STYLE = R"(
    QPushButton {
        background-color: #4CAF50;
        color: white;
        border: none;
        border-radius: 4px;
        padding: 6px 12px;
        font-size: 12px;
        font-weight: bold;
        min-width: 60px;
        min-height: 24px;
    }
    QPushButton:hover { background-color: #45a049; }
    QPushButton:pressed { background-color: #3d8b40; }
    QPushButton:disabled { background-color: #cccccc; color: #666666; }
)";

//刷新借阅图书界面，支持分页功能
void Widget::refreshBorrowPageTable(QTableWidget *table, int pageNum, int pageSize)
{
    table->setUpdatesEnabled(false);      // 禁用刷新，提升性能
    table->blockSignals(true);            // 禁用信号，防止多余触发

    // 应用当前排序状态
    MyVector<Book> sortedBooks;
    if (borrowPageTableSortState.lastSortedColumn >= 0 && borrowPageTableSortState.lastSortedColumn < 5) {
        // 有排序状态，应用排序
        SortBy sortBy;
        switch (borrowPageTableSortState.lastSortedColumn) {
        case 0: sortBy = SortBy::ISBN; break;       // ISBN列
        case 1: sortBy = SortBy::TITLE; break;      // 书名列
        case 2: sortBy = SortBy::AUTHOR; break;     // 作者列
        case 3: sortBy = SortBy::PUBLISHER; break;  // 出版社列
        case 4: sortBy = SortBy::YEAR; break;       // 出版年份列
        default: sortBy = SortBy::TITLE; break;
        }
        SortOrder order = borrowPageTableSortState.ascending ? SortOrder::ASCENDING : SortOrder::DESCENDING;
        sortedBooks = bookManager.getSortedBooks(sortBy, order);
    } else {
        // 没有排序状态，使用原始顺序
        sortedBooks = bookManager.getAllBooks();
    }

    //分页
    MyVector<Book> pagedResult;
    int totalItems = static_cast<int>(sortedBooks.getSize()); //总数
    int startIndex = (pageNum - 1) * pageSize;
    int endIndex = std::min(startIndex + pageSize, totalItems);

    for (int i = startIndex; i < endIndex; ++i) {
        pagedResult.add(sortedBooks[i]);
    }
    int rowCount = static_cast<int>(pagedResult.getSize());
    table->clearContents();
    table->setRowCount(rowCount);         // 一次性设置行数
    for (int i = 0; i < rowCount; ++i) {
        const Book &book = pagedResult[i];
        QString isbn = QString::fromStdString(book.getIsbn());
        QString title = QString::fromStdString(book.getTitle());
        QString author = QString::fromStdString(book.getAuthor());
        QString publisher = QString::fromStdString(book.getPublisher());
        QString year = QString::number(book.getPublishYear());
        table->setItem(i, 0, new QTableWidgetItem(isbn));
        table->setItem(i, 1, new QTableWidgetItem(title));
        table->setItem(i, 2, new QTableWidgetItem(author));
        table->setItem(i, 3, new QTableWidgetItem(publisher));
        table->setItem(i, 4, new QTableWidgetItem(year));
        // 借阅按钮
        QPushButton *btn = qobject_cast<QPushButton*>(table->cellWidget(i, 5));
        if (!btn) {
            btn = new QPushButton("借阅");
            btn->setStyleSheet(BUTTON_STYLE);
            table->setCellWidget(i, 5, btn);
            connect(btn, &QPushButton::clicked, this, [this, isbn, title](){
                handleBorrowPageBorrowClicked(isbn, title);
            });
        }
    }

    table->blockSignals(false);           // 恢复信号
    table->setUpdatesEnabled(true);       // 恢复刷新

    updatePageInfo(pageNum,pageSize, totalItems);
}

//刷新借阅图书界面，支持分页，按字段查询排序功能
void Widget::refreshBorrowPageTable(QTableWidget *table, int fieldIndex, const QString &keyword, int pageNum, int pageSize)
{
    table->setUpdatesEnabled(false); // 1. 禁用刷新
    table->blockSignals(true);

    table->clearContents();
    //按字段查询
    MyVector<Book> result;
    QString key = keyword.trimmed();
    if (key.isEmpty()) {
        result = bookManager.getAllBooks();
    } else {
        std::string keyStr = key.toStdString();
        switch (fieldIndex) {
        case 0: {
            Book* pBook = bookManager.findBookByIsbn(keyStr);
            if (pBook) result.add(*pBook);
            break;
        }
        case 1:
            result = bookManager.findBooksByTitle(keyStr);
            break;
        case 2:
            result = bookManager.findBooksByAuthor(keyStr);
            break;
        case 3:
            result = bookManager.findBooksByPublisher(keyStr);
            break;
        case 4: {
            bool ok = false;
            int year = key.toInt(&ok);
            if (ok) result = bookManager.findBooksByYear(year);
            break;
        }
        }
    }

    // 应用当前排序状态到搜索结果
    MyVector<Book> sortedResult;
    if (borrowPageTableSortState.lastSortedColumn >= 0 && borrowPageTableSortState.lastSortedColumn < 5) {
        // 有排序状态，应用排序到搜索结果
        SortBy sortBy;
        switch (borrowPageTableSortState.lastSortedColumn) {
        case 0: sortBy = SortBy::ISBN; break;       // ISBN列
        case 1: sortBy = SortBy::TITLE; break;      // 书名列
        case 2: sortBy = SortBy::AUTHOR; break;     // 作者列
        case 3: sortBy = SortBy::PUBLISHER; break;  // 出版社列
        case 4: sortBy = SortBy::YEAR; break;       // 出版年份列
        default: sortBy = SortBy::TITLE; break;
        }
        SortOrder order = borrowPageTableSortState.ascending ? SortOrder::ASCENDING : SortOrder::DESCENDING;
        sortedResult = bookManager.sortSearchResults(result, sortBy, order);
    } else {
        // 没有排序状态，使用原始搜索结果
        sortedResult = result;
    }

    //分页展示
    MyVector<Book> pagedResult;
    int totalItems = static_cast<int>(sortedResult.getSize());
    int startIndex = (pageNum - 1) * pageSize;
    int endIndex = std::min(startIndex + pageSize, totalItems);

    for (int i = startIndex; i < endIndex; ++i) {
        pagedResult.add(sortedResult[i]);
    }

    table->setRowCount(static_cast<int>(pagedResult.getSize())); // 2. 直接设置行数

    for (size_t i = 0; i < pagedResult.getSize(); ++i) {
        const Book &book = pagedResult[i];
        QString isbn = QString::fromStdString(book.getIsbn());
        QString title = QString::fromStdString(book.getTitle());
        QString author = QString::fromStdString(book.getAuthor());
        QString publisher = QString::fromStdString(book.getPublisher());
        QString year = QString::number(book.getPublishYear());
        table->setItem(static_cast<int>(i), 0, new QTableWidgetItem(isbn));
        table->setItem(static_cast<int>(i), 1, new QTableWidgetItem(title));
        table->setItem(static_cast<int>(i), 2, new QTableWidgetItem(author));
        table->setItem(static_cast<int>(i), 3, new QTableWidgetItem(publisher));
        table->setItem(static_cast<int>(i), 4, new QTableWidgetItem(year));
        // 借阅按钮
        QPushButton *btn = qobject_cast<QPushButton*>(table->cellWidget(i, 5));
        if (!btn) {
            btn = new QPushButton("借阅");
            btn->setStyleSheet(BUTTON_STYLE);
            table->setCellWidget(i, 5, btn);
            connect(btn, &QPushButton::clicked, this, [this, isbn, title](){
                handleBorrowPageBorrowClicked(isbn, title);
            });
        }
    }

    table->blockSignals(false);
    table->setUpdatesEnabled(true); // 3. 启用刷新

    updatePageInfo(pageNum,pageSize, totalItems);
}

//借阅图书
void Widget::handleBorrowPageBorrowClicked(const QString &isbn, const QString &title)
{
    if (!isLoggedIn) {
        QMessageBox::warning(this, "未登录", "请先登录后再借阅图书。");
        return;
    }
    try {
        bool success = borrowManager->borrowBook(isbn.toStdString(), currentUser.toStdString());
        if (success) {
            // 立即保存借阅记录到文件，确保数据同步
            QString borrowDataPath = QCoreApplication::applicationDirPath() + "/borrow_records.json";
            if (borrowManager->saveToFile(borrowDataPath)) {
                qDebug() << "借阅记录已保存到文件:" << borrowDataPath;
            } else {
                qDebug() << "借阅记录保存失败:" << borrowDataPath;
            }
            QMessageBox::information(this, "借阅成功", QString("成功借阅《%1》！").arg(title));
        } else {
            QMessageBox::warning(this, "借阅失败", "借阅失败，可能已借阅或库存不足。");
        }
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "借阅失败", e.what());
    }
}

// 保存图书和借阅记录数据
void Widget::saveBookAndBorrowData()
{
    try {
        QString bookDataPath = QCoreApplication::applicationDirPath() + "/books.json";
        QString borrowDataPath = QCoreApplication::applicationDirPath() + "/borrow_records.json";
        
        // 保存图书数据
        if (bookManager.saveToFile(bookDataPath)) {
            qDebug() << "图书数据保存成功:" << bookDataPath;
        } else {
            qDebug() << "图书数据保存失败:" << bookDataPath;
        }
        
        // 保存借阅记录数据
        if (borrowManager->saveToFile(borrowDataPath)) {
            qDebug() << "借阅记录数据保存成功:" << borrowDataPath;
        } else {
            qDebug() << "借阅记录数据保存失败:" << borrowDataPath;
        }
    } catch (const std::exception &e) {
        qDebug() << "保存图书和借阅记录数据失败:" << e.what();
    }
}

// 加载图书和借阅记录数据
void Widget::loadBookAndBorrowData()
{
    try {
        QString bookDataPath = QCoreApplication::applicationDirPath() + "/books.json";
        QString borrowDataPath = QCoreApplication::applicationDirPath() + "/borrow_records.json";
        
        // 加载图书数据
        if (QFile::exists(bookDataPath)) {
            if (bookManager.loadFromFile(bookDataPath)) {
                qDebug() << "图书数据加载成功:" << bookDataPath;
            } else {
                qDebug() << "图书数据加载失败:" << bookDataPath;
            }
        } else {
            qDebug() << "图书数据文件不存在，将创建新文件:" << bookDataPath;
        }
        
        // 加载借阅记录数据
        if (QFile::exists(borrowDataPath)) {
            if (borrowManager->loadFromFile(borrowDataPath)) {
                qDebug() << "借阅记录数据加载成功:" << borrowDataPath;
            } else {
                qDebug() << "借阅记录数据加载失败:" << borrowDataPath;
            }
        } else {
            qDebug() << "借阅记录数据文件不存在，将创建新文件:" << borrowDataPath;
        }
    } catch (const std::exception &e) {
        qDebug() << "加载图书和借阅记录数据失败:" << e.what();
    }
}

// 从文件刷新借阅记录数据
void Widget::refreshBorrowDataFromFile()
{
    try {
        QString borrowDataPath = QCoreApplication::applicationDirPath() + "/borrow_records.json";
        
        if (QFile::exists(borrowDataPath)) {
            if (borrowManager->loadFromFile(borrowDataPath)) {
                qDebug() << "借阅记录数据重新加载成功:" << borrowDataPath;
            } else {
                qDebug() << "借阅记录数据重新加载失败:" << borrowDataPath;
            }
        } else {
            qDebug() << "借阅记录数据文件不存在:" << borrowDataPath;
        }
    } catch (const std::exception &e) {
        qDebug() << "重新加载借阅记录数据失败:" << e.what();
    }
}

// 更新分页信息
void Widget::updatePageInfo(int pageNum, int pageSize, int totalResults)
{
    totalBorrowPage = (totalResults + pageSize - 1) / pageSize;
    lblPageInfo->setText(QString("第 %1/%2 页").arg(pageNum).arg(totalBorrowPage));

    btnFirst->setEnabled(pageNum > 1);
    btnPrev->setEnabled(pageNum > 1);
    btnNext->setEnabled(pageNum < totalBorrowPage);
    btnLast->setEnabled(pageNum < totalBorrowPage);
}

// 表格排序槽函数实现
void Widget::onBookTableHeaderClicked(int logicalIndex)
{
    // 跳过操作列（最后一列）
    if (logicalIndex >= 5) {
        return;
    }
    
    // 确定排序方向和字段
    if (bookTableSortState.lastSortedColumn == logicalIndex) {
        bookTableSortState.ascending = !bookTableSortState.ascending;
    } else {
        bookTableSortState.lastSortedColumn = logicalIndex;
        bookTableSortState.ascending = false; // 默认降序
    }
    // 直接调用带搜索参数的刷新函数，排序集成在其中
    refreshBookTable(qobject_cast<QTableWidget*>(mainStack->widget(BOOK_PAGE)->findChild<QTableWidget*>()), bookTableLastFieldIndex, bookTableLastKeyword);
}

void Widget::onBorrowTableHeaderClicked(int logicalIndex)
{
    // 跳过操作列（最后一列）
    if (logicalIndex >= 7) {
        return;
    }
    
    // 确定排序方向和字段
    if (borrowTableSortState.lastSortedColumn == logicalIndex) {
        borrowTableSortState.ascending = !borrowTableSortState.ascending;
    } else {
        borrowTableSortState.lastSortedColumn = logicalIndex;
        borrowTableSortState.ascending = false; // 默认降序
    }
    // 直接调用带搜索参数的刷新函数，排序集成在其中
    refreshBorrowTable(qobject_cast<QTableWidget*>(mainStack->widget(BORROW_PAGE)->findChild<QTableWidget*>()), borrowTableLastFieldIndex, borrowTableLastKeyword);
}

void Widget::onBorrowPageTableHeaderClicked(int logicalIndex)
{
    // 跳过操作列（最后一列）
    if (logicalIndex >= 5) {
        return;
    }
    
    // 如果点击的是同一列，切换排序方向
    if (borrowPageTableSortState.lastSortedColumn == logicalIndex) {
        borrowPageTableSortState.ascending = !borrowPageTableSortState.ascending;
    } else {
        borrowPageTableSortState.lastSortedColumn = logicalIndex;
        borrowPageTableSortState.ascending = false; // 默认降序
    }
    
    // 使用refreshBorrowPageTable方法刷新表格，它会自动应用当前排序状态
    QTableWidget* borrowPageTable = qobject_cast<QTableWidget*>(mainStack->widget(BORROW_BOOK_PAGE)->findChild<QTableWidget*>());
    if (borrowPageTable) {
        // 获取当前的搜索状态
        QComboBox* fieldCombo = qobject_cast<QComboBox*>(mainStack->widget(BORROW_BOOK_PAGE)->findChild<QComboBox*>());
        QLineEdit* searchEdit = qobject_cast<QLineEdit*>(mainStack->widget(BORROW_BOOK_PAGE)->findChild<QLineEdit*>());
        
        int fieldIndex = fieldCombo ? fieldCombo->currentIndex() : 0;
        QString keyword = searchEdit ? searchEdit->text() : "";
        int pageSize = cmbPageSize->currentText().toInt();
        
        refreshBorrowPageTable(borrowPageTable, fieldIndex, keyword, currentBorrowPage, pageSize);
    }
}

void Widget::saveAllData() {
    // 假设有成员 borrowManager
    borrowManager->saveToFile("borrow_records.json");
    borrowManager->saveWaitingQueues("waiting_queues.json");
    // 可扩展：保存用户、图书等
}

void Widget::loadAllData() {
    borrowManager->loadFromFile("borrow_records.json");
    borrowManager->loadWaitingQueues("waiting_queues.json");
    // 可扩展：加载用户、图书等
}

void Widget::setupSearchHistoryPopup() {
    historyModel = new QStandardItemModel(this);
    historyView = new QListView(this);
    historyView->setModel(historyModel);
    historyView->setSelectionMode(QAbstractItemView::SingleSelection);
    historyView->setFocusPolicy(Qt::NoFocus); // 禁止 QListView 获取焦点

    historyPopup = new QWidget(this);
    QVBoxLayout* popupLayout = new QVBoxLayout(historyPopup);
    popupLayout->setContentsMargins(0, 0, 0, 0);
    popupLayout->addWidget(historyView);
    historyPopup->setStyleSheet("background:white; border:1px solid #ccc;");
    historyPopup->setFixedHeight(150); // 设置固定高度
    QPoint pos = borrowPage_searchEdit->mapToGlobal(QPoint(0, borrowPage_searchEdit->height()));
    historyPopup->move(pos.x(), pos.y());
    historyPopup->resize(borrowPage_searchEdit->width(), 150);

    connect(historyView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &Widget::onHistoryItemSelected);

    MyStack<QString> temp = searchHistory;
    while(!temp.empty()){
        QStandardItem* modelItem = new QStandardItem(temp.top());
        historyModel->appendRow(modelItem);
        temp.pop();
    }

}

void Widget::showSearchHistory() {
    if (historyModel->rowCount() == 0) {
        hideSearchHistory();
        return;
    }

    // 计算弹窗位置（输入框正下方）
    QPoint pos = borrowPage_searchEdit->mapToGlobal(QPoint(0, borrowPage_searchEdit->height()));
    historyPopup->move(pos.x(), pos.y());
    historyPopup->resize(borrowPage_searchEdit->width(), 150);
    historyPopup->show();

    borrowPage_searchEdit->setFocus();
    borrowPage_searchEdit->setCursorPosition(borrowPage_searchEdit->text().length());
}

void Widget::hideSearchHistory() {
    historyPopup->hide();
}

void Widget::onHistoryItemSelected(const QModelIndex& index) {
    if (index.isValid()) {
        QString selected = index.data().toString();

        // 设置文本
        borrowPage_searchEdit->setText(selected);

        hideSearchHistory();
        // 强制将焦点返回到 QLineEdit
        borrowPage_searchEdit->setFocus();
        borrowPage_searchEdit->setCursorPosition(borrowPage_searchEdit->text().length());
    }
}

//更新搜索记录
void Widget::updateSearchHistory(){
    historyModel->clear();
    MyStack<QString> temp = searchHistory;
    while(!temp.empty()){
        QStandardItem* modelItem = new QStandardItem(temp.top());
        historyModel->appendRow(modelItem);
        temp.pop();
    }
}

// 定义槽函数
void Widget::onFocusChanged(QWidget* old, QWidget* now) {
    Q_UNUSED(old);
    if (now != borrowPage_searchEdit && now != historyPopup) {
        hideSearchHistory(); // 失焦时隐藏弹窗
    }else {
        showSearchHistory();
    }
}
