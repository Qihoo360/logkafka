<?php
require_once(dirname(__FILE__)."/getopt-php/Getopt.php");
require_once(dirname(__FILE__)."/getopt-php/Option.php");
require_once(dirname(__FILE__)."/base/Exception.php");
require_once(dirname(__FILE__)."/base/Monitor.php");

class LogConfException extends CustomException {}
class MonitorException extends CustomException {}

main();

function main()
{
    // Check Env
    CommandLineUtils::checkRequiredExtentions();

    // Options
    $helpOpt = new Option(null, 'help');
    $helpOpt -> setDescription('Print usage information.');

    $zookeeper_connectOpt = new Option(null, 'zookeeper_connect', Getopt::REQUIRED_ARGUMENT);
    $zookeeper_connectOpt -> setDescription('REQUIRED: 
                          The connection string for the zookeeper connection in the form
                          "host1:port1,host2:port2,host3:port3/chroot/path", The "/chroot/path" of connection string 
                          *MUST* be the same as of the kafka server. Multiple URLS can be given to allow fail-over.');

    $createOpt = new Option(null, 'create');
    $createOpt -> setDescription('Create a new config.');
    $deleteOpt = new Option(null, 'delete');
    $deleteOpt -> setDescription('Delete a config');
    $listOpt = new Option(null, 'list');
    $listOpt -> setDescription('List all available configs.');
    $monitorOpt = new Option(null, 'monitor');
    $monitorOpt -> setDescription('Monitor all available configs.');

    $hostnameOpt = new Option(null, 'hostname', Getopt::REQUIRED_ARGUMENT);
    $hostnameOpt -> setDescription('The hostname of machine which holds log files');
    $hostnameOpt -> setValidation(function($value) {
        $ret = AdminUtils::isHostnameValid($value);
        return $ret['valid'];
    });

    $log_pathOpt = new Option(null, 'log_path', Getopt::REQUIRED_ARGUMENT);
    $log_pathOpt -> setDescription('The log file path, like "/usr/local/apache2/logs/access_log.%Y%m%d"');
    $log_pathOpt -> setValidation(function($value) {
        $ret = AdminUtils::isFilePathValid($value);
        return $ret['valid'];
    });

    $topicOpt = new Option(null, 'topic', Getopt::REQUIRED_ARGUMENT);
    $topicOpt -> setDescription('The topic of messages to be sent.');
    $topicOpt -> setValidation(function($value) {
        return AdminUtils::isTopicValid($value);
    });

    $partitionOpt = new Option(null, 'partition', Getopt::REQUIRED_ARGUMENT);
    $partitionOpt -> setDescription('The partition of messages to be sent.'.
                                    '-1 : random'.
                                    'n(>=0): partition n'
                                   );
    $partitionOpt -> setDefaultValue('-1');
    $partitionOpt -> setValidation(function($value) {
        return is_numeric($value);
    });

    $keyOpt = new Option(null, 'key', Getopt::REQUIRED_ARGUMENT);
    $keyOpt -> setDescription('The key of messages to be sent.');
    $keyOpt -> setDefaultValue('');
    $keyOpt -> setValidation(function($value) {
        return is_string($value);
    });

    $requiredAcksOpt = new Option(null, 'required_acks', Getopt::REQUIRED_ARGUMENT);
    $requiredAcksOpt -> setDescription('Required ack number');
    $requiredAcksOpt -> setDefaultValue('1');
    $requiredAcksOpt -> setValidation(function($value) {
        return is_numeric($value);
    });

    $compression_codecOpt = new Option(null, 'compression_codec', Getopt::REQUIRED_ARGUMENT);
    $compression_codecOpt -> setDescription("Optional compression method of messages: ".
        implode(", ", AdminUtils::$COMPRESSION_CODECS)
    );
    $compression_codecOpt -> setDefaultValue('none');
    $compression_codecOpt -> setValidation(function($value) {
        return AdminUtils::isCompressionCodecValid($value);
    });

    $batchsizeOpt = new Option(null, 'batchsize', Getopt::REQUIRED_ARGUMENT);
    $batchsizeOpt -> setDescription('The batch size of messages to be sent');
    $batchsizeOpt -> setDefaultValue('1000');
    $batchsizeOpt -> setValidation(function($value) {
        return (is_numeric($value) && (int)$value > 0);
    });

    $follow_lastOpt = new Option(null, 'follow_last', Getopt::REQUIRED_ARGUMENT);
    $follow_lastOpt -> setDescription('If set to "false", when restarting logkafka process, 
                          the log_path formatted with current time will be collect;
                          If set to "true", when restarting logkafka process, the last 
                          collecting file will be collected continually');
    $follow_lastOpt -> setDefaultValue('true');
    $follow_lastOpt -> setValidation(function($value) {
        return in_array($value, array('true', 'false'));
    });

    $message_timeout_msOpt = new Option(null, 'message_timeout_ms', Getopt::REQUIRED_ARGUMENT);
    $message_timeout_msOpt -> setDescription('Local message timeout. This value is only enforced locally 
                          and limits the time a produced message waits for successful delivery. 
                          A time of 0 is infinite.');
    $message_timeout_msOpt -> setDefaultValue('0');
    $message_timeout_msOpt -> setValidation(function($value) {
        return (is_numeric($value) && (int)$value >= 0);
    });

    $regex_filter_patternOpt = new Option(null, 'regex_filter_pattern', Getopt::REQUIRED_ARGUMENT);
    $regex_filter_patternOpt -> setDescription("Optional regex filter pattern, the messages matching this pattern will be dropped");
    $regex_filter_patternOpt -> setDefaultValue('');
    $regex_filter_patternOpt -> setValidation(function($value) {
        return AdminUtils::isRegexFilterPatternValid($value);
    });

    $lagging_max_bytesOpt = new Option(null, 'lagging_max_bytes', Getopt::REQUIRED_ARGUMENT);
    $lagging_max_bytesOpt -> setDescription("log lagging max bytes, the monitor will alarm according to this setting");
    $lagging_max_bytesOpt -> setDefaultValue('');
    $lagging_max_bytesOpt -> setValidation(function($value) {
        return (is_numeric($value) && (int)$value >= 0);
    });

    $monitorNameOpt = new Option(null, 'monitor_name', Getopt::REQUIRED_ARGUMENT);
    $monitorNameOpt -> setDescription("the monitor name");
    $monitorNameOpt -> setDefaultValue('');
    $monitorNameOpt -> setValidation(function($value) {
        return AdminUtils::isMonitorNameValid($value);
    });

    $validOpt = new Option(null, 'valid', Getopt::REQUIRED_ARGUMENT);
    $validOpt -> setDescription('Enable now or not');
    $validOpt -> setDefaultValue('true');
    $validOpt -> setValidation(function($value) {
        return in_array($value, array('true', 'false'));
    });

    $parser = new Getopt(array(
        $zookeeper_connectOpt,

        $createOpt,
        $deleteOpt,
        $listOpt,
        $monitorOpt,

        $hostnameOpt,
        $log_pathOpt, 

        $topicOpt,
        $partitionOpt,

        $keyOpt,
        $requiredAcksOpt,
        $compression_codecOpt,
        $batchsizeOpt,
        $follow_lastOpt,
        $message_timeout_msOpt,
        $regex_filter_patternOpt,
        $lagging_max_bytesOpt,
        $monitorNameOpt,
        $validOpt,
    ));

    try {
        $parser->parse();
        // Error handling and --help functionality omitted for brevity
        $createOptVal  = 0;
        $deleteOptVal  = 0;
        $listOptVal    = 0;
        $monitorOptVal = 0;
        
        if ($parser["create"])  $createOptVal  = 1;
        if ($parser["delete"])  $deleteOptVal  = 1;
        if ($parser["list"])    $listOptVal    = 1;
        if ($parser["monitor"]) $monitorOptVal = 1;

    } catch (UnexpectedValueException $e) {
        echo "Error: ".$e->getMessage()."\n";
        echo $parser->getHelpText();
        exit(1);
    }

    // actions
    $actions = $createOptVal + $deleteOptVal + $listOptVal + $monitorOptVal; 
    
    if ($actions != 1)
        CommandLineUtils::printUsageAndDie($parser, "Command must include exactly one action: --create, --delete, --list or --monitor");
    
    // check args
    checkArgs($parser);

    // create admin utils
    $adminUtils = new AdminUtils($parser['zookeeper_connect']);
    $adminUtils->init();
    $adminUtils->checkZkState();

    // create monitor
    if ($monitorOptVal) {
        try {
            $monitor = createMonitor($parser['monitor_name']);
        } catch (Exception $e) {
            echo "Caught Exception ('{$e->getMessage()}')\n{$e}\n";
            exit(1);
        }
    } else {
        $monitor = NULL;
    }
    
    $adminUtils->setMonitor($monitor);
    
    try {
        if ($parser['create'])
            createConfig($adminUtils, $parser);
        else if ($parser['delete'])
            deleteConfig($adminUtils, $parser);
        else if ($parser['list'])
            listConfig($adminUtils, $parser);
        else if ($parser['monitor'])
            monitorConfig($adminUtils, $parser);
    } catch (LogConfException $e) {
        echo "Caught LogConfException ('{$e->getMessage()}')\n{$e}\n";
    } catch (Exception $e) {
        echo "Caught Exception ('{$e->getMessage()}')\n{$e}\n";
    }
}

/**
 * Print usage and exit
 */
function checkArgs($parser)
{/*{{{*/
    // check required args
    CommandLineUtils::checkRequiredArgs($parser, array('zookeeper_connect'));
    if ($parser['create'] !== NULL || $parser['delete'] !== NULL)
        CommandLineUtils::checkRequiredArgs($parser, array('hostname'));

    if ($parser['monitor'] !== NULL)
        CommandLineUtils::checkRequiredArgs($parser, array('monitor_name'));

    // check invalid args
    // FIXME
}/*}}}*/

function createConfig($adminUtils, $parser)
{/*{{{*/
    $required = array(
        "hostname",
        "log_path",
        "topic",
        );
    //CommandLineUtils::checkRequiredArgs($parser, $required);
    CommandLineUtils::checkRequiredArgs($parser, array('hostname','log_path','topic'));

    $configs = getConfig($parser, AdminUtils::$LOG_COLLECTION_CONFIG_ITEMS);

    $adminUtils->createConfig($configs);
}/*}}}*/

function deleteConfig($adminUtils, $parser)
{/*{{{*/
    $required = array(
        "hostname",
        "log_path",
        //"topic",
        );
    $invalidOptions = array_diff(array_keys(AdminUtils::$LOG_COLLECTION_CONFIG_ITEMS), $required);

    CommandLineUtils::checkRequiredArgs($parser, $required);
    //CommandLineUtils::checkInvalidArgs($parser, 'delete', $invalidOptions);

    $configs = getConfig($parser, AdminUtils::$LOG_COLLECTION_CONFIG_ITEMS);

    $adminUtils->deleteConfig($configs);
}/*}}}*/

function listConfig($adminUtils, $parser)
{/*{{{*/
    $configs = getConfig($parser, AdminUtils::$LOG_COLLECTION_CONFIG_ITEMS);
    $adminUtils->listConfig($configs);
}/*}}}*/

function monitorConfig($adminUtils, $parser)
{/*{{{*/
    $configs = getConfig($parser, AdminUtils::$LOG_COLLECTION_CONFIG_ITEMS);
    $adminUtils->monitorConfig($configs);
}/*}}}*/

function getConfig($parser, $items)
{/*{{{*/
    $configs = array();
    foreach ($items as $item_name => $item_detail)
    {
        if ($parser[$item_name] !== NULL)
        {
            $configs[$item_name] = $parser[$item_name];
        }
        else
        {
            $configs[$item_name] = $items[$item_name]['default'];
        }
    }

    return $configs;
}/*}}}*/

function createMonitor($monitor_name)
{
    $monitor_class_name = "Monitor$monitor_name";
    $monitor_file_name = "$monitor_class_name.php";
    require_once(dirname(__FILE__)."/plugin/$monitor_file_name");

    $monitor = new $monitor_class_name();

    return $monitor;
}

class CommandLineUtils 
{/*{{{*/
    static $EXTENTIONS_REQUIRED = array(
        'zookeeper' => array('version' => '0.1.0'),
    );
    /**
     * Check that all the listed options are present
     */
    static function checkRequiredArgs($parser, $required)
    {/*{{{*/
        foreach ($required as $arg)
        {
            if ($parser[$arg] == NULL)
                CommandLineUtils::printUsageAndDie($parser, "Missing required argument \"".$arg."\"");
        }
    }/*}}}*/

    static function checkInvalidArgs($parser, $usedOption, $invalidOptions)
    {/*{{{*/
        if ($parser[$usedOption] !== NULL)
        {
            foreach ($invalidOptions as $arg)
            {
                if ($parser[$arg] !== NULL)
                    CommandLineUtils::printUsageAndDie($parser, 
                        "Option \"".$usedOption."\" can't be used with option \"".$arg."\"");
            }
        }
    }/*}}}*/
    
    /**
     * Print usage and exit
     */
    static function printUsageAndDie($parser, $message)
    {/*{{{*/
        echo $message;
        exit(1);
    }/*}}}*/

    static function checkRequiredExtentions()
    {/*{{{*/
        foreach (CommandLineUtils::$EXTENTIONS_REQUIRED as $ext_name => $ext_info)
        {
            CommandLineUtils::checkExtention($ext_name, $ext_info['version']);
        }
    }/*}}}*/

    static function checkExtention($name, $version_min)
    {/*{{{*/
        if (!extension_loaded($name)) {
            if (!dl("$name.so")) {
                echo "Failed to load php extension($name)\n";
                exit(1);
            }
        }

        $ext = new ReflectionExtension($name);
        $ver = $ext->getVersion();
        if (version_compare($ver, $version_min) < 0) {
             echo "Required version of extension($name) is at least $version_min\n";
             exit(1);
        }
    }/*}}}*/
}/*}}}*/

class AdminUtils 
{/*{{{*/
    private $zkClient_;
    private $zookeeper_connect_;
    private $zookeeper_urls_;
    private $kafka_chroot_path_;
    private $log_collect_config_path_;
    private $log_collect_client_path_;
    private $monitor_;

    static $acl = array(
        array('perms' => 0x1f, 'scheme' => 'world','id' => 'anyone')
    );

    static $LOG_COLLECTION_CONFIG_ITEMS = array(
        'hostname'   => array('type'=>'string', 'default'=>''),
        'log_path' => array('type'=>'string', 'default'=>''),
        'topic'      => array('type'=>'string', 'default'=>''),
        'partition'  => array('type'=>'integer', 'default'=>'-1'),
        'key'        => array('type'=>'string','default'=>''),
        'required_acks' => array('type'=>'integer', 'default'=>'1'),
        'compression_codec' => array('type'=>'string', 'default'=>'none'),
        'batchsize'   => array('type'=>'integer', 'default'=>'1000'),
        'message_timeout_ms'   => array('type'=>'integer', 'default'=>'0'),
        'regex_filter_pattern'   => array('type'=>'string', 'default'=>''),
        'lagging_max_bytes'   => array('type'=>'integer', 'default'=>'0'),
        'follow_last' => array('type'=>'bool', 'default'=>'true'),
        'valid'       => array('type'=>'bool', 'default'=>'true'),
        );

    static $COMPRESSION_CODECS = array(
        'none',
        'gzip',
        'snappy',
        );

    function __construct($zookeeper_connect)
    {
        $this->zookeeper_connect_ = $zookeeper_connect;
        list($this->zookeeper_urls_, $this->kafka_chroot_path_) = AdminUtils::splitZookeeperConnect($zookeeper_connect);
        $this->log_collect_config_path_ = $this->kafka_chroot_path_."/logkafka/config";
        $this->log_collect_client_path_ = $this->kafka_chroot_path_."/logkafka/client";
    }

    public function setMonitor($monitor)
    {
        $this->monitor_ = $monitor;
    }

    public static function splitZookeeperConnect($zookeeper_connect)
    {
        $first_slash_pos = strpos($zookeeper_connect, '/');
        if ($first_slash_pos !== false)
        {
            $zookeeper_urls = substr($zookeeper_connect, 0, $first_slash_pos);
            $kafka_chroot_path = substr($zookeeper_connect, $first_slash_pos);
        }
        else
        {
            $zookeeper_urls = $zookeeper_connect;
            $kafka_chroot_path = '';
        }

        return array($zookeeper_urls, $kafka_chroot_path);
    }

    public function init()
    {
        $this->zkClient_ = new Zookeeper($this->zookeeper_urls_);
    }

    public function createConfig($configs)
    {/*{{{*/
        $hostname = $configs['hostname'];
        unset($configs['hostname']);
        $log_path = $configs['log_path'];
        unset($configs['log_path']);

        $path = $this->log_collect_config_path_.'/'.$hostname;
        self::createPath_($path);
        $data = $this->zkClient_->get($path);
        if ($data === NULL)
        {
            throw new LogConfException("$path doesn't exist!");
        }
        $info = json_decode($data);
        if (is_array($info))
            $info[$log_path] = $configs;
        else
            $info->$log_path = $configs;

        if (!$this->zkClient_->set($path, json_encode($info)))
        {
            throw new LogConfException("set $hostname failed!\n");
        }
    }/*}}}*/

    public function deleteConfig($configs)
    {/*{{{*/
        $hostname = $configs['hostname'];
        $path = $this->log_collect_config_path_.'/'.$hostname;
        $data = $this->zkClient_->get($path);
        if ($data === NULL) {
            throw new LogConfException("$path doesn't exist! \n");
        }
        $info = json_decode($data);
        if (is_array($info))
            unset($info[$configs['log_path']]);
        else
            unset($info->$configs['log_path']);

        if (!$this->zkClient_->set($path, json_encode($info))) {
            throw new LogConfException("set $hostname failed! \n");
        }
    }/*}}}*/

    public function listConfig($configs)
    {/*{{{*/
        if (empty($configs)) {
            throw new LogConfException("configs empty! \n");
        }

        $tmp_ret = $this->getInfo_($configs);

        if (empty($tmp_ret))
        {
            echo("No logkafka configurations.\n");
            return;
        }

        foreach ($tmp_ret as $hostname => $hostname_info)
        {
            if (empty($hostname_info))
            {
                echo("No logkafka configurations for hostname($hostname).\n");
            }

            foreach ($hostname_info as $path => $path_info)
            {
                echo("\n");
                echo("hostname: $hostname\n");
                echo("log_path: $path\n");

                print_r($path_info);
            }
        }
    }/*}}}*/

    public function monitorConfig($configs)
    {/*{{{*/
        if (empty($configs)) {
            throw new LogConfException("configs empty! \n");
        }

        $tmp_ret = $this->getInfo_($configs);

        if (empty($tmp_ret))
        {
            echo("No logkafka configurations.\n");
            return;
        }

        foreach ($tmp_ret as $hostname => $hostname_info)
        {
            if (empty($hostname_info))
            {
                echo("No logkafka configurations for hostname($hostname).\n");
            }

            foreach ($hostname_info as $path => $path_info)
            {
                if ($this->monitor_ != NULL)
                    $this->monitor_->mon($this->zookeeper_connect_, $hostname, $path, $path_info);
            }
        }
    }/*}}}*/

    private function getInfo_($configs)
    {/*{{{*/
        $ret = array();

        if (empty($configs)) {
            return $ret;
        }

        if (array_key_exists('hostname', $configs) && !empty($configs['hostname']))
        {
            // get logkafka/conf/$hostname znode value
            $hostname = $configs['hostname'];

            $ret[$hostname] = array();

            $tmp_ret = $this->getLogCollectionConf_($hostname);
            if ($tmp_ret['errno'] == 0) {
                $lk_host_confs = $tmp_ret['data'];
            } else {
                echo $tmp_ret['errmsg'];
            }
                
            $tmp_ret = $this->getLogCollectionState_($hostname);
            if ($tmp_ret['errno'] == 0) {
                $lk_host_stats = $tmp_ret['data'];
            } else {
                echo $tmp_ret['errmsg'];
            }

            if (empty($lk_host_confs)) {
                return $ret;
            }

            if (array_key_exists('log_path', $configs) && !empty($configs['log_path']) ) {
                $log_path = $configs['log_path'];
                $ret[$hostname][$log_path] =
                    $this->getConfigByHostAndPath_($lk_host_confs, $lk_host_stats, $hostname, $log_path);
            } else {
                $paths = array_keys($lk_host_confs);
                foreach ($paths as $path) {
                    $ret[$hostname][$path] =
                        $this->getConfigByHostAndPath_($lk_host_confs, $lk_host_stats, $hostname, $path);
                }
            }
        }
        else
        {
            // get logkafka/conf/<all hostname> znode value
            $tmp_ret = $this->getLogkafkaHosts_();
            if ($tmp_ret['errno'] == 0) {
                $hosts = $tmp_ret['data'];
            } else {
                echo $tmp_ret['errmsg'];
            }

            if (empty($hosts)) {
                return $ret;
            }
            
            foreach ($hosts as $host)
            {
                $ret[$host] = array();

                $tmp_ret = $this->getLogCollectionConf_($host);
                if ($tmp_ret['errno'] == 0) {
                    $lk_host_confs = $tmp_ret['data'];
                } else {
                    echo $tmp_ret['errmsg'];
                }
                    
                $tmp_ret = $this->getLogCollectionState_($host);
                if ($tmp_ret['errno'] == 0) {
                    $lk_host_stats = $tmp_ret['data'];
                } else {
                    echo $tmp_ret['errmsg'];
                }

                if (empty($lk_host_confs)) {
                    continue;
                }

                if (array_key_exists('log_path', $configs) && !empty($configs['log_path']) ) {
                    $log_path = $configs['log_path'];
                    $ret[$host][$log_path] = 
                        $this->getConfigByHostAndPath_($lk_host_confs, $lk_host_stats, $host, $log_path);
                } else {
                     $paths = array_keys($lk_host_confs);
                     foreach ($paths as $path) {
                         $ret[$host][$path] = 
                             $this->getConfigByHostAndPath_($lk_host_confs, $lk_host_stats, $host, $path);
                     }
                }
            }
        }

        return $ret;
    }/*}}}*/

    private function getConfigByHostAndPath_($lk_host_confs, $lk_host_stats, $hostname, $log_path)
    {/*{{{*/
        $ret = array();
        if (empty($lk_host_confs)) $lk_host_confs = array();
        if (empty($lk_host_stats)) $lk_host_stats = array();

        if (array_key_exists($log_path, $lk_host_confs))
        {
            $ret['conf'] = $lk_host_confs[$log_path];
            if (array_key_exists($log_path, $lk_host_stats))
                $ret['stat'] = $lk_host_stats[$log_path];
        }

        return $ret;
    }/*}}}*/

    private function getLogCollectionState_($hostname)
    {/*{{{*/
        $state = array();
        $path = $this->log_collect_client_path_.'/'.$hostname;
        $state = array();
        if ($this->zkClient_->exists($path))
        {
            $data = $this->zkClient_->get($path);
            if ($data !== NULL)
            {
                $state = json_decode($data, true);
            }
        }

        return array('errno' => 0, 'errmsg' => "", 'data' => $state);
    }/*}}}*/

    private function createPath_($path)
    {/*{{{*/
        $dirs = explode('/', $path);
        $path = '';
        foreach ($dirs as $dir)
        {
            if (empty($dir)) continue;
            $path = "$path/$dir";
            if ($this->zkClient_->exists($path) === false)
            {
                if(NULL == ($this->zkClient_->create($path, '', self::$acl)))
                    return false;
            }
        }

        return true;
    }/*}}}*/

    static function isHostnameValid($hostname)
    {/*{{{*/
        $ret = array('valid'=>true, 'data'=>'');

        if (preg_match('/localhost/', $hostname)) { // do not use localhost
            $ret = array('valid'=>false, 'data'=>'use the real hostname rather than localhost!');
            echo $ret['data'], PHP_EOL;
            return $ret;
        }

        $validHostnameRegex = "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$"; // conform to RFC 1123

        if (!preg_match("/$validHostnameRegex/", $hostname)) {
            $ret = array('valid'=>false, 'data'=>"hostname is invalid, does not match regex pattern '$validHostnameRegex', which conforms to RFC 1123!");
            echo $ret['data'], PHP_EOL;
            return $ret;
        }

        // valid hostname.
        return $ret;
    }/*}}}*/

    static function isFilenameValid($filename)
    {/*{{{*/
        if (strpbrk($filename, "\\/?*:|\"<>") === FALSE) {
            /* $filename is legal; doesn't contain illegal character. */
            return true;
        }
        else {
            /* $filename contains at least one illegal character. */
            return false;
        }
    }/*}}}*/

    static function isFilePathValid($filepath)
    {/*{{{*/
        $ret = array('valid'=>true, 'data'=>'');

        if (!preg_match('/^\//', $filepath)) {   // first char is slash
            $ret = array('valid'=>false, 'data'=>'the first char of absolute file path must be slash /');
            echo $ret['data'], PHP_EOL;
            return $ret;
        }

        if (preg_match('/\/{2,}/', $filepath)) {  // no continuous slash
            $ret = array('valid'=>false, 'data'=>'the absolute file path can not contain continuous slash /');
            echo $ret['data'], PHP_EOL;
            return $ret;
        }

        $filepath_arr = explode('/', $filepath); 
        foreach ($filepath_arr as $item)
        {
            if (empty($item)) continue;
            if (!self::isFilenameValid($item)) { // illegal part
                $ret = array('valid'=>false, 'data'=>'the absolute file path can not contain special characters');
                echo $ret['data'], PHP_EOL;
                return $ret;
            }
        }

        // valid path.
        return $ret;
    }/*}}}*/

    static public function isTopicValid($topic) 
    {/*{{{*/
        // 0 < length < 256 
        $length = strlen($topic);
        $length_valid = ($length > 0 && $length < 256) ? true: false;

        // just "A-Z", "a-z", "0-9", "_", "-" is valid character 
        $pattern_valid = !preg_match('/[^A-Za-z0-9._-]/', $topic)? true: false;

        return ($length_valid and $pattern_valid);
    }/*}}}*/

    static public function isCompressionCodecValid($compression_codec) 
    {/*{{{*/
        return in_array($compression_codec, self::$COMPRESSION_CODECS);
    }/*}}}*/

    static public function isRegexFilterPatternValid($regex_filter_pattern) 
    {/*{{{*/
        $subject = 'This is some text I am searching in';
        if (@preg_match("/$regex_filter_pattern/", $subject) === false) {
            // the regex failed and is likely invalid
            return false;
        }

        return true;
    }/*}}}*/

    static public function isMonitorNameValid($monitor_name) 
    {/*{{{*/
        // TODO
        return true;
    }/*}}}*/

    public function checkZkState()
    {/*{{{*/
        $ext = new ReflectionExtension('zookeeper');
        $ver = $ext->getVersion();
        if (version_compare($ver, '0.2.0') < 0) {
             //throw new LogConfException("Required version of extension($name) is at least $version_min!");
             return;
        }

        $state = getState($this->zkClient_);
        if (ZOK != $state) {
            throw new LogConfException("zookeeper connection state wrong: $state");
        }
    }/*}}}*/

    private function getLogCollectionConf_($hostname)
    {/*{{{*/
        $path = $this->log_collect_config_path_.'/'.$hostname;
        $data = $this->zkClient_->get($path);
        if ($data === NULL)
        {
            return array('errno' => 1, 'errmsg' => "$path doesn't exist!", 'data' => '');
        }

        $info = array();
        $info = json_decode($data, true);

        return array('errno' => 0, 'errmsg' => "", 'data' => $info);
    }/*}}}*/

    private function getLogkafkaHosts_()
    {/*{{{*/
        $path = $this->log_collect_config_path_;
        $data = $this->zkClient_->getChildren($path);
        if ($data === NULL)
        {
            return array('errno' => 1, 'errmsg' => "$path doesn't exist!", 'data' => '');
        }

        return array('errno' => 0, 'errmsg' => "", 'data' => $data);
    }/*}}}*/
}/*}}}*/
?>
