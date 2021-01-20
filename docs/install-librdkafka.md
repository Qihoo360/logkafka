## install librdkafka

For Mac:
```
brew install librdkafka
```

For linux:
```
cd /tmp
wget -N https://github.com/edenhill/librdkafka/archive/master.zip -O librdkafka.zip; unzip librdkafka.zip; rm -f librdkafka.zip
cd librdkafka-master
./configure
make -j4
sudo make install
```
	
Any problems, check [this](https://github.com/edenhill/librdkafka)
