# 目的

- C++の特徴である処理速度の高さを活かし、複数のアルゴリズムを比較。
- ChatGPT5.4を使用することで、体験しながらC++を学習

# Pathfinding Visualizer (C++ / Docker / CMake)

迷路形式のグリッドに対して、**BFS / Dijkstra / A\*** で経路探索を行う C++ 製 CLI アプリです。

## この作品で見せられること

- C++ によるアルゴリズム実装
- `queue` / `priority_queue` / `vector` などの標準ライブラリ活用
- 探索手法ごとの差分比較
- Docker による再現可能な実行環境

## 対応アルゴリズム

- BFS
- Dijkstra
- A\*
- compare（3手法をまとめて比較）

## マップ記号

- `S` : スタート
- `G` : ゴール
- `#` : 壁
- `.` : 通路

## ディレクトリ構成

```txt
pathfinding-visualizer/
  src/
    main.cpp
  data/
    sample_map.txt
  CMakeLists.txt
  Dockerfile
  README.md
```

## ローカル実行

### ビルド

```bash
cmake -S . -B build
cmake --build build
```

### 実行例

```bash
./build/pathfinding_visualizer data/sample_map.txt compare
```

## Docker 実行

### イメージ作成

```bash
docker build -t pathfinding-visualizer .
```

### 実行例

```bash
docker run --rm pathfinding-visualizer /app/data/sample_map.txt compare
```

## 出力内容

- 使用アルゴリズム
- 経路発見の有無
- 訪問ノード数
- 経路長
- 実行時間
- 経路を `*` で重ねたマップ表示

## 今後の拡張案

- 探索過程のアニメーション表示
- 重み付きマップ対応
- ランダム迷路生成
- アルゴリズム比較モード
- テスト追加

## C++ を選んだ理由

経路探索は、データ構造の選択や反復処理の効率が結果に表れやすく、C++ の**速度**と**制御のしやすさ**を示しやすい題材だからです。

## AI使用

ChatGPT5.4を使用
