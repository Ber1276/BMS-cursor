#include "widget.h"
#include "./ui_widget.h"
#include <QMessageBox>
#include "include/BookManager.h"
#include "include/User.h"
#include "include/BorrowManager.h"
#include "include/PermissionManager.h"
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
    , borrowPage_widget(nullptr)
    , borrowPage_searchEdit(nullptr)
    , borrowPage_searchBtn(nullptr)
    , borrowPage_table(nullptr)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    
    // 初始化管理器
    borrowManager = new BorrowManager(&bookManager, &userManager);
    permissionManager = new PermissionManager(&userManager);
    
    // 加载用户数据
    loadUserData();
    
    // 设置UI
    setupCustomUi();
    
    // 默认显示图书管理页面（无需登录）
    switchToPage(BOOK_PAGE);
}

Widget::~Widget()
{
    // 保存用户数据
    saveUserData();
    
    delete borrowManager;
    delete permissionManager;
    delete ui;
}

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
    bookTable->setHorizontalHeaderLabels(headers);
    bookTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    bookTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bookTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    bookTable->setSelectionMode(QAbstractItemView::SingleSelection);
    
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
    
    QTableWidget *borrowTable = new QTableWidget(borrowPage);
    borrowTable->setColumnCount(7);
    QStringList borrowHeaders;
    borrowHeaders << "记录ID" << "ISBN" << "用户名" << "借阅日期" << "到期日期" << "归还日期" << "状态";
    borrowTable->setHorizontalHeaderLabels(borrowHeaders);
    borrowTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    borrowTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    borrowTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    borrowTable->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // 操作按钮
    QHBoxLayout *borrowBtnLayout = new QHBoxLayout();
    QPushButton *btnBorrowBook = new QPushButton("借书", borrowPage);
    QPushButton *btnReturnBook = new QPushButton("还书", borrowPage);
    QPushButton *btnRenewBook = new QPushButton("续借", borrowPage);
    borrowBtnLayout->addWidget(btnBorrowBook);
    borrowBtnLayout->addWidget(btnReturnBook);
    borrowBtnLayout->addWidget(btnRenewBook);
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
    QHBoxLayout *userSearchLayout = new QHBoxLayout();
    QLineEdit *userSearchEdit = new QLineEdit(userPage);
    userSearchEdit->setPlaceholderText("输入用户名关键字搜索");
    QPushButton *btnSearchUser = new QPushButton("搜索", userPage);
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
    borrowPage_widget = new QWidget(this);
    QVBoxLayout *borrowPage_layout = new QVBoxLayout(borrowPage_widget);

    // 搜索区
    QHBoxLayout *borrowPage_searchLayout = new QHBoxLayout();
    borrowPage_searchEdit = new QLineEdit(borrowPage_widget);
    borrowPage_searchEdit->setPlaceholderText("请输入书名、作者或ISBN搜索");
    borrowPage_searchBtn = new QPushButton("搜索", borrowPage_widget);
    borrowPage_searchLayout->addWidget(borrowPage_searchEdit);
    borrowPage_searchLayout->addWidget(borrowPage_searchBtn);
    borrowPage_layout->addLayout(borrowPage_searchLayout);

    // 表格
    borrowPage_table = new QTableWidget(borrowPage_widget);
    borrowPage_table->setColumnCount(6);
    QStringList borrowPage_headers;
    borrowPage_headers << "ISBN" << "书名" << "作者" << "出版社" << "出版年份" << "操作";
    borrowPage_table->setHorizontalHeaderLabels(borrowPage_headers);
    borrowPage_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    borrowPage_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    borrowPage_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    borrowPage_table->setSelectionMode(QAbstractItemView::SingleSelection);
    borrowPage_layout->addWidget(borrowPage_table);

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
    connect(btnBook, &QPushButton::clicked, this ,[this](){ switchToPage(BOOK_PAGE); });
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
    
    // 用户管理功能信号槽
    connect(btnAddUser, &QPushButton::clicked, this,[this, userTable]{ onAddUser(userTable); });
    connect(btnEditUser, &QPushButton::clicked, this,[this, userTable]{ onEditUser(userTable); });
    connect(btnDeleteUser, &QPushButton::clicked, this,[this, userTable]{ onDeleteUser(userTable); });
    connect(btnRefreshUser, &QPushButton::clicked, this,[this, userTable]{ refreshUserTable(userTable); });
    connect(btnSearchUser, &QPushButton::clicked, this, [=]{
        refreshUserTable(userTable, userSearchEdit->text().trimmed());
    });
    connect(userSearchEdit, &QLineEdit::returnPressed, this, [=]{
        refreshUserTable(userTable, userSearchEdit->text().trimmed());
    });
    
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
    btnAddUser->setStyleSheet(btnStyle);
    btnEditUser->setStyleSheet(btnStyle);
    btnDeleteUser->setStyleSheet(btnStyle);
    btnRefreshUser->setStyleSheet(btnStyle);
    btnSearchUser->setStyleSheet(btnStyle);

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
    
    // 更新登录状态显示
    updateLoginStatus();

    // 信号槽
    connect(borrowPage_searchBtn, &QPushButton::clicked, this, [this](){
        refreshBorrowPageTable(borrowPage_searchEdit->text().trimmed());
    });
    connect(borrowPage_searchEdit, &QLineEdit::returnPressed, this, [this](){
        refreshBorrowPageTable(borrowPage_searchEdit->text().trimmed());
    });
}

