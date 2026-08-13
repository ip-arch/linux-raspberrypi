Raspberry Pi 4による組込みLinux演習用セットアップファイル群
Raspberry Pi ImagerでSDカードを作成後、bootfsボリュームに４つの設定ファイルをコピーします。

- IPアドレスをICS用に192.168.137.2に固定
- ヘッドレス用にsshをイネーブル
- 32bitカーネルに固定
- ユーザー・パスワードを設定(変更には次のコマンドを実行）
``` bash
echo "user:"`openssl passwd -6 ` > userconf.txt

