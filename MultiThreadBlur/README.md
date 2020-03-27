# [Лабораторная работа №2](https://docs.google.com/document/d/1bKzU4040KnjLZpV7aoj7n_0Ehrx8TJiWsrk2RRjCo3s/edit)

## Параллельная обработка непересекающихся данных

#### Отчёт

https://docs.google.com/spreadsheets/d/1ZxLhPMk5K-T6OWGnKVMh8ZQWzQffaMhx0PNPPA8bjI0/edit?usp=sharing

------

# [Лабораторная работа №4](https://docs.google.com/document/d/1kCOQdaPe1Ulq0fYE9KniMat1-Wv8IB47ZowNGwIzwMk/edit)

#### Отчёт

https://1drv.ms/x/s!AiyxKhcZ2MhC0zQv_LcLXLC_1In9?e=u0TCfA

------

# Инструкции

#### Запуск test.bat

```bash
$ test.bat [кол-во ядер процессора]
```

В скрипте зашит путь к файлу [FLAG_B24.BMP](Binaries/Images/FLAG_B24.BMP)

Выходной файл будет находится в той же папке, что и [FLAG_B24.BMP](Binaries/Images/FLAG_B24.BMP) с названием `RESULT.BMP`

На выходе получится файл `result.txt` с таймингами процессов в миллисекундах

#### Запуск MultiThreadBlur_x(32|64).exe

```bash
$ ./MultiThreadBlur_x(32|64).exe \
	<путь к исходному файлу> \
    <путь к выходному файлу> \
    <количество потоков> \
    <количество ядер процессора> \
    <радиус размытия> \
    <путь к папке логирования (по умлочанию ./Logs/)> \
    <приоритеты потоков через запятую (например -1,-1,0,0,1,1...)>
```

##### Примечание!

Разбор аргументов происходит по индексу, соответственно, чтобы указать необходимый аргумент нужно так-же указать все аргументы до необходимого

#### Таблица поддерживаемых приоритетов потоков

Используется `Win32` метод [SetThreadPriority](https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-setthreadpriority), принимающий следующие значения:

| Приоритет | Значение     |
| --------- | ------------ |
| -1        | BELOW_NORMAL |
| 0         | NORMAL       |
| 1         | ABOVE_NORMAL |

#### Известные ограничения

Количество потоков не может быть установлено больше чем `<высота картинки> / <высота секции обработки потоком>`, где`<высота секции обработки потоком>` это максимальное значение между `<высотка картинки> / <количество потоков>` и `2 (px)` 

*Пример:*

Для картинки высотой `124px` максимальное количество потоков будет **62**

#### Библиотеки и API

- [EasyBMP](http://easybmp.sourceforge.net/)
- [WinAPI](https://docs.microsoft.com/en-us/windows/win32/)
