# 演習環境設定スクリプト

## 概要

本スクリプトは、Linuxカーネル演習で使用する開発環境を構築するためのスクリプトです。

以下の環境での使用を想定しています。

* Windows + WSL上のDebian
* ネイティブDebian

WSL環境では、Raspberry PiのOSイメージ作成時に使用するセットアップファイルをWindows側へコピーします。ネイティブDebianでは、この処理は自動的にスキップされます。

## 前提

### WSLの場合

WindowsにWSLをインストールし、Debianを導入した状態から実行してください。

スクリプトは、WSLのホームディレクトリを演習環境の構築先として使用します。

```sh
DIR=$HOME
```

`DIR` は、演習で使用するファイルの基準ディレクトリです。将来、構築先を変更する場合は、この変数を変更してください。

### ネイティブDebianの場合

Debianをインストールし、インターネットに接続できる状態で実行してください。

## 実行方法

スクリプトを取得したディレクトリで、以下を実行します。

```sh
chmod +x wsl_script.sh
./wsl_script.sh
```

スクリプトでは、最初にパッケージ情報を更新し、システムを更新した後、演習に必要なパッケージをインストールします。

```sh
sudo apt update
sudo apt upgrade -y
```

その後、セミナー用のLinuxカーネルリポジトリを取得し、Raspberry Pi用の開発環境を構築します。

## Raspberry Pi環境

スクリプトは、セミナー用のRaspberry Pi Linuxカーネルリポジトリを取得します。

```text
linux-raspberrypi/
```

その後、以下を実行してRaspberry Pi 4用のカーネル開発環境を構築します。

```sh
make pi4
```

さらに、演習で使用する確認用カーネルモジュール `timer.ko` を作成します。

```sh
cd modules
make timer.ko
```

## WSLでのRaspberry Piセットアップファイル

WSL上でスクリプトを実行した場合は、Raspberry Piのセットアップに必要なファイルを

```text
C:\linux_rpi_setup
```

にコピーします。

このファイルは、Raspberry Pi ImagerでOSイメージを書き込んだ後、Raspberry Piの `bootfs` ボリュームへコピーするために使用します。

WSL以外のDebian環境では、この処理は実行されません。

## Raspberry PiへのSSH接続

演習では、受講者PCとRaspberry PiをICS（Internet Connection Sharing）で接続します。

スクリプトは、Raspberry Piを以下のSSHホスト名で接続できるように設定します。

```text
Host: rpi
Address: 192.168.137.2
User: user
```

設定後は、例えば以下のように接続できます。

```sh
ssh rpi
```

Raspberry Piのホスト鍵はKnownHostsには登録しません。これは、受講者PCとRaspberry Pi間の演習用ネットワークで使用するためです。

## インストールされる主なツール

以下のツールをインストールします。

* Git
* GCC / build-essential
* ARM hard-float クロスコンパイラ
* jq
* xz-utils
* bison
* flex
* bc
* universal-ctags
* Vim
* file
* wget

また、演習で使用する `gdbserver` パッケージを取得します。

## 再実行について

スクリプトは、既に取得済みのGitリポジトリやファイルについては、可能な範囲で再取得を避けるようになっています。

例えば、Linuxカーネルリポジトリは既に存在する場合にはcloneしません。

ただし、演習環境を最初から確認する場合は、クリーンなDebian環境を用意して実行することを推奨します。

## 実行後の確認

スクリプト終了後、以下を確認してください。

### Linuxカーネル

```sh
ls ~/linux-raspberrypi
```

### Raspberry Pi用モジュール

```sh
ls ~/linux-raspberrypi/modules/timer.ko
```

### SSH設定

```sh
ssh rpi
```

Raspberry Piが接続されていない状態では、`ssh rpi` は接続できません。

## 注意事項

* スクリプトは `sudo` を使用します。
* `apt upgrade` により、Debianのインストール直後に利用可能な更新を適用します。
* Raspberry Piとの接続には、演習で指定されたネットワーク構成を使用してください。
* `192.168.137.2` は演習環境で使用するRaspberry Piのアドレスです。
* このスクリプトで取得するLinuxカーネルリポジトリは、本セミナーの教材と対応しているため、通常のLinuxカーネル開発用リポジトリとして扱わないでください。

