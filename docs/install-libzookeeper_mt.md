## install libzookeeper_mt


```
cd /tmp

wget -N http://archive.apache.org/dist/zookeeper/zookeeper-3.4.6/zookeeper-3.4.6.tar.gz
tar zxvf zookeeper-3.4.6.tar.gz
rm -f zookeeper-3.4.6.tar.gz

cd zookeeper-3.4.6/src/c
./configure
make -j4
sudo make install
```
Any problems, check [this](http://zookeeper.apache.org/doc/r3.1.2/zookeeperProgrammers.html#Installation)
