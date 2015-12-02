## Features

### <a name="Filter"></a>Filter

#### <a name="Regex Filter"></a>Regex Filter

  We currently support regular expression filter. You can add **regex\_filter\_pattern** through kafka-manager or php script. The default pattern value is "", i.e. no filter.

  The regular expression pattern conforms to PCRE2.

  The dropped messages will be recorded as debug level log. If needed, we will add a new property **regex\_filter\_log\_path**, and the corresponding dropped messages will be recorded into **regex\_filter\_log\_path**.

### <a name="Log"></a>Log

#### <a name="Log Path Pattern"></a>Log Path Pattern

* log path without timeformat
  
  e.g. `/usr/local/logkafka/systest/src/logkafka_test`

  We do **NOT** suggest this kind of log path, with log file getting bigger, it must be rotated, assumed that it's renamed as `/usr/local/logkafka/systest/src/logkafka_test.bak` when it's still opened by logkafka, original log path will be reopened until .bak was totally collected. BUT, If logkafka crash after rotating and before finishing the .bak, the uncollected content of .bak will be lost.
  
  For reliability, add another config for `/usr/local/logkafka/systest/src/logkafka_test.bak`.
  
* log path with timeformat

  e.g. 
  
  ```
  /usr/local/logkafka/systest/src/logkafka_test.%Y%m%d%H
  /usr/local/logkafka/systest/src/logkafka_test.%Y-%m-%d-%H
  /usr/local/logkafka/systest/src/%Y/%m/%d%H/logkafka_test
  ```
  
  The timeformat is allowed to embeded in log path. For certain log path, the log files will be collected chronologically.
  
####  <a name="Line Delimiter"></a>Line Delimiter

The default line delimiter is '\n', you can set the `line_delimiter` when creating or modifying log config. The `line_delimiter` can noly accept the decimal value of ascii character.

Check [ASCII chart](http://www.bluesock.org/~willg/dev/ascii.html).

The logkafka will remove line delimiter by default, if you want to keep it, set `remove_delimiter` to `false`.

  
### Monitor

The Monitor will check collecting information periodically.

#### monitoring with ```Default``` Monitor

The ```Default``` will check for **Not Collecting** or **Lagging (collection were stuck or too slow)**, and print all alarming information to terminal.

* Monitor all configs

```
php tools/log_config.php --monitor --monitor_name=Default \
                         --zookeeper_connect=127.0.0.1:2181
```

* Monitor all configs with specified logkafka_id

```
php tools/log_config.php --monitor --monitor_name=Default \
                         --zookeeper_connect=127.0.0.1:2181 \
                         --logkafka_id=test.qihoo.net
```

* Monitor all configs with specified logkafka_id and log path

```
php tools/log_config.php --monitor --monitor_name=Default \
                         --zookeeper_connect=127.0.0.1:2181 \
                         --logkafka_id=test.qihoo.net \
                         --log_path=/usr/local/apache2/logs/access_log.%Y%m%d
```

#### How to write you won monitor

Create a file ```tools/plugin/Monitor<YourMonitorName>.php```

In this file, Add a class ```Monitor<YourMonitorName>```, extents ```Monitor```, and implements the function ```public function mon($zookeeper_connect, $logkafka_id, $path_pattern, $collectionInfo)```

Then, you can use it by command

```
php tools/log_config.php --monitor --monitor_name=<YourMonitorName> \
                         --zookeeper_connect=127.0.0.1:2181
```

#### Other Monitor Options

|   Option            |     Default      |    Description  |
|---------------------|------------------|-----------------|
| monitor_max_count   |        0         |  Specify checking max count, if setting it to 0, the monitor will keep checking |
| monitor_interval_ms |       3000       |  Specify checking interval.  |

Examples:

```
php tools/log_config.php --monitor --monitor_name=Default \
                         --monitor_max_count=3 \
                         --monitor_interval_ms=3000 \
                         --zookeeper_connect=127.0.0.1:2181
```






