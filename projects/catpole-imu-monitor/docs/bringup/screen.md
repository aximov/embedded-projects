# screen を使ってシリアルモニタを開く

mac で USB シリアル通信デバイスを開くには screen を使う。

まずはデバイスを探す

```
% ls /dev/cu.usbmodem*
/dev/cu.usbmodem11303
```

見つかったデバイスは screen コマンドで読み取れる

```
% screen /dev/cu.usbmodem11303 115200
```

115200 は通信速度を表す baud (ボーレート)。コードの方で設定している値と合わせる必要がある。

Ctrl + a, Shift + h でログを保存できる。ログは screenlog.0 というファイルに保存される。毎回同じファイル名で上書きされるので注意。

screen の終了方法は以下

1. Ctrl + A を押す
2. K を押す
3. y を押す