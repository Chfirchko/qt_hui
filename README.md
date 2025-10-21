# HUI

## Сборка через CMake


```bash
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j12
```
## Сборка через QMake - DEPRECATED

```bash
cd build 
qmake6 ../hui.pro
make -j12
```

## Запуск
```bash
./hui
```
