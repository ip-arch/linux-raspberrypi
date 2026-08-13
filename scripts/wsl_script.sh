#!/bin/sh
# WSLでDebianをインストール後、演習環境を構築するためのスクリプト
set -eu
# 環境の構築先ディレクトリをDIR変数に設定する。ここでは、WSLのホームディレクトリとしている。
DIR=$HOME
sudo apt update
sudo apt upgrade -y
sudo apt install wget sudo git build-essential crossbuild-essential-armhf jq xz-utils bison flex bc universal-ctags vim file -y
cd $DIR
[ -d linux-raspberrypi ] || git clone https://github.com/ip-arch/linux-raspberrypi.git
wget -nc https://github.com/ip-arch/linux-raspberrypi/releases/download/emb_materials/gdbserver_13.1-3_armhf.deb

cd $DIR/linux-raspberrypi 
# ラズパイセットアップファイルをC：直下にコピー
# Raspberry Pi ImagerでOSイメージを書き込み後、bootfsボリュームにこの内容をコピーする
mkdir -p /mnt/c/linux_rpi_setup; cp setup/* /mnt/c/linux_rpi_setup
# Linuxカーネルを取得し、ラズパイのカーネル開発環境作成
make pi4
# 確認用 timer.ko 作成
cd modules; make timer.ko

# ラズパイのアドレスはKnownHostsに反映しない
# 受講者PCとラズパイ間のICS接続のみに使用する
mkdir -p ~/.ssh
grep -q "Host rpi" ~/.ssh/config || cat >> ~/.ssh/config <<'EOF'
Host rpi 
	HostName 192.168.137.2 
	UserKnownHostsFile /dev/null 
	StrictHostKeyChecking no 
	User user 
EOF
chmod 700 ~/.ssh
chmod 600 ~/.ssh/config

