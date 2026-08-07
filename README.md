# linux-raspberrypi
Embedded Linux seminar course materials

RaspberryPIを用いた組込みLinux教材配布のためのレポジトリです。
RaspberryPI4とRaspberryPI3をサポートします。
現時点では、RaspberryPI5を入手していないので未テストです

1. 環境設定
  1. Makefile.hの冒頭にある下記環境変数を使用するRaspberryPIのOSイメージファイルと整合させてください。
    DEBIAN_CODENAME=bookworm
    RASPIOS_TARG=armhf
    RASPIOS_SZ=lite
    RASPIOS_DATE=2025-05-13
    LINUXVER=6.12.25
    ARCH=arm
  2. make pi4 もしくは make pi3で、 必要なツール群とLinuxカーネルをダウンロードし、カーネルモジュール開発環境を整えます。
  3. make pi4-verycleanもしくはpi3-verycleanで、上記コマンドで生成したファイル群を消します。
2. linux/usr/src/linux配下にLinuxカーネルソースコードが展開されます。
3. modules配下にカーネルモジュールサンプルプログラム群が展開されます。
4. apps/Cとapps/Python配下にはCとPythonのアプリケーション関係のファイルが展開されます。 
