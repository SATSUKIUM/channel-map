# ブランチ調査と設定所有権の移行（2026-10-30）

## 調査対象

- channel-map: /Users/kashima/GitHub/channel-map
- nestdaq-user-impl: /Users/kashima/scp_landing_zone/20261018/nestdaq-user-impl
- originをfetchした上で、コミット日時だけでなく祖先関係と固有コミットを比較。

## nestdaq-user-implの関係（作業開始時）

| ブランチ | 先端 | 判定 |
| --- | --- | --- |
| fix/checked-channel-lookup | a8e6946（2026-10-25） | 前回の検索API移行を含む。GPUブランチとの共通祖先は17bc943 |
| origin/LogicFilterMore32GPU | e14424d（2026-10-22） | CUDA可視デバイス指定を追加。検索API移行は未収録 |
| origin/LogicFilterMore32 | a4ef1c8（2026-10-18） | 検索API移行ブランチの祖先。固有コミットなし |
| origin/main | 22e01d0（2026-10-20） | 検索API移行ブランチの祖先。固有コミットなし |
| deve_logicfilter_gpu | 7ceddf9 | 別系統と分岐しており「最新」という一本の順序にはならない |
| origin/deve_tracking / deve_tracking_geometry / deve_tracking_geometry_lfcheck | 2121319 / 23a8631 / 3626718 | 別系統の旧開発履歴。今回の作業土台へ無差別に統合しない |

fix/checked-channel-lookupとorigin/LogicFilterMore32GPUは固有コミットが各1個。
前者は検索API移行、後者はGPUの可視デバイス指定で、変更対象は重ならない。
新しいfeat/map-owned-configを前者から作成し、後者を通常のmergeで統合。
既存ブランチへのforce-pushや履歴の書き換えは行っていない。
統合後にorigin/main、origin/LogicFilterMore32GPU、origin/fix/checked-channel-lookupが
いずれも新ブランチの祖先であることを確認。

## 今回のAPI方針

- マップがDETConfItemと各設定をunique_ptrで排他的に所有する。
- setGeometry / setTdcCalibration / setDriftCalibrationへ用途別に登録。
- 外部の生ポインタ登録・公開メンバポインタ指定テンプレートを廃止。
- 設定はgetConfigurationから読み取り専用参照。較正は具体型のgetterを提供。
- 同種設定の再登録は拒否して既存の参照を維持。成功した設定はマップの破棄まで生存。
- 設定失敗時はfalseを返し、渡された新しいunique_ptrは破棄する。
- DET識別子版は逆引きが必要。FEアドレス版は逆引き不要。
- 数値定義、回転規約、多項式の計算方式の再設計は今回の範囲外。

## 検証

- channel-map: CMake Release設定・ビルド・インストールと実データのスケルトン実行に成功。
- 既存検索3ケース、初期化診断2ケース、所有権2ケースに成功。
- 所有権テストは呼び出し元スコープ終了後の生存、重複拒否、型付きconst参照、
  別チャンネル追加後の参照の安定性、singleton破棄時のデストラクタ呼び出しまで確認。
- HTML掲載例を抽出し、幾何情報・両較正の登録と読み取りをコンパイル・実行。
- nestdaq-user-impl: CMake設定はFairMQ、hiredis、redis++、UHBOOK等の依存不足により未完了。
  全体のビルド・実行は未確認。新しいchannel-mapをインストールしてから利用側も再ビルドが必要。

対応するchannel-mapコードコミット: `7b2762346b4d3d41679e17f96e7af6cce7547a1a`
対応するnestdaq-user-impl移行コミット: `e1e832b3ad93b1715d01eb1336a741221af05e94`
