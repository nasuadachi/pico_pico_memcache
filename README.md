# pico_pico_memcache

MIT License     nasu_adachi

Luckfox pico pro max用に書いたkey valueをget/setするだけのプログラムです

get / set する度にソケットはクローズされます

## 使い方

```
% telnet localhost 11211
set MyKey TestValue
```

```
% telnet localhost 11211
get MyKey
TestValue
```

- 設定ファイルなどというものはなく、ソースコードにハードコードされています
- トランザクションなんて高度なものはありませんが、一応最新のデータは手に入ります


基本的にpico内でsetして、他のデバイスからgetするという使い方を想定しています

## ビルド
```
# arm-rockchip830-linux-uclibcgnueabihf-g++ pico_pico_memcache.cpp -o pico_pico_memcache
```

## 実行
```
% ./pico_pico_memcache
```

デーモン化しておくと便利ですが、-zなんて気の利いたコマンドはないです

```
% ./pico_pico_memcache &
```

end