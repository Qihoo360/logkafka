## install php zookeeper extension

We assume that you already have php installed, version > 5.3.

1. check your php version

	```
	php -v
	```

2. download php zookeeper package

	```
	cd /tmp
	wget -N -O- https://pecl.php.net/get/zookeeper-0.2.2.tgz | tar zxvf -
	cd zookeeper-0.2.2
	phpize
	./configure --with-php-config=/usr/local/php/bin/php-config  --with-libzookeeper-dir=/usr/local/zookeeper
	make
	sudo make install
	```

3. add zookeeper item to php.ini

	check your php.ini

	```
	php -i | grep php.ini
	```

	add below to your php.ini

	```
	; zookeeper
	extension=zookeeper.so
```