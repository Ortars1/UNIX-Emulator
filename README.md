Разработать эмулятор для языка оболочки ОС. Необходимо сделать работу эмулятора как можно более похожей на сеанс shell в UNIX-подобной ОС. Эмулятор должен запускаться из реальной командной строки, а файл с 
виртуальной файловой системой не нужно распаковывать у пользователя. 
Эмулятор принимает образ **виртуальной файловой системы** в виде файла формата 
**zip.** Эмулятор должен работать в режиме **CLI**. 
**Конфигурационный файл** имеет формат **toml** и содержит:

• Имя компьютера для показа в приглашении к вводу.

• Путь к архиву виртуальной файловой системы. 

• Путь к лог-файлу.

**Лог-файл** имеет формат **csv** и содержит все действия во время последнего 
сеанса работы с эмулятором. 
Необходимо поддержать в эмуляторе команды **ls, cd и exit**, а также 
следующие команды: 

**1. rmdir.** 

**2. uniq.**

**ЗАПУСК**

Для работы эмулятора необходима zlib-ngd2.dll в папке с исполняемым файлом Emulator.exe, конфигурационный файл config.toml, файл для логирования session_log.csv и виртуальная файловая система virtual_fs.zip, которая разворачивается во временной папке пользователя.

Перейдите в папку с эмулятором с помощью cd и выполните команду: **Emulator.exe <имя конфигурационного файла>** (если ничего не меняли то **config.toml**).
