libgp：C++でEigenを用いてGaussian Processを行うプログラム

Covariance Function=共分散関数、カーネル関数 cf
get(x1,x2) がカーネル関数の結果

Compute関数内
Lがカーネル行列（共分散行列）。対称なので下三角だけ計算している。
逆行列を求めるためにコレスキー分解もする

k_star
新しい入力x_starと入力{x_i}のカーネル関数のベクトル

alpha = Y * K
alphaはデータセットから決定されるので、あらかじめalphaとして保持しておく

平均は０固定