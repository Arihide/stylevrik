# Style-based VRIK -learn

## What is this?
Scaled GPLVMと呼ばれる学習手法を用いて、あらかじめ用意しておいた姿勢データから、姿勢の自然さを評価する関数を学習するプログラムです。


### 実行環境

- Anaconda3
    - python3.6
    - GPy (ガウス過程ライブラリ)
    - mathutils (3DCGに特化した計算ライブラリ)
    - plotly


### mathutilの導入

pipや本家のblender-mathutilsからではうまくいかなかった。
https://gitlab.com/m1lhaus/blender-mathutils からダウンロードする。

```conda env create --file stylevrik_env.yaml```

```conda activate stylevrik_env```


学習
```python learn.py bvh/(target bvh filename).bvh```