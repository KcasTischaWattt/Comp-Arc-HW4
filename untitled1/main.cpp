#include <iostream>
#include <vector>
#include "Visitor.cpp"
#include <pthread.h>
#include <fstream>
#include <set>

// Мьютексы
pthread_mutex_t mutex;
pthread_mutex_t log_mutex;
// Барьер
pthread_barrier_t barrier;
// Число книг
int totalBooks;
// Вектор книг - количество каждой из книг
std::vector<int> books;
// Строка - результат работы программы, которую потом выведем в файл
std::string res_str;
// Вектор потоков
std::vector<pthread_t> threads;
// Вывод программы - изначально я думал сделать вывод либо в консоль, лио в файл, но потом отказался, так что это теперь - рудимент
std::ostream &out = std::cout;
// Общее количество дней
int totalDays = 1;
// Максимальное число дней
#define MAX_DAYS 30

// Функция для потокобезопасного вывода на печать
void log(const std::string &str) {
    pthread_mutex_lock(&log_mutex);
    res_str += "Day " + std::to_string(totalDays) + ": " + str + "\n";
    out << "DAY " << totalDays << ": " << str << "\n";
    out.flush();
    pthread_mutex_unlock(&log_mutex);
}

// Функция посетителя библиотеки, которая навешивается на поток
void *visitorAction(void *v) {
    // Задаём сид, отталкиваясь от текущего времени
    srand(clock());
    // Создаём экземпляр визитора из переданного нам параметра
    Visitor visitor = *(Visitor *) v;
    // Временная строка для вывода на консоль
    std::string str;
    // Сам цикл логики
    while (true) {
        // Проверка на то, есть ли у пользователя не сданные книги
        for (auto itr = visitor.books.begin(); itr != visitor.books.end();) {
            // Если время у одной их книг == 0, то пора её вернуть
            if (!--(itr->second)) {
                log("Visitor " + std::to_string(visitor.id) + " returns book number "
                      + std::to_string(itr->first));
                books[itr->first]++;
                itr = visitor.books.erase(itr);
            } else {
                itr++;
            }
        }
        // Проверка на то, появились ли в библиотеке книги, которые пользователь хотел взять в прошлые разы
        if (!visitor.wantedBooks.empty()) {
            for (auto it = visitor.wantedBooks.begin(); it != visitor.wantedBooks.end();) {
                int b = *it;
                // Начало критической секции - чтобы к библиотекарю обращался только один человек, и не происходила ситуация, когда книги ушли в минус
                pthread_mutex_lock(&mutex);
                // Если книга появилась - берём её
                if (books[b]) {
                    books[b]--;
                    // Выбираем срок, на который хотим взять книгу
                    int time = 1 + std::rand() % 14;
                    visitor.books[b] = time;
                    log("Visitor " + std::to_string(visitor.id) + " finally takes book number "
                          + std::to_string(b) + " for " + std::to_string(time) + " days");
                    it = visitor.wantedBooks.erase(it);
                } else {
                    it++;
                }
                // Конец критической секции
                pthread_mutex_unlock(&mutex);
            }
        }
        // Сама секция, где берутся книги
        // Посетитель берёт книги с вероятностью 1/4
        if (std::rand() % 4 + 1 == 4) {
            // Количество книг
            int amountOfBooks = std::rand() % 3 + 1;
            // Сет книг, которые хоти взять - две одинаковые взять нельзя
            std::set<int> book_set;
            // Выбор случайных книг в сет
            for (int i = 0; i < amountOfBooks; ++i) {
                // Выбираем книгу, которую хотим взять
                int bookNumber = std::rand() % totalBooks;
                // Добавляем книгу в сет
                book_set.insert(bookNumber);
            }
            // Если выбрали книги - спрашиваем у библиотекаря, есть ли они
            if (!book_set.empty()) {
                for (auto b : book_set) {
                    // Если такой книги у посетителя ещё нет - то пытаемся взять книгу
                    if (!visitor.books.contains(b)) {
                        log("Visitor " + std::to_string(visitor.id) + " wants to take book "
                              + std::to_string(b));
                        // Начало критической секции
                        pthread_mutex_lock(&mutex);
                        // Если книга есть - берём её
                        if (books[b]) {
                            // Выбираем время, на которое хотим взять книгу
                            int time = 1 + std::rand() % 14;
                            visitor.books[b] = time;
                            books[b]--;
                            log("Visitor " + std::to_string(visitor.id) + " takes book number "
                                  + std::to_string(b) + " for " + std::to_string(time) +
                                  " days");
                            // Если книги нет библиотеке и списке желаемого - добавляем в список желаемых книг
                        } else if (!visitor.wantedBooks.contains(b)) {
                            // Добавляем книгу в список желаемого
                            visitor.wantedBooks.insert(b);
                            log("Visitor " + std::to_string(visitor.id) + " cant take book "
                                  + std::to_string(b) +
                                  " and have to wait");
                        }
                        // Выходим из критической секции
                        pthread_mutex_unlock(&mutex);
                    }
                }
            }
            // Очищаем сет книг
            book_set.clear();
        }
        out.flush();
        // Говорим, что поток дошёл до барьера
        int status = pthread_barrier_wait(&barrier);
        // Если это последний из запущенных потоков - то значит, все пользователи сегодня отработали
        if (status == PTHREAD_BARRIER_SERIAL_THREAD) {
            // 'Запускаем' новый день
            totalDays++;
        // Выкидываем сообщение об ошибке
        } else if (status != 0) {
            std::cout << "error wait barrier in thread " << visitor.id << "with status " << status;
            exit(-15);
        }
        // Если дошли до последнего дня - то заканчиваем.
        if (totalDays >= MAX_DAYS) {
            return nullptr;
        }
    }
}

