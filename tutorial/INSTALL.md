## Полезные ссылки
[Установка С/C++ в VS Code](https://code.visualstudio.com/docs/languages/cpp)

## VS Code
Мы будем использовать `VS Code`.
Просто устанавливаем его на свой компютер.

## Установка C/C++
### Установка дополнения в `VS Code`
В `полезных ссылках` есть об этом информация.
1. Открываем `VS Code`
2. Выберите значок просмотра расширений на панели действий или используйте сочетание клавиш `Ctrl+Shift+X`.
3. Ищем `C/C++`.
4. Устанавливаем.
![](/tutorial/img/1.png)

### Установка компилятора
1. Скачиваем [MSYS2](https://www.msys2.org/)
2. Устанавливаем `MSYS2`, желательно в корневую папку `C:/`
3. Открываем `MSYS2 MINGW64`
![](/tutorial/img/2.png)

4. Вводим команды
```shell
pacman -S mingw-w64-x86_64-arm-none-eabi-gcc
```
```shell
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain
```
Если что-то не получается, советую обратится к [C/C++ for Visual Studio Code](https://code.visualstudio.com/docs/languages/cpp).


## Установка и настройка GTK
GTK - это GNU на которой мы будем делать интерфейс. 

После установки `С/C++` в `VS Code`.
Открываем `MSYS2 MINGW64` и в пишем:
```shell
    pacman -S mingw-w64-x86_64-gtk4
```
После установки нужно настроить подключение библиотек в VS Code.
Для этого в проекте в файле `.vscode/task.json` нужно добавить пути и библиотеки.

Пути можно достать из команды:
```shell
    pkg-config --cflags gtk4  
```
Библиотеки можно достать из команды:
```shell
    pkg-config --libs gtk4  
```
Эти комады могут выдавать не полный путь к каталогам. По этому там где есть в начале `-I` и `-L` нужно будет дописать полный путь.
<details>
<summary>
В итоге в .vscode/task.json должно получиться так: 
</summary>

```json
    {
    "tasks": [
        {
            "type": "cppbuild",
            "label": "C/C++: gcc.exe build active file!!!!",
            "command": "C:\\msys64\\mingw64\\bin\\gcc.exe",
            "args": [
                "-IC:/msys64/mingw64/include/gtk-4.0",
                "-IC:/msys64//mingw64/include/pango-1.0",
                "-IC:/msys64//mingw64/include/harfbuzz",
                "-IC:/msys64//mingw64/include/gdk-pixbuf-2.0",
                "-IC:/msys64//mingw64/include/cairo",
                "-IC:/msys64//mingw64/include/glib-2.0",
                "-IC:/msys64//mingw64/lib/glib-2.0/include",
                "-IC:/msys64//mingw64/include/freetype2",
                "-IC:/msys64//mingw64/include",
                "-IC:/msys64//mingw64/include/graphene-1.0",
                "-IC:/msys64//mingw64/lib/graphene-1.0/include",
                "-mfpmath=sse",
                "-msse",
                "-msse2",
                "-IC:/msys64//mingw64/include/fribidi",
                "-IC:/msys64//mingw64/include/webp",
                "-DLIBDEFLATE_DLL",
                "-IC:/msys64//mingw64/include/libpng16",
                "-IC:/msys64//mingw64/include/pixman-1",      

                "-fdiagnostics-color=always",
                "-g",
                "${file}",
                "-o",
                "${fileDirname}\\${fileBasenameNoExtension}.exe",

                "-LC:/msys64/mingw64/lib",
                "-lgtk-4",
                "-lpangowin32-1.0",
                "-lharfbuzz",
                "-lpangocairo-1.0",
                "-lpango-1.0",
                "-lgdk_pixbuf-2.0",
                "-lcairo-gobject",
                "-lcairo",
                "-lvulkan-1.dll",
                "-lgraphene-1.0",
                "-lgio-2.0",
                "-lglib-2.0",
                "-lintl",
                "-lgobject-2.0",

            ],
            "options": {
                "cwd": "${fileDirname}"
            },
            "problemMatcher": [
                "$gcc"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "detail": "Task generated by Debugger."
        },
    ],
    "version": "2.0.0"
}
```

</details>

Также для удобства редактирования стоит добавить пути к библиотекам в `.vscode/c_cpp_properties.json`.

Этот файл появится когда вы зайдёте в `C/C++ Configurations`.
Для этого нажмите `ctrl+shift+P` и напишите `edit`, нажмите `C/C++: Edit Configurations`. После этого у вас появится `.vscode/c_cpp_properties.json`.
<details>
<summary>
Прописываем туда пути. Должно получиться так:
</summary>

```json
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**",
                "C:/msys64/mingw64/include/gtk-4.0",
                "C:/msys64/mingw64/include/pango-1.0",
                "C:/msys64/mingw64/include/harfbuzz",
                "C:/msys64/mingw64/include/gdk-pixbuf-2.0",
                "C:/msys64/mingw64/include/cairo",
                "C:/msys64/mingw64/include/glib-2.0",
                "C:/msys64/mingw64/lib/glib-2.0/include",
                "C:/msys64/mingw64/include/freetype2",
                "C:/msys64/mingw64/include/graphene-1.0",
                "C:/msys64/mingw64/lib/graphene-1.0/include",
                "C:/msys64/mingw64/include/fribidi",
                "C:/msys64/mingw64/include/webp",
                "C:/msys64/mingw64/include/libpng16",
                "C:/msys64/mingw64/include/pixman-1"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "compilerPath": "C:\\msys64\\mingw64\\bin\\gcc.exe",
            "cStandard": "c17",
            "cppStandard": "gnu++17",
            "intelliSenseMode": "windows-gcc-x64"
        }
    ],
    "version": 4
}
```
</details>

## Распространение приложения
Готовый exe файл можно открыть и он будет работать, но на других устройствах нет. Для этого нужно добавить dll файлы к нашему exe файлу. 

Какие dll использует ваш проект можно через:

```shell
    ldd main.exe | sed -n 's/\([^ ]*\) => \/mingw.*/\1/p' | sort
```
Их можно найти в каталоге `C:/msys2/mingw64/bin`.

Все важные `dll` я закинул в папку `lib`.