// 全局登录对话框
bool Widget::showGlobalLoginDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("用户登录");
    dialog.setFixedSize(300, 200);
    dialog.setModal(true);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    
    // 标题
    QLabel *titleLabel = new QLabel("请登录以继续操作", &dialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; margin: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // 表单
    QFormLayout *formLayout = new QFormLayout();
    QLineEdit *usernameEdit = new QLineEdit(&dialog);
    QLineEdit *passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setEchoMode(QLineEdit::Password);
    
    formLayout->addRow("用户名:", usernameEdit);
    formLayout->addRow("密码:", passwordEdit);
    mainLayout->addLayout(formLayout);
    
    // 按钮布局
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnLogin = new QPushButton("登录", &dialog);
    QPushButton *btnRegister = new QPushButton("注册", &dialog);
    QPushButton *btnCancel = new QPushButton("取消", &dialog);
    
    btnLayout->addWidget(btnLogin);
    btnLayout->addWidget(btnRegister);
    btnLayout->addWidget(btnCancel);
    mainLayout->addLayout(btnLayout);
    
    // 设置按钮样式
    QString btnStyle = R"(
        QPushButton {
            background: #4CAF50;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-size: 14px;
        }
        QPushButton:hover {
            background: #45a049;
        }
        QPushButton:pressed {
            background: #3d8b40;
        }
    )";
    
    QString cancelBtnStyle = R"(
        QPushButton {
            background: #f44336;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-size: 14px;
        }
        QPushButton:hover {
            background: #da190b;
        }
    )";
    
    btnLogin->setStyleSheet(btnStyle);
    btnRegister->setStyleSheet(btnStyle);
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
    dialog.setFixedSize(350, 250);
    dialog.setModal(true);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    
    // 标题
    QLabel *titleLabel = new QLabel("创建新用户账户", &dialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; margin: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // 表单
    QFormLayout *formLayout = new QFormLayout();
    QLineEdit *usernameEdit = new QLineEdit(&dialog);
    QLineEdit *passwordEdit = new QLineEdit(&dialog);
    QLineEdit *confirmPasswordEdit = new QLineEdit(&dialog);
    QComboBox *roleCombo = new QComboBox(&dialog);
    
    passwordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    roleCombo->addItem("普通用户", USER);
    roleCombo->addItem("管理员", ADMIN);
    
    formLayout->addRow("用户名:", usernameEdit);
    formLayout->addRow("密码:", passwordEdit);
    formLayout->addRow("确认密码:", confirmPasswordEdit);
    formLayout->addRow("角色:", roleCombo);
    mainLayout->addLayout(formLayout);
    
    // 按钮布局
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnRegister = new QPushButton("注册", &dialog);
    QPushButton *btnCancel = new QPushButton("取消", &dialog);
    
    btnLayout->addWidget(btnRegister);
    btnLayout->addWidget(btnCancel);
    mainLayout->addLayout(btnLayout);
    
    // 设置按钮样式
    QString btnStyle = R"(
        QPushButton {
            background: #2196F3;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-size: 14px;
        }
        QPushButton:hover {
            background: #1976D2;
        }
    )";
    
    QString cancelBtnStyle = R"(
        QPushButton {
            background: #f44336;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-size: 14px;
        }
        QPushButton:hover {
            background: #da190b;
        }
    )";
    
    btnRegister->setStyleSheet(btnStyle);
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
            // 用户取消登录，切换到图书管理页面
            switchToPage(BOOK_PAGE);
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
                break;
            case USER_PAGE:
                btnUser->setStyleSheet("QPushButton{color:white;background:#444;border:none;font-size:16px;} QPushButton:hover{background:#555;}");
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
    
    // 如果当前在需要权限的页面，切换到图书管理页面
    if (mainStack && mainStack->currentIndex() != BOOK_PAGE) {
        switchToPage(BOOK_PAGE);
    }
}

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
        refreshBookTable(table);
    }
}

void Widget::on_pushButton_clicked()
{
    QMessageBox::information(this, "Button Clicked", "You clicked the button!");
}

