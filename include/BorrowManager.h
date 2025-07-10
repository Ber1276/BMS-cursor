#ifndef BORROW_MANAGER_H
#define BORROW_MANAGER_H

#include "MyVector.h"
#include "BorrowRecord.h"
#include "BookManager.h"
#include "User.h"
#include "MyQueue.h"
#include <map>

// 前向声明
class QString;

// 借阅记录排序枚举
enum class BorrowSortBy {
    RECORD_ID,
    ISBN,
    USERNAME,
    BORROW_DATE,
    DUE_DATE,
    RETURN_DATE,
    STATUS
};

enum class BorrowSortOrder {
    ASCENDING,
    DESCENDING
};

class BorrowManager {
private:
    MyVector<BorrowRecord> records;
    BookManager* bookManager;
    UserManager* userManager;
    static const int DEFAULT_BORROW_DAYS = 30;
    // 新增：等待队列，key为isbn，value为用户名队列
    std::map<std::string, MyQueue<std::string>> waitingQueues;
    
    // 排序辅助方法
    void sortBorrowRecords(MyVector<BorrowRecord> &recordList, BorrowSortBy sortBy, BorrowSortOrder order) const;
    
public:
    BorrowManager(BookManager* bookManager, UserManager* userManager);
    bool borrowBook(const std::string& isbn, const std::string& username);
    bool returnBook(const std::string& isbn, const std::string& username);
    bool renewBook(const std::string& isbn, const std::string& username);

    // 通过ID操作的方法
    void returnBook(int recordId);
    void renewBook(int recordId);
    bool returnBookByRecordId(const std::string& recordId);
    
    MyVector<BorrowRecord> getUserBorrowRecords(const std::string& username);
    MyVector<BorrowRecord> getBookBorrowRecords(const std::string& isbn);
    MyVector<BorrowRecord> getOverdueRecords();
    const MyVector<BorrowRecord>& getAllBorrowRecords() const { return records; }
    size_t getBorrowCount(const std::string& username) const;
    size_t getOverdueCount(const std::string& username) const;

    //查找方法
    BorrowRecord *findByRecordId(MyVector<BorrowRecord> &record, const std::string& recordId);
    MyVector<BorrowRecord> findByISBN(MyVector<BorrowRecord> &record, const std::string& ISBN);
    MyVector<BorrowRecord> findByUsername(MyVector<BorrowRecord> &record, const std::string& username);
    MyVector<BorrowRecord> findByBorrowDate(MyVector<BorrowRecord> &record, const std::string& borrowDate);
    MyVector<BorrowRecord> findByDueDate(MyVector<BorrowRecord> &record, const std::string& dueDate);
    MyVector<BorrowRecord> findByStatus(MyVector<BorrowRecord> &record, const std::string& status);

    
    // 排序方法
    MyVector<BorrowRecord> getSortedBorrowRecords(BorrowSortBy sortBy, BorrowSortOrder order = BorrowSortOrder::ASCENDING) const;
    MyVector<BorrowRecord> sortSearchResults(const MyVector<BorrowRecord> &searchResults, BorrowSortBy sortBy, BorrowSortOrder order = BorrowSortOrder::ASCENDING) const;
    
    // 数据持久化方法
    bool saveToFile(const QString& filename) const;
    bool loadFromFile(const QString& filename);

    // 新增：等待队列相关
    int getWaitingCount(const std::string& isbn) const;
    bool isUserInQueue(const std::string& isbn, const std::string& username) const;
    void saveWaitingQueues(const QString& filename) const;
    bool loadWaitingQueues(const QString& filename);
};

#endif 
