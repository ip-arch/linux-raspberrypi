# linux-raspberrypi
Embedded Linux seminar course materials
⚠️ 注意：本プログラムは現在開発中（未完成）です。動作保証はしておりません。

Raspberry Piを用いた組込みLinux教材配布のためのレポジトリです。
Raspberry Pi 4とRaspberry Pi 3をサポートします。
現時点では、Raspberry Pi 5を入手していないので未テストです
***Pi 5 は64bitカーネルしかサポートしないので、setup下のconfig.txtは使いません。***

1. 環境設定
  1. Makefile.hの冒頭にある環境変数(SBOM_DIST_URL)を使用するRaspberry Pi のOSイメージファイルと整合させてください。
     最新OSを用いる場合、Raspberry Pi OS downloadsのページにSBOMへのリンクがあるので、そのリンクアドレスを貼り付けます。
     このページのLegacyイメージにはSBOMがないので利用できません。ダウンロードページから利用するカーネルに合わせて
     SBOMを探してください。デフォルトの値として、Linux v6.12を用いるbookworm版のURLを埋め込んでいます。
SBOM_DIST_URL := https://downloads.raspberrypi.com/raspios_lite_armhf/images/raspios_lite_armhf-2025-05-13/2025-05-13-raspios-bookworm-armhf-lite.sbom.xz
  2. make pi4 もしくは make pi3で、 必要なツール群とLinuxカーネルをダウンロードし、カーネルモジュール開発環境を整えます。
     make pi5も作成していますが、実行環境がないため、動作未確認です。(pi5は64bitカーネルでしか動作しません)
  3. make pi4-verycleanもしくはpi3-verycleanで、上記コマンドで生成したファイル群を消します。
2. linux/usr/src/linux配下にLinuxカーネルソースコードが展開されます。
3. modules配下にカーネルモジュールサンプルプログラム群が展開されます。
4. apps/Cとapps/Python配下にはCとPythonのアプリケーション関係のファイルが展開されます。 
5. setup下にRaspberry Pi Imagerで書き込んだbootfsに置くファイルがあります。32bitカーネル、Windows ICSとの接続のためのIP固定、sshで動作させるヘッドレスを実現します。 セミナー用に、ユーザー名とパスワードを固定しているので、実システムで運用する場合には、userconf.txtは置き換えてください。
