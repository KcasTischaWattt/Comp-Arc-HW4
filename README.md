# Comp-Arc-HW4
<br> Работа выполнена Татауровым Матвеем, БПИ 217 на предполагаемую оценку 8.
<br> Вариант 37.
## Как пользоваться программой
### Вариант 1 - запуск без командной строки
<br> Выбираем тип ввода - консольный, из файла или рандомный
<br> - Если выбрали консольный, то формат следующий:
```
Сначала вводится количество регулярных посетителей
Далее - количество различных книг
А далее в N строках вас попросят ввести количество экземляров i-ой книги
```
<br> - В случае с файлом, он должен быть подобного формата:
```
5 - количество регулярных посетителей
3 - количество различных книг
1 - в следующих N строках количество экземляров i-ой книги
1
1
```
<br> - В случае с рандомного ввода надо ввести лишь единственное число - сид генерации
### Вариант 2 - запуск с использованием командной строки
<br> Я работал на винде в CLion, так что аргументы указывал в параметрах запуска
<br> - Вариант 1(Для ввода из файла):
```
input.txt output.txt
```
<br> - Вариант 2(Для рандомного ввода):
```
seed
```
## Отчёт на 4 балла:
### 1. Приведено решение задачи на C++: 
https://github.com/KcasTischaWattt/Comp-Arc-HW4/blob/main/untitled1/main.cpp
### 2. Модель параллельных вычислений:
> Здесь потоки описывают посетителей библиотеки, которые конкурируют за ресурсы - книги. Поток запрашивает информацию о состоянии какой-то ячейки памяти и ждёт ответа. Ответ и определяет дальнейшие действия потока - либо он "забирает" книгу себе, либо кладёт её в свой условный "список желаемого", после чего достигает барьера и ждёт, пока барьера достигнут другие. Далее начинается новый день, в начале которого поток проверяет свои ресурсы, и определяет, надо или не надо ему сдавать книгу. После чего запрашивает информацию о каждой книги из списка желаемого, и забирает те, что появились. И так далее, до конца месяца, после чего программа завершается.
### 3. Входные данные программы:
> Все вариативные диапазоны - в пределах разумного) Ограничения интов никто не отменял)
> 1. Количество посетителей-потоков, книг и экземпляров каждой книги задаётся вручную
> 2. Количество книг, которые хочет взять посетитель, а также срок, на который он хочет взять эти книги - случайно генерируемые значения

## Отчёт на 5 баллов:
### Сценарий(частично был описан выше, но всё же):
> 1. Запуск программы. В библиотеку заходят первые посетители. Им пока нечего сдавать, они просто выбирают книги
> 2. Посетители выбрали книги, часть из них успела взять свои, часть - нет. Последние добавили эти книги в список желаемого
> 3. Посетители расходятся, начинается новый день
> 4. Часть посетителей возвращает свои книги. Далее все посетители запрашивают информацию о книгах из списка желаемого. Часть успевает взять, а часть продолжает ждать появления этих книг.
> 5. Посетители снова выбирают книги - пункты 2-4 повторяются много раз.
> 6. Месяц заканчивается, программа завершает работу
## Отчёт на 6 баллов:
### 1.  Реализован ввод данных из командной строки(см. выше);
### 2.  Обобщённый алгоритм:
> 1. Запуск программы. Ввод данных пользователем. Инициализация количества пользователей(М), количества книг(N книг по Ki экземпляров каждая)
> 2. Инициализация барьера. Создание и запуск M потоков. Далее переходим к рссмотрению алгоритма самого потока
> 3. В поток передаётся экземпляр посетителя с уникальным id. Сперва идёт проверка map книг посетителя на наличие книг, которые надо сдать(У которых значение == 0). В случае обнаружениея подобных они удаляются из мапы посетителя и добавляются в общий вектор книг.
> 4. Далее идёт проверка на то, появились ли в библиотеке книги, которые посетитель хотел взять - добавил в вектор wantedBooks. Если подобные книги обнаруживаются, то они удаляются из общего вектора книг и добавляются пользователю.
> 5. Последнее значимое действие потока перед барьером - выбор книг. Сначала генерируется случайное число, и если оно подходит(вероятность 1 к 4), то пользователь приступает к выбору от 1 до 3 книг. Это число, как и книги, выбирается случайным образом. Все книги поочерёдно добавляются в сет книг - две одинаквые взять не выйдет. После этого идёт проверка - если ли каждая из книг на полках библиотеки или нет. Если есть - то посетитель её берёт. Если нет - то пользователь проверяет, нет ли её в списке желаемого, и только потом добавляет туда.
> 6. Поток доходит до барьера. Если это последний поток - увеличивается счётчик дней. Если счётчик дней дошёл до MAX_DAYS - то поток заканчивает работу
> 7. Потоки джоинятся. Происходит запись в файл. Программа завершает работу.
## Отчёт на 7 баллов:
### 1.  В программу добавлены ввод данных из файла и вывод результатов в файл. Результаты дублируются на экран - сделано;
### 2.  Приведены входные и выходные файлы с различными результатами выполнения программы 
https://github.com/KcasTischaWattt/Comp-Arc-HW4/tree/main/test_files
## Отчёт на 8 баллов:
### 1.  В программу добавлен генератор случайных данных в допустимых диапазонах - сделано;
### 2.  Приведены итоги тестов с различными результатами выполнения программы 
https://github.com/KcasTischaWattt/Comp-Arc-HW4/tree/main/test_files
