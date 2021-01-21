## install librdkafka

For Mac and linux:
```
cd /tmp

wget -N https://github.com/edenhill/librdkafka/archive/0.8.6.zip -O librdkafka.zip;
unzip librdkafka.zip
rm -f librdkafka.zip

cd librdkafka-master
./configure
make -j4
sudo make install
```
	
Any problems, check [this](https://github.com/edenhill/librdkafka)
