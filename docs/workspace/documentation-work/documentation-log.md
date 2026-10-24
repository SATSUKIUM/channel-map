# 基準コミットと配置変更

基準コミット: `561f2f3e8760473ed81b757636fd8cbbca045942`

2026-10-25: 現時点のHTMLを `docs/index.html` に配置。整備ログを `docs/workspace/documentation-work/documentation-log.md` に配置し、両方をGit追跡対象とする。コード本体は変更していない。以下は作成時のログで、パスや成果物の記述は当時のローカル作業環境を表す。生成スクリプト・検証用バイナリ等は元の除外対象ディレクトリに残し、公開HTMLは直接編集する。

---

# ドキュメント整備ログ

作成日: 2026-10-25

## 成果物と方針

- index.html: 日本語の独立HTML。CSS内蔵、外部フォント・JavaScript・画像への依存なし。画面幅に応じたレイアウト、目次、印刷用CSS付き。
- READMEの記述をそのまま翻訳するのではなく、現在のヘッダ・実装・CMake・CSVに照合して再構成。
- README.md、ライブラリ実装は変更していない。コミット・プッシュはしていない。
- generate.pyはHTMLを再生成するためのローカル編集用ファイル。
- example.cpp / example.csv / example は掲載コードの検証用成果物。example.cppは初回生成時のコードで、HTML側は検証後に出力を明示的な10進数へ修正した。

## 必須項目

1. 概要: FEEと検出器の相互対応、ChannelMapDopeness、dope vector。
2. インストール: C++17、CMake、ROOT、Linux向け導入手順と利用側CMake。
3. 使用方法: 9列CSV、初期化、正引き、逆引き、完全なmain.cppの例。

## 追加した項目と理由・根拠

- 旧APIとの区別: README前半はChannelMap/ChannelTuple、現在のCMakeはsrc/*.cppを対象とし、旧実装はsrc/notManagedNow/にある。現行API中心に整理。
- CSV移行: mapdata_T103_20260404.csvは9列、mapdata.csvなどは8列。detector.channelを数値にし、読み出し名をdetector.readoutへ分離する必要がある。
- 検索成功の意味: src/channel_map_dopeness.cppのgetDopeKey_*は範囲判定で、未登録スロットの検出をしない。誤用防止のため強調。
- キーの使い分け: FE側とDET側の配列添字は異なる。同じuint32_tでも混用不可。getterは無検査アクセス。
- 辞書と型の上限: item.hpp、dictionary.cpp。ソート順で番号が変わり、uint8_tの種類数超過は検出しない。ハードコードを避ける説明を追加。
- メモリ・重複・再初期化: 配列は範囲の直積で確保、キー重複は代入による上書き。initialize/readCSV/dictionaryには再初期化前の全消去がない。
- CSVパーサの制約: channel_map_readCSV.cpp/inputNormalization.cpp。単純分割、数値表記の制約、列不整合の読み飛ばし、exit/例外の可能性を追記。
- 設定情報の登録と寿命: detector_configurations.hpp、geometry.hpp、calibration.hpp、registerDETConfItem/getOrCreateDETConfItem。unique_ptrでの内部所有と生ポインタ関連付けを区別。
- ビルドの制約: CMakeLists.txtはROOT REQUIRED。ChannelMapConfig.cmake.inは.soと絶対パスを固定。Linux前提を明示し、macOSや再配置対応を保証しない。
- 次の整備候補: READMEでの旧APIの扱い、CSV移行、対応環境、測定条件付き性能情報、ライセンス確認。ライセンスは推測で付与しない。

## 検証

- HTMLの8セクションのID重複なし、目次リンクの参照先を確認。
- 外部HTTP(S)アセット参照なし。
- 掲載したmain.cppに幾何情報登録例を組み込み、clang++ -std=c++17と現行srcの8ファイルでコンパイル・実行成功。
- 2行のCSVで正引きutof/0/0/0/left、逆引きIP下位2オクテット2/169・CH8、幾何情報のZ=100を確認。
- ライブラリの診断ログがstd::coutの16進書式を残すため、逆引き表示例にstd::decを追加。
- CMakeのインストール手順自体はこのmacOS環境では未実行。ブラウザでの表示確認も未実施。
- git check-ignore -vでworkspace/documentation/index.htmlとログが除外されることを確認。

## .gitignoreについて

.gitignoreに /workspace/documentation/ を追加した。ただし元々.gitignore自身が除外され、現在はGit未追跡。したがってこの変更は通常のgit statusやgit diffには出ない。将来この除外設定自体を共有するには、.gitignoreを追跡対象にする等の方針判断が必要。今回の指示ではローカル除外として反映し、ステージングは行っていない。
