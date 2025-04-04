# XClock Overlay

透過的なデジタル時計をデスクトップに表示するX11アプリケーション。

## 機能

- 半透明な背景
- カスタマイズ可能な色（背景色・文字色）
- 透明度の調整
- ウィンドウサイズと位置の指定
- マウスイベントの透過（クリックスルー）

## 必要なライブラリ

### RHEL 9 / Rocky Linux 9
```bash
sudo dnf install libX11-devel libXext-devel libXfixes-devel libXrender-devel gcc make
```

### Ubuntu 24.04
```bash
sudo apt install libx11-dev libxext-dev libxfixes-dev libxrender-dev gcc make
```

## ビルド方法

```bash
make
```

## インストール

```bash
sudo make install
```

## 使い方

```bash
xclock_overlay [options]
```

### オプション

- `-geometry WxH+X+Y`: ウィンドウサイズと位置を指定
  - 例: `-geometry 140x40+100+50`（幅140px、高さ40px、x=100、y=50）
  - 例: `-geometry 140x40-0+0`（画面右上に配置）
- `-bg #RRGGBB`: 背景色を指定（デフォルト: #000000）
- `-fg #RRGGBB`: 文字色を指定（デフォルト: #FFFFFF）
- `-alpha N`: 透明度を指定（0-100、デフォルト: 50）
- `-h, --help`: ヘルプメッセージを表示

### 使用例

```bash
# デフォルト設定で起動
xclock_overlay

# カスタム設定で起動
xclock_overlay -geometry 200x60+500+20 -bg "#000080" -fg "#FFFF00" -alpha 70
```

## ビルド済みバイナリ

`dist/`ディレクトリに以下のプラットフォーム向けのビルド済みバイナリを提供しています：

- `dist/el9/xclock_overlay`: RHEL 9 / Rocky Linux 9用
- `dist/ubuntu2404/xclock_overlay`: Ubuntu 24.04用

## ライセンス

MIT License - 詳細は[LICENSE](LICENSE)ファイルを参照してください。
