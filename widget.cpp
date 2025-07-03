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

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , borrowManager(nullptr)
    , permissionManager(nullptr)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    // setAttribute(Qt::WA_TranslucentBackground);
    borrowManager = new BorrowManager(&bookManager, &userManager);
    permissionManager = new PermissionManager(&userManager);
    setupCustomUi();
}

Widget::~Widget()
{
    delete borrowManager;
    delete permissionManager;
    delete ui;
}

void Widget::setupCustomUi()
{
    // 右上角窗口控制按钮
    QWidget *titleBar = new QWidget(this);
    titleBar->setFixedHeight(36);
    titleBar->setStyleSheet("background: transparent;");
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(0, 0, 8, 0);
    titleLayout->addStretch();
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
    QPushButton *btnBook = new QPushButton("图书管理", navBar);
    QPushButton *btnBorrow = new QPushButton("借阅管理", navBar);
    QPushButton *btnUser = new QPushButton("用户管理", navBar);
    for (auto btn : {btnBook, btnBorrow, btnUser}) {
        btn->setFixedHeight(40);
        btn->setStyleSheet("QPushButton{color:white;background:transparent;border:none;font-size:16px;} QPushButton:hover{background:#444;}");
        navLayout->addWidget(btn);
    }
    navLayout->addStretch();
    // 主内容区
    QStackedWidget *stack = new QStackedWidget(this);
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
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnEdit);
    btnLayout->addWidget(btnDelete);
    btnLayout->addStretch();
    bookLayout->addLayout(btnLayout);
    bookLayout->addWidget(bookTable);
    stack->addWidget(bookPage); // 图书管理页
    // 借阅管理页
    QWidget *borrowPage = new QWidget(this);
    QVBoxLayout *borrowLayout = new QVBoxLayout(borrowPage);
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
    stack->addWidget(borrowPage); // 借阅管理页
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
    userBtnLayout->addWidget(btnAddUser);
    userBtnLayout->addWidget(btnEditUser);
    userBtnLayout->addWidget(btnDeleteUser);
    userBtnLayout->addStretch();
    userLayout->addLayout(userBtnLayout);
    userLayout->addWidget(userTable);
    stack->addWidget(userPage); // 用户管理页
    // 主内容区背景色
    stack->setStyleSheet("background:#f5f6fa; border-top-right-radius:16px; border-bottom-right-radius:16px;");
    // 各页面背景色
    bookPage->setStyleSheet("background:#f5f6fa;");
    borrowPage->setStyleSheet("background:#f5f6fa;");
    userPage->setStyleSheet("background:#f5f6fa;");
    // 总体布局
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(navBar);
    mainLayout->addWidget(stack);
    // 嵌入到外层垂直布局
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->setSpacing(0);
    outerLayout->addWidget(titleBar);
    outerLayout->addLayout(mainLayout);
    setLayout(outerLayout);
    // 信号槽
    connect(btnBook, &QPushButton::clicked, [this, stack]{ stack->setCurrentIndex(0); });
    connect(btnBorrow, &QPushButton::clicked, [this, stack]{ stack->setCurrentIndex(1); });
    connect(btnUser, &QPushButton::clicked, [this, stack]{ stack->setCurrentIndex(2); });
    // 图书管理功能信号槽
    connect(btnAdd, &QPushButton::clicked, [this, bookTable]{ onAddBook(bookTable); });
    connect(btnEdit, &QPushButton::clicked, [this, bookTable]{ onEditBook(bookTable); });
    connect(btnDelete, &QPushButton::clicked, [this, bookTable]{ onDeleteBook(bookTable); });
    // 借阅管理功能信号槽
    connect(btnBorrowBook, &QPushButton::clicked, [this, borrowTable]{ onBorrowBook(borrowTable); });
    connect(btnReturnBook, &QPushButton::clicked, [this, borrowTable]{ onReturnBook(borrowTable); });
    connect(btnRenewBook, &QPushButton::clicked, [this, borrowTable]{ onRenewBook(borrowTable); });
    // 用户管理功能信号槽
    connect(btnAddUser, &QPushButton::clicked, [this, userTable]{ onAddUser(userTable); });
    connect(btnEditUser, &QPushButton::clicked, [this, userTable]{ onEditUser(userTable); });
    connect(btnDeleteUser, &QPushButton::clicked, [this, userTable]{ onDeleteUser(userTable); });
    // 信号槽
    connect(btnMin, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(btnMax, &QPushButton::clicked, [this, btnMax]{
        if (isMaximized()) {
            showNormal();
            btnMax->setText("□");
        } else {
            showMaximized();
            btnMax->setText("❐");
        }
    });
    connect(btnClose, &QPushButton::clicked, this, &QWidget::close);

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
    btnBorrowBook->setStyleSheet(btnStyle);
    btnReturnBook->setStyleSheet(btnStyle);
    btnRenewBook->setStyleSheet(btnStyle);
    btnAddUser->setStyleSheet(btnStyle);
    btnEditUser->setStyleSheet(btnStyle);
    btnDeleteUser->setStyleSheet(btnStyle);

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
    QTableWidget::item:selected {
        background: #b2bec3;
        color: #222;
    }
    )";
    bookTable->setStyleSheet(tableStyle);
    borrowTable->setStyleSheet(tableStyle);
    userTable->setStyleSheet(tableStyle);

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

    // 初始刷新
    refreshBookTable(bookTable);
    refreshBorrowTable(borrowTable);
    refreshUserTable(userTable);
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
            Book updatedBook(isbn.toStdString(), title.toStdString(), author.toStdString(), publisher.toStdString(), year);
            if (!bookManager.updateBook(oldIsbn.toStdString(), updatedBook)) {
                QMessageBox::warning(this, "修改失败", "未找到原图书或ISBN已更改为已存在的图书。");
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
    QString title = table->item(row, 1)->text();
    int ret = QMessageBox::question(this, "确认删除", QString("确定要删除图书：%1 (ISBN: %2) 吗？").arg(title, isbn), QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        try {
            if (!bookManager.removeBook(isbn.toStdString())) {
                QMessageBox::warning(this, "删除失败", "未找到该图书或删除失败。");
            }
            refreshBookTable(table);
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "删除失败", e.what());
        }
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
    auto records = borrowManager->getOverdueRecords(); // 可改为getUserBorrowRecords等
    for (size_t i = 0; i < records.getSize(); ++i) {
        table->insertRow(i);
        const auto &rec = records[i];
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(rec.getRecordId())));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(rec.getBookIsbn())));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(rec.getUsername())));
        table->setItem(i, 3, new QTableWidgetItem(QDateTime::fromSecsSinceEpoch(rec.getBorrowDate()).toString("yyyy-MM-dd")));
        table->setItem(i, 4, new QTableWidgetItem(QDateTime::fromSecsSinceEpoch(rec.getDueDate()).toString("yyyy-MM-dd")));
        table->setItem(i, 5, new QTableWidgetItem(rec.getIsReturned() ? QDateTime::fromSecsSinceEpoch(rec.getReturnDate()).toString("yyyy-MM-dd") : ""));
        table->setItem(i, 6, new QTableWidgetItem(rec.getIsReturned() ? "已归还" : (rec.getDueDate() < QDateTime::currentSecsSinceEpoch() ? "逾期" : "借阅中")));
    }
}