void Widget::on_navBook_clicked(){}
void Widget::on_navBorrow_clicked(){}
void Widget::on_navUser_clicked(){}
void Widget::on_navImport_clicked(){}

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
        table->setItem(i, 0, new QTableWidgetItem(QString::number(records[i].getId())));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(records[i].getIsbn())));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(records[i].getUsername())));
        table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(records[i].getBorrowDateStr())));
        table->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(records[i].getDueDateStr())));
        table->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(records[i].getReturnDateStr())));
        table->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(records[i].getStatus())));
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
            refreshBorrowTable(table);
            QMessageBox::information(this, "借书成功", "图书借阅成功！");
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "借书失败", e.what());
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
    
    // 检查权限：普通用户只能归还自己的记录，管理员可以归还所有记录
    QString recordUsername = table->item(row, 2)->text();
    if (!hasPermission(ADMIN) && recordUsername != currentUser) {
        QMessageBox::warning(this, "权限不足", "您只能归还自己的借阅记录。");
        return;
    }
    
    int recordId = table->item(row, 0)->text().toInt();
    try {
        borrowManager->returnBook(recordId);
        refreshBorrowTable(table);
        QMessageBox::information(this, "还书成功", "图书归还成功！");
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
        refreshBorrowTable(table);
        QMessageBox::information(this, "续借成功", "图书续借成功！");
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "续借失败", e.what());
    }
}

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

void Widget::onAddUser(QTableWidget *table)
{
    if (!hasPermission(ADMIN)) {
        QMessageBox::warning(this, "权限不足", "只有管理员才能添加用户。");
        return;
    }
    
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
    if (dialog.exec() == QDialog::Accepted) {
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
            userManager.addUser(user);
            saveUserData();
            refreshUserTable(table);
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "添加失败", e.what());
        }
    }
}

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
    file.seek(0);

    QProgressDialog *progress = new QProgressDialog("正在导入书籍...", "取消", 0, totalLines, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);
    progress->setValue(0);

    QFutureWatcher<int> *watcher = new QFutureWatcher<int>(this);
    connect(progress, &QProgressDialog::canceled, watcher, &QFutureWatcher<int>::cancel);

    auto importTask = [fileName, totalLines, this, progress, watcher]() -> int {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return 0;
        QTextStream in(&file);
        int count = 0;
        int progressStep = qMax(1, totalLines / 1000);
        while (!in.atEnd()) {
            if (watcher->isCanceled()) break;
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;
            QString jsonStr = line;
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
                bookManager.addBook(book);
            } catch (...) {
                // 重复或异常跳过
            }
            ++count;
            if (count % progressStep == 0) {
                QMetaObject::invokeMethod(progress, "setValue", Qt::QueuedConnection, Q_ARG(int, count));
            }
        }
        QMetaObject::invokeMethod(progress, "setValue", Qt::QueuedConnection, Q_ARG(int, totalLines));
        return count;
    };

    connect(watcher, &QFutureWatcher<int>::finished, this,[=]{
        progress->close();
        refreshBookTable(table);
        QMessageBox::information(this, "导入完成", QString("成功导入%1条书籍信息。\n(如有重复或异常已自动跳过)").arg(watcher->result()));
        watcher->deleteLater();
        progress->deleteLater();
    });

    watcher->setFuture(QtConcurrent::run(importTask));
    progress->exec();
}

void Widget::refreshBorrowPageTable(const QString &keyword)
{
    borrowPage_table->setRowCount(0);
    const auto &books = bookManager.getAllBooks();
    int row = 0;
    for (size_t i = 0; i < books.getSize(); ++i) {
        const Book &book = books[i];
        QString isbn = QString::fromStdString(book.getIsbn());
        QString title = QString::fromStdString(book.getTitle());
        QString author = QString::fromStdString(book.getAuthor());
        QString publisher = QString::fromStdString(book.getPublisher());
        QString year = QString::number(book.getPublishYear());

        // 关键字过滤
        if (!keyword.isEmpty() &&
            !isbn.contains(keyword, Qt::CaseInsensitive) &&
            !title.contains(keyword, Qt::CaseInsensitive) &&
            !author.contains(keyword, Qt::CaseInsensitive)) {
            continue;
        }

        borrowPage_table->insertRow(row);
        borrowPage_table->setItem(row, 0, new QTableWidgetItem(isbn));
        borrowPage_table->setItem(row, 1, new QTableWidgetItem(title));
        borrowPage_table->setItem(row, 2, new QTableWidgetItem(author));
        borrowPage_table->setItem(row, 3, new QTableWidgetItem(publisher));
        borrowPage_table->setItem(row, 4, new QTableWidgetItem(year));
        // 借阅按钮
        QPushButton *btn = new QPushButton("借阅");
        borrowPage_table->setCellWidget(row, 5, btn);
        connect(btn, &QPushButton::clicked, this, [this, isbn, title](){
            handleBorrowPageBorrowClicked(isbn, title);
        });
        ++row;
    }
}

void Widget::handleBorrowPageBorrowClicked(const QString &isbn, const QString &title)
{
    if (!isLoggedIn) {
        QMessageBox::warning(this, "未登录", "请先登录后再借阅图书。");
        return;
    }
    bool success = borrowManager->borrowBook(isbn.toStdString(), currentUser.toStdString());
    if (success) {
        QMessageBox::information(this, "借阅成功", QString("成功借阅《%1》！").arg(title));
    } else {
        QMessageBox::warning(this, "借阅失败", "借阅失败，可能已借阅或库存不足。");
    }
}

