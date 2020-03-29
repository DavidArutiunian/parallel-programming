# [Лабораторная работа №5](https://docs.google.com/document/d/1K39fOoM8axuKAa0DBRQdWzNhzec26daRAMFzrg86sIs/edit)

## Примитивы синхронизации данных. Критические секции.

| Версия .exe                                   | Критические секции |
| --------------------------------------------- | ------------------ |
| CriticalSection_x32.exe                       | Да                 |
| CriticalSection_x64.exe                       | Да                 |
| CriticalSections_NO_CRITICAL_SECTIONS_x32.exe | Нет                |
| CriticalSections_NO_CRITICAL_SECTIONS_x64.exe | Нет                |

### Примеры вывода

#### CriticalSection_x32.exe

```bash
$ ./CriticalSections_x32.exe
100000 // результат суммирования в цикле из 100_000 итераций первого потока
200000 // результат суммирования в цикле из 100_000 итераций второго потока
Result: 200000 // конечный результат
```

#### CriticalSections_NO_CRITICAL_SECTIONS_x32.exe

```bash
$ ./CriticalSections_NO_CRITICAL_SECTIONS_x32.exe
123515 // результат суммирования в цикле из 100_000 итераций первого потока
162050 // результат суммирования в цикле из 100_000 итераций второго потока
Result: 162050 // конечный результат
```

