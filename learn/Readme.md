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