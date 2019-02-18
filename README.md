# Style-based VRIK

## What is this?
Unityで、手先のコントローラから全身のアバターの姿勢を生成するためのシステムです。  
その姿勢の生成のためにScaled GPLVMという機械学習の手法を用いているのが特徴です。

このシステムを使うと、以下のような（なんとも微妙な）動きを生成することができます。

![システムの動作例](./VRWalkGIF.gif)

## システム構成
このシステムは大きく3つのディレクトリに分けられます

1. learn        ->  pythonを用いてモデルの学習を行う
2. Plugins      ->  学習済みモデルを読み込んでリアルタイムに姿勢の最適化を行うUnityのプラグイン
3. unity        ->  最終的なアバターの姿勢をビジュアライズするためのUnityプロジェクト

次からはunityでの学習済みのモデルを用いた実行の方法について説明します。  
pythonでの学習については ```./learn/README.md``` を、Unityのプラグインについては ```./Plugin/README.md``` を参照してください。

## Unityでの実行方法
1. Unity 2018.2.14f1 をインストール。最新版のUnityだと多分プラグインが動作しません  
2. Oculusを接続
3. ```./unity```ディレクトリをUnity Projectとして読み込む
4. main.unity を開く 
5. 再生ボタンを押して実行 

<!-- ## License
Copyright © 2019, Arihide Takahashi. Released under the GPL License. -->