void Widget::onBorrowBook(QTableWidget *table)
{
    QDialog dialog(this);
    dialog.setWindowTitle("借书");
    QFormLayout form(&dialog);
    QLineEdit *isbnEdit = new QLineEdit(&dialog);
    QLineEdit *userEdit = new QLineEdit(&dialog);
    form.addRow("ISBN:", isbnEdit);
    form.addRow("用户名:", userEdit);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted) {
        QString isbn = isbnEdit->text().trimmed();
        QString username = userEdit->text().trimmed();
        if (isbn.isEmpty() || username.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "ISBN和用户名不能为空！");
            return;
        }
        bool ok = borrowManager->borrowBook(isbn.toStdString(), username.toStdString());
        if (ok) {
            QMessageBox::information(this, "借书成功", "借书操作成功！");
        } else {
            QMessageBox::warning(this, "借书失败", "借书失败，可能是用户/图书不存在或已借阅未归还。");
        }
        refreshBorrowTable(table);
    }
}

void Widget::onReturnBook(QTableWidget *table)
{
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "未选择", "请先选择要归还的借阅记录行。");
        return;
    }
    QString isbn = table->item(row, 1)->text();
    QString username = table->item(row, 2)->text();
    QString status = table->item(row, 6)->text();
    if (status == "已归还") {
        QMessageBox::information(this, "已归还", "该借阅记录已归还，无需重复操作。");
        return;
    }
    int ret = QMessageBox::question(this, "确认还书", QString("确定要归还图书 (ISBN: %1) 用户: %2 吗？").arg(isbn, username), QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        bool ok = borrowManager->returnBook(isbn.toStdString(), username.toStdString());
        if (ok) {
            QMessageBox::information(this, "还书成功", "还书操作成功！");
        } else {
            QMessageBox::warning(this, "还书失败", "还书失败，未找到对应借阅记录或已归还。");
        }
        refreshBorrowTable(table);
    }
}

void Widget::onRenewBook(QTableWidget *table)
{
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "未选择", "请先选择要续借的借阅记录行。");
        return;
    }
    QString isbn = table->item(row, 1)->text();
    QString username = table->item(row, 2)->text();
    QString status = table->item(row, 6)->text();
    if (status == "已归还") {
        QMessageBox::information(this, "已归还", "该借阅记录已归还，无法续借。");
        return;
    }
    int ret = QMessageBox::question(this, "确认续借", QString("确定要续借图书 (ISBN: %1) 用户: %2 吗？").arg(isbn, username), QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        bool ok = borrowManager->renewBook(isbn.toStdString(), username.toStdString());
        if (ok) {
            QMessageBox::information(this, "续借成功", "续借操作成功！");
        } else {
            QMessageBox::warning(this, "续借失败", "续借失败，未找到对应借阅记录或已归还。");
        }
        refreshBorrowTable(table);
    }
}

void Widget::refreshUserTable(QTableWidget *table)
{
    table->setRowCount(0);
    const auto &users = userManager;
    // 由于UserManager没有直接暴露用户列表，假设有getAllUsers()，否则可遍历fuzzyFindUsers("")
    MyVector<User> allUsers = users.fuzzyFindUsers("");
    for (size_t i = 0; i < allUsers.getSize(); ++i) {
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(allUsers[i].username)));
        table->setItem(i, 1, new QTableWidgetItem(allUsers[i].role == ADMIN ? "管理员" : "普通用户"));
    }
}

void Widget::onAddUser(QTableWidget *table)
{
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
            refreshUserTable(table);
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "添加失败", e.what());
        }
    }
}

void Widget::onEditUser(QTableWidget *table)
{
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
            refreshUserTable(table);
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "修改失败", e.what());
        }
    }
}

void Widget::onDeleteUser(QTableWidget *table)
{
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "未选择", "请先选择要删除的用户行。");
        return;
    }
    QString username = table->item(row, 0)->text();
    int ret = QMessageBox::question(this, "确认删除", QString("确定要删除用户：%1 吗？").arg(username), QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        if (!userManager.removeUser(username.toStdString())) {
            QMessageBox::warning(this, "删除失败", "未找到该用户或删除失败。");
            return;
        }
        refreshUserTable(table);
    }
}

