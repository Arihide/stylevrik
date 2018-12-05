Unityで使われるdllに関するフォルダ

実験しながらなのでかなり散らかっている。

### g++のコマンド
g++ -Wall -I Eigen -fPIC -shared -O2 -o ../libEigen.so Eigen.cpp

### g++のコマンド2
g++ -Wall -I Eigen -fPIC -shared -O2 -o ../AI4Animation/Assets/Plugins/libEigen.so Eigen.cpp

### cl.exeによるテスト
```test.bat```を実行する

### DLLのビルド
```build_dll.bat```を実行する

A compact formula for the derivative of a 3-D rotation in exponential coordinates

Practical Parameterization of Rotations Using the Exponential Map