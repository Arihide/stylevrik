# Style-based VRIK -Plugin

## What is this?

Unityで使われるプラグイン(DLL)を作るためのディレクトリです。


<!-- ### g++のコマンド
g++ -Wall -I Eigen -fPIC -shared -O2 -o ../libEigen.so Eigen.cpp

### g++のコマンド2
g++ -Wall -I Eigen -fPIC -shared -O2 -o ../AI4Animation/Assets/Plugins/libEigen.so Eigen.cpp -->

## 環境

プラグインをビルドするために、
- Build Tools for Visual Studio 2017

をインストールします。

### cl.exeによるテスト
1. ```test.bat```を実行する
2. ```VRIK_SolverTest.exe```を実行する
3. ```All tests were passed```と出力されればOK

### DLLのビルド
1. ```build_dll.bat```を実行する
2. ```VRIKSolver.dll```が作られる

<!-- A compact formula for the derivative of a 3-D rotation in exponential coordinates  
Practical Parameterization of Rotations Using the Exponential Map -->