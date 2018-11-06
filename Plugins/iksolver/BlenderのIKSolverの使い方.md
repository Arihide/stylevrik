# iksolverの使い方

Blender固有のクラスへの依存はない？＝＝独立したモジュールになっている？

1.IK_CreateSegmentによってIK_Segmentを作成し、IK_SetParentなどを用いてつなぎ合わせてIK_Treeを作る
IK_CreateSegmentの引数は自由度に関するフラグ
全く動かないのであればflag=0,XYZすべてに動くならflag=IK_XDOF|IK_YDOF|IK_ZDOF

IK_SetTransformでTransformをセット

IK_SetTransfromの引数
start: 親ボーンの終端から今のボーンの始点のオフセット量。親とつながっていれば当然０のはず
rest_basis: 親ボーンから見たローカル回転？
basis: rest_basisからの現在の変化量, bPoseChannel構造体のpose_matを使う。pose_matはボーンからではなく、オブジェクトから見たローカル回転？
length: 最終的なボーンの長さ

2.iktreeができたらIK_CreateSolverでIK_Solverを作成

3.IK_SolverAddGoalなどを使って拘束条件を設定。

4.IK_Solveで計算

IKの収束判定：
関節の中で一番大きい角度の変化量deltaのノルムnormが十分小さくなったら終了

m_angle オイラー角
関節角度はオイラー角で保持

jacobian.cppではjacobianそのものの計算はしない
SetDerivativesを通してjacobianを設定する
alpha?
beta->エフェクタ空間のベクトル？
jacobian.m_d_theta 角度の微小変化量