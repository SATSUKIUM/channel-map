# 設定所有権と用途別API（2026-10-30）

現在のコード基準コミット: `7b2762346b4d3d41679e17f96e7af6cce7547a1a`

- 両リポジトリにfeat/map-owned-configブランチを作成。調査・統合の経緯はbranch-and-ownership-20261030.mdに記載。
- 設定をマップ側だけで所有し、呼び出し側にはconstポインタを返す。旧APIは削除。
- setGeometry/setTdcCalibration/setDriftCalibrationで登録し、再登録はfalse。既存の設定は破棄・置換しない。
- DETConfItemの所有ポインタとDETIdItemの設定参照を非公開化し、外部の生ポインタを登録できなくした。
- スケルトンとテストを新APIへ移行し、寿命・破棄のテストを追加。
- HTMLの設定節を全面更新。掲載例を実際にコンパイルし、幾何・較正の登録と参照を検証。
- 前回ベンチマークの数値は以前の検索API実装での測定値。今回は性能の再測定なし。

以下は過去の更新記録です。

---

# chmap-configの廃止（2026-10-30）

現在のコード基準コミット: `60a5161617bb3905fa138e9ba996e7ff95e7cfd7`

- 利用方法をCMakeに統一する方針によりscript/chmap-configを削除。
- CMakeのスクリプトインストール設定、READMEのコメント内Makefile例を削除。
- VERSIONはソース内のCMake用メタデータとして維持。単体ファイルのインストールは廃止し、バージョン情報はCMakeパッケージ設定に含める。
- HTML・テスト案内を更新。過去ログのchmap-configに関する記録は履歴として維持。
- 検証: 新規ディレクトリでReleaseの設定・ビルド・インストールに成功し、実CSVを使ったスケルトンの正常終了を確認。
- 新規インストール先とinstall_manifestにchmap-configがないこと、およびCMakeのバージョン設定が2026.10.30を保持していることを確認。
- 過去にインストールしたスクリプトはCMake installでは自動削除されない。既存環境のファイルは今回変更していない。

以下は過去の更新記録です。

---

# ROOT・Gitの必須条件を撤去（2026-10-30）

現在のコード基準コミット: `e5fcd656a7d854bcbb87f05eadf6f8cd3449376a`

- 以前はDebug指定時に旧APIのdebug/search-time.cppが自動ビルドされていた。旧ツールを自動ビルドから外し、無条件・Debug時のROOT検索を撤去。Debugコンパイル設定は維持。
- バージョンは同梱のVERSION（2026.10.30）で管理。CMakeとchmap-configともにGitを呼ばない。インストール先にもshare/channel-map/VERSIONを配布。
- chmap-configは非CMake利用向けの互換用補助として残す。現在のライブラリビルドやfind_packageによる利用には不要。
- READMEとHTMLからROOTとGitの必須条件を除去。Git cloneは任意のソース取得方法としてのみ記載。
- 検証: macOS上でRelease・DebugのCMake設定、ビルド、インストールに成功。ROOT検索を無効にして実行（現在は検索自体がないため指定変数は未使用の警告となる）。
- Debugは.gitを含まないソースコピーと、呼び出すと失敗するGitスタブをPATH先頭に置いて確認。ソース／インストール済みchmap-config --versionは2026.10.30を返す。
- 両ビルドの現行スケルトンを実CSVで実行して成功。ROOTライブラリをリンクせずビルドできた。
- CMake利用側Configの.so固定など、既存のmacOSインストール利用の問題は別課題として維持。

以下は過去の更新記録です。

---

# 初期化診断の更新（2026-10-25）

現在のコード基準コミット: `d16eacfdff4844b4890dc5c49f479e563350a973`

- 一対一の保証は利用者の責任とし、重複を理由に拒否・集約しない。
- CSV読み込み直後、ソート前にFE・DETの双方の重複を検出。標準エラーへ両方の対応と読み込みレコード番号を表示。同一行の繰り返し、逆引き無効時も対象。
- 検索関数に追加処理はなく、初期化時だけ検査。
- 正引き・逆引き配列の合計見積もりが1 GB（10億バイト）を超える場合に確保前に警告。sizeofを使用し、辞書・設定・一時領域等は見積もり対象外。メモリ制限は設けない。
- 既存の配列メモリ表示の固定値48／24バイトを実型のsizeofへ変更し、表示単位をMiBへ修正。
- 正確な警告と不正アクセス防止のため逆引きの積はuint64_tで計算。uint32_tで保持できないスロット数ではoverflow_error。これは1GBの制限とは別のキー表現上の制約。
- テスト: FE重複、DET重複、同一内容の重複、初期化の続行、重複なしの無出力、メモリ境界・正逆合計・64bit計算を確認。65行CSVで巨大な逆引きを確保せずに警告と桁あふれ拒否を確認。
- 既存検索テスト3ケース成功。実CSVのスケルトン実行で重複2組の表示と正常終了を確認。CSVは変更していない。
- HTMLの基準コミットと運用上の注意を更新。

以下は過去の更新記録です。

---

# 現在のドキュメント基準

コード基準コミット: `981bd850c9e8723a77c74e896c2461f9a9146382`

更新日: 2026-10-25

## 未登録チェック付き検索APIへの追従

- docs/index.htmlの基本例をfindDET/findFEへ移行。戻り値はconstポインタ、未登録・未初期化・逆引き未作成の扱いとポインタの寿命を記載。
- 旧getDopeKey_*は範囲判定のみという互換動作を明記。存在確認を利用側だけで管理するという旧案内を差し替え。
- 検出器名ID 0の予約、名前最大255種類、面・読み出し各256種類、超過時の例外を反映。
- FEE下位2オクテットとCHの全ゼロ予約、CSV読み込み時の拒否を追加。
- 幾何情報登録例を取得したDETIdItemでの登録に変更。逆引き必須、未登録・空unique_ptrの拒否を記載。
- 文字列逆引きのint引数の上限チェックを反映し、呼び出し前の狭い型への変換とCSVの範囲検証は依然必要であることを記載。
- 辞書の再構築では内部テーブルをクリアするようになったが、マップ全体の再初期化は依然非対応なので説明を修正。
- 新版のブランチをcloneする手順と、ライブラリ・利用側双方の再ビルドを追加。
- スケルトン、テスト、ベンチマーク、外部利用側の対応を追加。
- 対応するnestdaq-user-implはa8e6946（LogicFilterMore32GPUからのfix/checked-channel-lookup）。依存不足と既存のAppleClang互換性問題により全体ビルドは未確認。
- README.mdの英語化は今回の対象外。docs/index.htmlを直接編集し、以前の除外対象generate.pyは使用していない。

## 今回の検証

- HTMLからmain.cpp・CSV・幾何情報登録例を抽出し、そのまま組み合わせてclang++ -std=c++17 -O2でコンパイル・実行。
- 正引きutof/0/0/0/left、逆引き2.169:8、幾何情報Z=100を確認。
- セクションID、目次リンク、ローカル文書リンク、外部アセット非依存を確認。
- インストール手順、ブラウザでの視覚確認、DAQ上での性能検証は未実施。
- 前回実施の通常ビルド3ケースの成功と、起動が完了しなかったサニタイザー実行を区別して記載。
- ベンチマーク数値は前回のローカル測定結果を記載し、今回再測定はしていない。

以下は過去の基準コミットと作成時点の記録です。旧仕様の記述は履歴として残しています。

---

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
