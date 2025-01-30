скачал с https://github.com/kristapsdz/kplot
в репозитории вбил :

```shell
make
```
вылезли ошибки в exempl, все `arch4rand()` заменил на `rand()`, а `arch4rand()_unifor(100)` на `rand() % 100`.

```shell
make install
```

При создании heatmap выяснилось что это операция слишком много ест. Нужно создать картинку с графиком, и отдельно её сохранить.