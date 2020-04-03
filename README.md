# lightweight-launcher

## 概要
Windows向け軽量ランチャー

## 機能
- exeをダブルクリックで起動するとタスクトレイに常駐する（要Windows）
    - LLauncher.exeとLLauncher.iniは同じフォルダに配置すること
    - スタートアップに登録しておくと良い
        - win + Rに「shell:startup」と打ち込み、LLauncher.exeのショートカットを入れておく
- 常駐した状態で、初期設定だと「Shift + Ctrl + Z」でランチャーが起動
    - マウス位置を取得し、その座標にメニューがポップアップする
- LLauncher.iniに起動したいファイルorフォルダのパスを記載する
- メニューが出た状態で、以下の操作で任意のファイルorフォルダにアクセス可能
    - リストを直接クリック
    - 矢印キーで選んでEnterキー
    - リストの上からA,B,C…Zキー押下（ショートカットキー機能）
- 設定を選ぶとLLauncher.iniが開かれる（0キーも可）
- 終了を選ぶとタスクキルされる（1キーも可）

## 使用技術
- 内部仕様：C言語
- GUI：WIN32API

## コンパイルコマンド

>gcc LLauncher.c -mwindows

## 注意点
- 文字コードは「Shift JIS」じゃないと文字化けするので注意