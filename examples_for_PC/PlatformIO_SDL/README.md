
## Visual Studio Code + PlatformIO + SDL2 環境で LovyanGFXを使用する手順

まず最初にVisual Studio Code をインストールし、PlatformIO を使用できる状態にする。  

 ---

### PlatformIOにて、 `platform = native` のビルドができる状態にする。

手順は下記 URL から `Installation` の項目を読んで実施する。  
https://docs.platformio.org/en/latest/platforms/native.html#installation

#### Linuxの場合

`apt` で `build-essential` をインストールする。
```
sudo apt update
sudo apt install build-essential
```

#### macOSの場合
ターミナルから `xcode-select` をインストールする。
```
xcode-select --install
```

#### Windowsの場合
`MSYS2` をここ https://www.msys2.org/ から入手してインストールする。
そのあと、Windowsの`システムのプロパティ`->`環境変数` を開き、 `PATH` に以下の３つのパスを追加する。
```
C:\msys64\mingw32\bin
C:\msys64\ucrt64\bin
C:\msys64\usr\bin
```


 ---

### PlatformIOにて、 SDL2 が使用できる状態にする。

手順は下記 URL から `Install SDL2` の項目を読んで実施する。  
https://docs.lvgl.io/latest/en/html/get-started/pc-simulator.html#install-sdl-2


#### Linuxの場合

apt-getでlibsdl2をインストールする。

```
sudo apt-get install libsdl2 libsdl2-dev
```

#### MacOS OSXの場合

Homebrewを使ってsdl2をインストールする。
```
 brew install sdl2
```

#### Windowsの場合

`platform = native` のビルドを可能にする手順において、 msys2をインストール済みのはずなので、
githubの SDLのリポジトリにアクセスし、SDL2-devel-x.xx.x-mingw のリリースパッケージを入手する。  
https://github.com/libsdl-org/SDL/releases

本記事作成時点のファイル名は `SDL2-devel-2.28.1-mingw.zip`  
これを解凍し、出てきたフォルダの中にある `x86_64-w64-mingw32` フォルダを開き、中に以下の 4つのフォルダがあることを確認。
 - share
 - bin
 - include
 - lib

C:\msys64\mingw32\ を開き、上記の４つのフォルダと同名のフォルダが存在することを確認したら、C:\msys64\mingw32\ 内に上記フォルダの内容を追加する。（上書きコピー）

 ---




