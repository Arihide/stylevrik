# Style-based VRIK -learn

## What is this?
Scaled GPLVMと呼ばれる学習手法を用いて、あらかじめ用意しておいた姿勢データから、姿勢の自然さを評価する関数を学習するプログラムです。


## 環境

### Anaconda3 のインストール
pythonを用いて学習を行いますが、その環境の構築にAnaconda3を使います。
なので、Anaconda3をインストールして下さい。

### パッケージのインストール

次に、学習に必要なパッケージをインストールします。```learn```ディレクトリから以下のコマンドを実行します。  

```conda env create --file stylevrik_env.yaml```  

すると、もろもろのパッケージがインストールされた環境が出来上がるので、  

```conda activate stylevrik_env```

を実行して環境を起動します。

### mathutilsパッケージの導入
学習には上でインストールしたもののほかに、 ```mathutils``` と呼ばれる3DCGに特化した数学ライブラリを用いています。
しかし、このパッケージは ```pip``` や ```conda``` からでは正しくインストールすることができません。

なので、https://gitlab.com/m1lhaus/blender-mathutils からソースをダウンロードしてビルドすることでパッケージをインストールします。


学習
```python learn.py bvh/(target bvh filename).bvh```