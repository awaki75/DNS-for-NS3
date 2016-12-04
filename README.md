# DNS module implementation for ns-3 simulator.

## 目的

[janakawest/DNS-for-NS3](https://github.com/janakawest/DNS-for-NS3) を動かす


## 導入方法

```sh
# 必要なパッケージをインストール
sudo apt-get install build-essential python-dev python-pygccxml python-pygoocanvas python-pygraphviz

# ns-3の圧縮ファイルをダウンロード
wget https://www.nsnam.org/release/ns-allinone-3.26.tar.bz2

# 圧縮ファイルを展開
tar xf ns-allinone-3.26.tar.bz2

cd ns-allinone-3.26

# このリポジトリをclone
git clone https://github.com/awaki75/DNS-for-NS3.git ns-3.26/src/dns

# ビルド
./build.py

cd ns-3.26

# examplesの中はパスが通っていないのでdns-example.ccをscratchにコピー
cp src/dns/examples/dns-example.cc scratch

# dns-exampleを実行
./waf --run scratch/dns-example --vis
```
