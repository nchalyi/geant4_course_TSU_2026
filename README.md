Здесь содержатся материалы по курсу "Введение в Geant4"

# Установка Geant4

Geant4 можно установить разными способами:
1) Использование готового DOCKER-образа (Рекомендуем! Именно этот вариант будет использоваться на практических занятиях!)
2) Использование виртуальной машины ([например](https://extra.lp2ib.in2p3.fr/G4/download/))
3) Сборка из исходного кода (подробнее см. [официальную документацию](https://geant4-userdoc.web.cern.ch/UsersGuides/InstallationGuide/html/index.html))

Ниже предоставлена инструкция по настройке DOCKER-образа.

---

# Использование DOCKER-образа

## Windows

1. **Установите необходимые приложения:**
   - [Docker Desktop](https://www.docker.com/products/docker-desktop/) — основное приложение для работы с контейнерами.
   - Xserver (графический сервер), например [VcXsrv](https://vcxsrv.com/).

2. **Настройте Docker Desktop:**
   - Запустите Docker Desktop.
   - При первом запуске Docker предложит установить WSL 2 (Windows Subsystem for Linux), если он ещё не установлен. Согласитесь и следуйте инструкциям.
   - Убедитесь, что в настройках Docker Desktop включена интеграция с WSL 2 (Settings → Resources → WSL Integration).

3. **Запустите Xserver:**
   - Запустите установленный Xserver (например, VcXsrv).
   - Для VcXsrv: при запуске выберите "One large window" или "Multiple windows" -> Далее -> "Start no client" -> Далее -> Готово.
   - Убедитесь, что Xserver работает и не блокируется брандмауэром (должен появится значок X в Области уведомлений в правом нижнем углу).

4. **Откройте терминал или PowerShell** и загрузите этот Docker-образ с Geant4:
   ```bash
   docker pull jeffersonlab/geant4:g4v11.3.2-almalinux94
   ```
   Также этот образ можно найти в приложении Docker Desktop.

5. **Запустите контейнер:**
   ```bash
   docker run -it -e DISPLAY=host.docker.internal:0 -v C:\Users\YourName\your-directory\:/workspace jeffersonlab/geant4:g4v11.3.2-almalinux94 /bin/bash
   ```
   - Вместо `C:\Users\YourName\your-directory\` укажите путь к вашей рабочей папке на компьютере. Если указать `.`, то монтируется текущая папка.
   - Флаг `-it` запускает контейнер в интерактивном режиме с терминалом.
   - `-e DISPLAY=host.docker.internal:0` передаёт адрес графического сервера (Xserver) внутрь контейнера.
   - `-v` монтирует вашу локальную папку в `/workspace` внутри контейнера. Именно таким образом мы будем передавать файлы внутрь контейнера во время работы.
   - После запуска вы окажетесь внутри контейнера в командной оболочке Bash.

6. **Начните работу:**
   ```bash
   cd /workspace
   ```
   Здесь вы можете компилировать и запускать Geant4-приложения.

---

## Linux

1. **Установите Docker Desktop:**
   - Следуйте официальной [инструкции для Linux](https://docs.docker.com/desktop/setup/install/linux/).
   - Убедитесь, что Docker запущен и работает: `sudo systemctl status docker` (или `docker ps`).

2. **Установите утилиты Xserver:**
   - **Ubuntu / Debian:**
     ```bash
     sudo apt update
     sudo apt install x11-xserver-utils
     ```
   - **CentOS / AlmaLinux / Fedora:**
     ```bash
     sudo dnf install xorg-x11-xauth
     ```

3. **Разрешите доступ к Xserver из Docker:**
   ```bash
   xhost +local:docker
   ```
   Это разрешит локальным процессам (включая Docker) подключаться к вашему Xserver.

4. **Загрузите Docker-образ:**
   ```bash
   docker pull jeffersonlab/geant4:g4v11.3.2-almalinux94
   ```

5. **Запустите контейнер:**
   ```bash
   docker run -it -e DISPLAY=$DISPLAY -v /home/username/your-directory/:/workspace jeffersonlab/geant4:g4v11.3.2-almalinux94 /bin/bash
   ```
   - Вместо `/home/username/your-directory/` укажите путь к вашей рабочей папке. Если указать `.`, то монтируется текущая папка.
   - `-e DISPLAY=$DISPLAY` передаёт текущую переменную окружения DISPLAY внутрь контейнера.
   - На некоторых системах может потребоваться добавить `--net=host`, чтобы контейнер мог получить доступ к Xserver через сеть хоста:
     ```bash
     docker run -it --net=host -e DISPLAY=$DISPLAY -v /home/username/your-directory/:/workspace jeffersonlab/geant4:g4v11.3.2-almalinux94 /bin/bash
     ```

6. **Начните работу:**
   ```bash
   cd /workspace
   ```

---

## MacOS

1. **Установите Docker Desktop для Mac:**
   - Скачайте [Docker Desktop](https://www.docker.com/products/docker-desktop/) для macOS (Intel или Apple Silicon в зависимости от вашего Mac).
   - Установите и запустите Docker Desktop. Убедитесь, что в строке меню появился значок Docker.

2. **Установите Xserver:**
   - MacOS больше не включает встроенный Xserver. Рекомендуется установить [XQuartz](https://www.xquartz.org/).
   - После установки перезагрузитесь или перезапустите XQuartz через Launchpad.
   - Запустите XQuartz. В настройках XQuartz (Preferences → Security) убедитесь, что включено "Allow connections from network clients".

3. **Настройте XQuartz:**
   - Откройте терминал и выполните:
     ```bash
     xhost +localhost
     ```
   - Убедитесь, что DISPLAY настроен правильно:
     ```bash
     echo $DISPLAY
     ```
     Обычно это `:0` или `:1`.

4. **Загрузите Docker-образ:**
   ```bash
   docker pull jeffersonlab/geant4:g4v11.3.2-almalinux94
   ```

5. **Запустите контейнер:**
   ```bash
   docker run -it -e DISPLAY=host.docker.internal:0 -v /Users/yourusername/your-directory/:/workspace jeffersonlab/geant4:g4v11.3.2-almalinux94 /bin/bash
   ```
   - Вместо `/Users/yourusername/your-directory/` укажите путь к вашей рабочей папке. Если указать `.`, то монтируется текущая папка.
   - `host.docker.internal` — специальное DNS-имя, которое на macOS указывает на хост-машину.
   - Если у вас используется XQuartz, иногда требуется добавить `--net=host` или передать сокет X11 вручную:
     ```bash
     docker run -it -e DISPLAY=host.docker.internal:0 -v /tmp/.X11-unix:/tmp/.X11-unix -v /Users/yourusername/your-directory/:/workspace jeffersonlab/geant4:g4v11.3.2-almalinux94 /bin/bash
     ```

6. **Начните работу:**
   ```bash
   cd /workspace
   ```

---

## Общие рекомендации

- **Проверка графики:** Чтобы убедиться, что Xserver работает корректно, внутри контейнера можно установить и запустить простую программу, например `xeyes` или `xclock`. Если окно отображается на вашем хосте — графическая подсистема работает.
- **Права доступа:** В Linux и macOS при работе с Xserver могут потребоваться дополнительные настройки безопасности. Команда `xhost +local:docker` упрощает доступ, но в многопользовательской среде используйте её осторожно.
- **Работа без графики:** Если вам не нужен графический вывод (например, только компиляция), аргумент `-e DISPLAY` можно опустить.

Успехов!