//  Метод, из которого запукаются потоки
int start(int argc, char **argv) {
    // сид для генерации
    int seed;
    // Входной и выходной файл
    std::string input_file, output_file;
    // Вариант ввод
    int option = 0;
    // 3 аргумента -> input_file, output_file;
    if (argc == 3) {
        option = 2;
        input_file = argv[1];
        output_file = argv[2];
        // 2 аргумента -> seed;
    } else if (argc == 2) {
        option = 3;
        seed = std::stoi(argv[1]);
    }
    // Если не ввели аргументы - то просто консольный ввод
    if (!option) {
        std::cout << "Select the input mode:\n"
                     "1) Console\n"
                     "2) File\n"
                     "3) Random\n"
                     "Your choice: ";
        std::cin >> option;
    }
    // Число посетителей
    int totalVisitors;
    if (option == 1) {
        // Ручной ввод в консоль
        std::cout << "\nEnter count of visitors:";
        std::cin >> totalVisitors;
        std::cout << "\nEnter count of books:";
        std::cin >> totalBooks;
        books = std::vector<int>(totalBooks);
        // Вводим количество каждой из книг книги
        for (int i = 0; i < totalBooks; ++i) {
            std::cout << "\nEnter amount of book " << i << " : ";
            std::cin >> books[i];
        }
    } else if (option == 2) {
        if (argc != 3) {
            // Ввод с помощью файла.
            std::cout << "\nEnter the name of INPUT file:";
            std::cin >> input_file;
            std::cout << "Enter the name of OUTPUT file:";
            std::cin >> output_file;
        }
        // Поток файла ввода
        std::ifstream in(input_file);
        if (!in.is_open()) {
            std::cout << "Cannot open a file" << std::endl;
            return 1;
        }
        in >> totalVisitors >> totalBooks;
        books = std::vector<int>(totalBooks);
        for (int i = 0; i < totalBooks; ++i) {
            in >> books[i];
        }
        in.close();
    } else {
        // Случайная генерация.
        if (argc != 2) {
            std::cout << "Enter a seed for random generation:";
            std::cin >> seed;
        }
        std::srand(seed);
        totalVisitors = 1 + std::rand() % 32;
        totalBooks = 1 + std::rand() % 128;
        books = std::vector<int>(totalBooks);
        for (int i = 0; i < totalBooks; ++i) {
            books[i] = 1 + std::rand() % 32;
        }
    }
    // Инициализируем барьер
    pthread_barrier_init(&barrier, nullptr, totalVisitors);
    threads = std::vector<pthread_t>(totalVisitors);
    std::string str;
    // Вектор посетителей, которые потом будут переданы в потоки
    std::vector<Visitor> visitors;
    // Задаём им айдишники
    for (int i = 0; i < totalVisitors; ++i) {
        Visitor visitor(i);
        visitors.push_back(visitor);
    }
    // Создаём потоки
    for (int i = 0; i < totalVisitors; ++i) {
        pthread_create(&threads[i], nullptr, visitorAction, &visitors[i]);
    }
    // Джоиним потоки
    for (auto t : threads) {
        pthread_join(t, nullptr);
    }
    log("The end of the month");
    // Вывод в файл
    if (option == 2) {
        std::ofstream out_file(output_file, std::ofstream::out);
        if (!out_file.is_open()) {
            std::cout << "Cannot open a file" << std::endl;
            return 1;
        }
        out_file << res_str;
        out_file.close();
    }
    return 0;
}

int main(int argc, char **argv) {
    try {
        // Запуск программы
        int res = start(argc, argv);
        return res;
    } catch (const std::exception &exception) {
        std::cout << "\nSomething went wrong... " << std::endl;
        return 1;
    }
}