## logkafka - Collect logs and send lines to Apache Kafka 0.8

## Introduction [中文文档](https://github.com/Qihoo360/logkafka/wiki)

logkafka sends log file contents to kafka 0.8 line by line. It treats one line of file as one kafka message.

See [FAQ](docs/FAQ.md) if you wanna deploy it in production environment.

![logkafka](docs/imgs/logkafka.png?raw=true "logkafka")

[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/Qihoo360/logkafka?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

## Features

* log collecting configuration management with zookeeper
* [log path with timeformat (collect files chronologically)](docs/Features.md#Log Path Pattern)
* log file rotating
* batching messages
* compression (none, gzip, snappy)
* [message regex filter](docs/Features.md#Regex Filter)
* [user-defined line delimiter](docs/Features.md#Line Delimiter)
* [user-defined monitor](docs/Features.md#Monitor)

## Differences with other log aggregation and monitoring tools 
The main differences with **flume**, **fluentd**, **logstash** are

* Management of log collecting configs and state: 

  flume, fluentd, logstash keep log file configs and state locally: start local server for management of log file configs and state.
  
  logkafka keep log file configs and state in zookeeper: watch the node in zookeeper for configs changing, record file position in local position file, and push position info to zookeeper periodically.

* Order of log collecting

  flume, fluentd, logstash all have INPUT type 'tail', they collecting all files simultaneously, without considering chronological order of log files.
  
  logkafka will collect files chronologically.
  
## Users of logkafka

* [Qihoo360](http://www.360totalsecurity.com/features/360-total-security-mac/) - Collecting openstack operation logs
* [ICBC](http://www.icbc.com.cn/ICBC/sy/default.htm) - Collecting ssdb operation log
* *Let [me](zheolong@126.com) know if you are using logkafka.*

[details...](docs/Users.md)


## Requirements

* librdkafka

* libzookeeper_mt

* libuv

* libpcre2 

* PHP 5.3 and above (with zookeeper extension)

## Build

Two methods, choose accordingly.

1. Install [librdkafka(>0.8.6)](docs/install-librdkafka.md), [libzookeeper_mt](docs/install-libzookeeper_mt.md), [libuv(>v1.6.0)](docs/install-libuv.md), [libpcre2(>10.20)](docs/install-libpcre2.md) manually, then

	```
	cmake -H. -B_build -DCMAKE_INSTALL_PREFIX=_install
	cd _build
	make -j4
	make install
	```

2. Just let cmake handle the dependencies ( **cmake version >= 3.0.2** ).

	```
	cmake -H. -B_build -DCMAKE_INSTALL_PREFIX=_install \
	                   -DINSTALL_LIBRDKAFKA=ON \
	                   -DINSTALL_LIBZOOKEEPER_MT=ON \
	                   -DINSTALL_LIBUV=ON \
	                   -DINSTALL_LIBPCRE2=ON
	cd _build
	make -j4
	make install
	```

## Usage

Note: If you already have kafka and zookeeper installed, you can start from step 2 and replace zk connection string with your own in the following steps, default is ```127.0.0.1:2181```.

1. Deploy Kafka and Zookeeper in local host
    
   ```
   tools/grid bootstrap
   ```

2. Start logkafka 

   * local conf
   
   Customizing _install/conf/logkafka.conf to your needs
   
   ```
    zookeeper.connect = 127.0.0.1:2181
    pos.path       = ../data/pos.myClusterName
    line.max.bytes = 1048576
    ...
   ```
   
   * run

   Run in the foreground
   
   ```
   _install/bin/logkafka -f _install/conf/logkafka.conf -e _install/conf/easylogging.conf
   ```

   Or as a daemon

   ```
   _install/bin/logkafka --daemon -f _install/conf/logkafka.conf -e _install/conf/easylogging.conf
   ```
   
3. Configs Management

  Use UI or command line tools.

  3.1 UI (with kafka-manger)

  We add logkafka as one kafka-manager extension. You need to [install and start kafka-manager](https://github.com/zheolong/kafka-manager/), add cluster with logkafka enabled, then you can manage logkafka with the 'Logkafka' menu.
  
  * How to add cluster with logkafka enabled

  ![logkafka](docs/imgs/add_cluster.png?raw=true "add cluster")
  
  * How to create new config

  ![logkafka](docs/imgs/create_logkafka.png?raw=true "create logkafka")

  * How to delete configs

  ![logkafka](docs/imgs/delete_logkafka.png?raw=true "delete logkafka")

  * How to list configs and monitor sending progress
  
  ![logkafka](docs/imgs/list_logkafka.png?raw=true "list logkafka")


  3.2 Command line tools
  
  We use php script (tools/log_config.php) to create/delete/list collecting configurations in zookeeper nodes.
  
  If you do not know how to install php zookeeper module, check [this](docs/install-php-zookeeper-extension.md).


   * How to create configs
   
     Example: 
   
     Collect apache access log on host "test.qihoo.net" to kafka brokers with zk connection string "127.0.0.1:2181". The topic is "apache_access_log".
   
	   ```
	   php tools/log_config.php --create \
	                            --zookeeper_connect=127.0.0.1:2181 \
	                            --logkafka_id=test.qihoo.net \
	                            --log_path=/usr/local/apache2/logs/access_log.%Y%m%d \
	                            --topic=apache_access_log
	   ```
	   
	   Note: 
	   * [hosname, log_path] is the key of one config.
   
   * How to delete configs
   
	   ```
	   php tools/log_config.php --delete \
	                            --zookeeper_connect=127.0.0.1:2181 \
	                            --logkafka_id=test.qihoo.net \
	                            --log_path=/usr/local/apache2/logs/access_log.%Y%m%d
	   ```
      
   * How to list configs and monitor sending progress
   
	   ```
	   php tools/log_config.php --list --zookeeper_connect=127.0.0.1:2181
	   ```
	   
	   shows 
	   
	   ```
	   logkafka_id: test.qihoo.net
		log_path: /usr/local/apache2/logs/access_log.%Y%m%d
		Array
		(
		    [conf] => Array
		        (
		            [logkafka_id] => test.qihoo.net
		            [log_path] => /usr/local/apache2/logs/access_log.%Y%m%d
		            [topic] => apache_access_log
		            [partition] => -1
		            [key] =>
		            [required_acks] => 1
		            [compression_codec] => none
		            [batchsize] => 1000
		            [message_timeout_ms] => 0
		            [follow_last] => 1
		            [valid] => 1
		        )

		)
	   ```
	   
	More details about configuration management, see `php tools/log_config.php --help`.
   
## Benchmark

We test with 2 brokers, 2 partitions

| Name                 | Description                |
| -------------------- | -------------------------- |
| rtt min/avg/max/mdev | 0.478/0.665/1.004/0.139 ms |
| message average size | 1000 bytes                 |
| batchsize            | 1000                       |
| required_acks        | 1                          |
| compression_codec    | none                       |
| message\_timeout\_ms | 0                          |
| peak rates           | 20.5 Mb/s                  |

## Third Party
The most significant third party packages are:

* confuse

* easylogging

* tclap

* rapidjson

Thanks to the creators of these packages.

## Developers

Make sure you have lcov installed, check [this](http://ltp.sourceforge.net/coverage/lcov/readme.php)

compile with unittest and debug type

```
cmake -H. -B_build -DCMAKE_INSTALL_PREFIX=_install \
                   -Dtest=ON \
                   -DCMAKE_BUILD_TYPE=Debug
cd _build
make
make logkafka_coverage  # run unittest
```

## TODO

1. Regex filter
2. Multi-line mode
