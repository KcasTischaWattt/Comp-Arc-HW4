#include <map>
#include <set>

// Класс посетитель
class Visitor {
public:
    // Конструктор с параметром - айди
    explicit Visitor(int id) {
        this->id = id;
    }
    // Сет желаемых книг
    std::set<int> wantedBooks;
    // Мапа книг - ключ - номер книги, значение - сколько времени осталось до сдачи
    std::map<int, int> books;
    // айди пользователя
    int id;
};