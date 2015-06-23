## FAQ

#### What kind of log path supported

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
  

#### What situation will cause data loss

1. logkafka crash or killed when librdkafka still holds unsent messages

We choose librdkafka as our message producer, librdkafka only provide *async* interface and use internal memory queue. If you set config `message_timeout_ms=0 (default value)`, the librdkafka will keep your unsent messages in its memory queue until sent successfully. Consequencely, the messages will be lost if the program crashes when there are still unsent messages in librdkafka's queue.

Solution: the optimal solution is providing [disk persistent queue](https://github.com/edenhill/librdkafka/issues/31) in librdkafka, and we are working on it.
  
2. log file was deleted before logkafka start collecting it

Assumed that current time is 2015.01.01.03, f1 is being collecting, and f2, f3 are not collected.

```
  f1: /usr/local/logkafka/systest/src/logkafka_test.2015.01.01.01 (Collecting)
  f2: /usr/local/logkafka/systest/src/logkafka_test.2015.01.01.02 (Uncollected)
  f3: /usr/local/logkafka/systest/src/logkafka_test.2015.01.01.03 (Uncollected)
```

Now, delete f1 and f2, the data of f2 will be lost.

The log file will be colleted as fast as possible, but if log file is too huge, or going through network failure, collection will be stuck.  

Solution: monitor the log collecting state will prevent this from happening.

#### Will out-of-order delivery happen

Yes, but just when network failue happen.

