


dpotri -> 入力：三角行列

GPyのModelの保存形式について
基本的にModelオブジェクトのsave_modelメソッドを呼び出すことでモデルの保存をすることができるが、
引数のCompressをFalseにしてもZipファイルで出力されてしまう。
また、このZipの形式もあやしく、解凍することができない。
- 調べてみるとgp.pyの中で強制的にCompressをTrueにしているようだ

GPyのPosteriorの生成はExact_gaussian_inference内で行われている。

[cholesky分解について](http://nalab.mind.meiji.ac.jp/~mk/labo/text/cholesky.pdf)

必要となるデータ<BR>
    もとになるモーションデータ（ポーズデータの集合）
    対応する潜在変数
    カーネルや重みのハイパーパラメータ

手順
    カーネル関数と潜在変数によって共分散行列を求める
    共分散行列の逆行列を求める。ここで、共分散行列は正定値対称行列なので、コレスキー分解を用いることで計算量を減らせる。（それでもかなり時間がかかる。データ数の3乗のオーダー）