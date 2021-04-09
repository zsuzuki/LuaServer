# HTTP Server(LUA)

cpprestsdkとlua言語を使用して、手軽に動作するhttpサーバー・クライアントを作成。

httpプロトコルを利用しているだけで，実際の通信内容は基本的にjsonのやり取りのみ。

## 使用ライブラリ

| ライブラリ | 機能 | URL |
|----------|-----|-----|
| lua | 制御用スクリプト | https://www.lua.org/ |
| cpprestsdk| REST SDK | https://github.com/Microsoft/cpprestsdk |
| MultipartEncoder | http マルチパートエンコード | https://github.com/AndsonYe/MultipartEncoder |
| sol2 | C++ luaラッパーライブラリ | https://github.com/ThePhD/sol2 |
| json | C++ jsonユーティリティ | https://github.com/nlohmann/json |

依存関係としてboost(1.65~)も必要。
ビルドにはcmakeを使用している。
