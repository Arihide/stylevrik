Unityで使われるdllに関するフォルダ

実験しながらなのでかなり散らかっている。

### g++のコマンド
g++ -Wall -I Eigen -fPIC -shared -O2 -o ../libEigen.so Eigen.cpp

### g++のコマンド2
g++ -Wall -I Eigen -fPIC -shared -O2 -o ../AI4Animation/Assets/Plugins/libEigen.so Eigen.cpp

### clのコマンド
cl /EHsc /c iksolver.cpp iksolver/intern/*.cpp /I C:\Users\Takahashi\Desktop\VRAvator\Plugins /DM_PI=3.14159265358979323846 <br>
link /DLL /OUT:iksolver.dll iksolver.obj IK_*.obj


A compact formula for the derivative of a 3-D rotation in exponential coordinates

Practical Parameterization of Rotations Using the Exponential Map