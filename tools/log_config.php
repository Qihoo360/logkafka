<?php
require(dirname(__FILE__)."/getopt-php/Getopt.php");
require(dirname(__FILE__)."/getopt-php/Option.php");
require(dirname(__FILE__)."/Exception.php");

class LogConfException extends CustomException {}

main();

function main()
{
    // Check Env
    CommandLineUtils::checkRequiredExtentions();

    // Options
    $helpOpt = new Option(null, 'help');
    $helpOpt -> setDescription('Print usage information.');

    $zookeeperOpt = new Option(null, 'zookeeper', Getopt::REQUIRED_ARGUMENT);
    $zookeeperOpt -> setDescription('REQUIRED: The connection string for the zookeeper connection in the form host:port.'.
                                    'Multiple URLS can be given to allow fail-over.');

    $createOpt = new Option(null, 'create');
    $createOpt -> setDescription('Create a new config.');
    $deleteOpt = new Option(null, 'delete');
    $deleteOpt -> setDescription('Delete a config');
    $listOpt = new Option(null, 'list');
    $listOpt -> setDescription('List all available configs.');

    $hostnameOpt = new Option(null, 'hostname', Getopt::REQUIRED_ARGUMENT);
    $hostnameOpt -> setDescription('The hostname of machine which holds log files');

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
    $partitionOpt -> setDefaultValue(-1);
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
    $requiredAcksOpt -> setDefaultValue(1);
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
    $batchsizeOpt -> setDefaultValue(1000);
    $batchsizeOpt -> setValidation(function($value) {
        return (is_numeric($value) && $value > 0);
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
    $message_timeout_msOpt -> setDefaultValue(0);
    $message_timeout_msOpt -> setValidation(function($value) {
        return (is_numeric($value) && $value >= 0);
    });

    $validOpt = new Option(null, 'valid', Getopt::REQUIRED_ARGUMENT);
    $validOpt -> setDescription('Enable now or not');
    $validOpt -> setDefaultValue('true');
    $validOpt -> setValidation(function($value) {
        return in_array($value, array('true', 'false'));
    });

    $parser = new Getopt(array(
        $zookeeperOpt,

        $createOpt,
        $deleteOpt,
        $listOpt,

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
        $validOpt,
    ));

    try {
        $parser->parse();
        // Error handling and --help functionality omitted for brevity
        $listOptVal   = 0;
        $createOptVal = 0;
        $deleteOptVal = 0;
        
        if ($parser["list"])   $listOptVal   = 1;
        if ($parser["create"]) $createOptVal = 1;
        if ($parser["delete"]) $deleteOptVal = 1;
    
    } catch (UnexpectedValueException $e) {
        echo "Error: ".$e->getMessage()."\n";
        echo $parser->getHelpText();
        exit(1);
    }

    // actions
    $actions = $listOptVal + $createOptVal + $deleteOptVal; 
    
    if ($actions != 1)
        CommandLineUtils::printUsageAndDie($parser, "Command must include exactly one action: --list, --create, or --delete");
    
    // check args
    checkArgs($parser);
    
    $zkClient = new Zookeeper($parser['zookeeper']);
    AdminUtils::checkZkState($zkClient);
    
    try {
        if ($parser['create'])
            createConfig($zkClient, $parser);
        else if ($parser['delete'])
            deleteConfig($zkClient, $parser);
        else if ($parser['list'])
            listConfig($zkClient, $parser);
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
    CommandLineUtils::checkRequiredArgs($parser, array('zookeeper'));
    if ($parser['list'] === NULL )
        CommandLineUtils::checkRequiredArgs($parser, array('hostname'));

    // check invalid args
    // FIXME
}/*}}}*/

function createConfig($zkClient, $parser)
{/*{{{*/
    $required = array(
        "hostname",
        "log_path",
        "topic",
        );
    //CommandLineUtils::checkRequiredArgs($parser, $required);
    CommandLineUtils::checkRequiredArgs($parser, array('hostname','log_path','topic'));

    $configs = getConfig($parser, AdminUtils::$LOG_COLLECTION_CONFIG_ITEMS);

    AdminUtils::createConfig($zkClient, $configs);
}/*}}}*/

function deleteConfig($zkClient, $parser)
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

    AdminUtils::deleteConfig($zkClient, $configs);
}/*}}}*/

function listConfig($zkClient, $parser)
{/*{{{*/
    $configs = getConfig($parser, AdminUtils::$LOG_COLLECTION_CONFIG_ITEMS);
    AdminUtils::listConfig($zkClient, $configs);
}/*}}}*/

function getConfig($parser, $items)
{/*{{{*/
    $configs = array();
    foreach ($items as $item_name => $item_detail)
    {
        if ($parser[$item_name] !== NULL)
        {
            $configs[$item_name] = $parser[$item_name];
            if ($items[$item_name]['type'] == 'integer')
            {
                $configs[$item_name] = (int)$configs[$item_name];
            }
            if ($items[$item_name]['type'] == 'bool')
            {
                $configs[$item_name] = $configs[$item_name] == "true" ? true: false;
            }
        }
        else
        {
            $configs[$item_name] = $items[$item_name]['default'];
        }
    }

    return $configs;
}/*}}}*/

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
    const LOG_COLLECT_CONFIG_PATH = "/logkafka/config";
    const LOG_COLLECT_CLIENT_PATH = "/logkafka/client";

    static $acl = array(
        array('perms' => 0x1f, 'scheme' => 'world','id' => 'anyone')
    );

    static $LOG_COLLECTION_CONFIG_ITEMS = array(
        'hostname'   => array('type'=>'string', 'default'=>''),
        'log_path' => array('type'=>'string', 'default'=>''),
        'topic'      => array('type'=>'string', 'default'=>''),
        'partition'  => array('type'=>'integer', 'default'=>-1),
        'key'        => array('type'=>'string','default'=>''),
        'required_acks' => array('type'=>'integer', 'default'=>1),
        'compression_codec' => array('type'=>'string', 'default'=>'none'),
        'batchsize'   => array('type'=>'integer', 'default'=>1000),
        'message_timeout_ms'   => array('type'=>'integer', 'default'=>0),
        'follow_last' => array('type'=>'bool', 'default'=>true),
        'valid'       => array('type'=>'bool', 'default'=>true),
        );

    static $COMPRESSION_CODECS = array(
        'none',
        'gzip',
        'snappy',
        );

    static function createConfig($zkClient, $configs)
    {/*{{{*/
        $hostname = $configs['hostname'];
        $path = self::LOG_COLLECT_CONFIG_PATH.'/'.$hostname;
        self::createPath($zkClient, $path);
        $data = $zkClient->get($path);
        if ($data === NULL)
        {
            throw new LogConfException("$path doesn't exist!");
        }
        $info = json_decode($data);
        if (is_array($info))
            $info[$configs['log_path']] = $configs;
        else
            $info->$configs['log_path'] = $configs;

        if (!$zkClient->set($path, json_encode($info)))
        {
            throw new LogConfException("set $hostname failed!\n");
        }
    }/*}}}*/

    static function deleteConfig($zkClient, $configs)
    {/*{{{*/
        $hostname = $configs['hostname'];
        $path = self::LOG_COLLECT_CONFIG_PATH.'/'.$hostname;
        $data = $zkClient->get($path);
        if ($data === NULL) {
            throw new LogConfException("$path doesn't exist! \n");
        }
        $info = json_decode($data);
        if (is_array($info))
            unset($info[$configs['log_path']]);
        else
            unset($info->$configs['log_path']);

        if (!$zkClient->set($path, json_encode($info))) {
            throw new LogConfException("set $hostname failed! \n");
        }
    }/*}}}*/

    static function listConfig($zkClient, $configs)
    {/*{{{*/
        if (empty($configs)) {
            throw new LogConfException("configs empty! \n");
        }

        if (array_key_exists('hostname', $configs) && !empty($configs['hostname']))
        {
            // get logkafka/conf/$hostname znode value
            $hostname = $configs['hostname'];

            $tmp_ret = AdminUtils::getLogCollectionConf($zkClient, $hostname);
            if ($tmp_ret['errno'] == 0) {
                $lk_host_confs = $tmp_ret['data'];
            } else {
                echo $tmp_ret['errmsg'];
            }
                
            $tmp_ret = AdminUtils::getLogCollectionState($zkClient, $hostname);
            if ($tmp_ret['errno'] == 0) {
                $lk_host_stats = $tmp_ret['data'];
            } else {
                echo $tmp_ret['errmsg'];
            }

            if (empty($lk_host_confs)) {
                echo("No logkafka configurations for hostname($hostname).\n");
                return;
            }

            if (array_key_exists('log_path', $configs) && !empty($configs['log_path']) ) {
                AdminUtils::listConfigByHostAndPath($zkClient, $lk_host_confs, $lk_host_stats, $hostname, $configs['log_path']);
            } else {
                $paths = array_keys($lk_host_confs);
                foreach ($paths as $path) {
                    AdminUtils::listConfigByHostAndPath($zkClient, $lk_host_confs, $lk_host_stats, $hostname, $path);
                }
            }
        }
        else
        {
            // get logkafka/conf/<all hostname> znode value
            $tmp_ret = AdminUtils::getLogkafkaHosts($zkClient);
            if ($tmp_ret['errno'] == 0) {
                $hosts = $tmp_ret['data'];
            } else {
                echo $tmp_ret['errmsg'];
            }

            if (empty($hosts)) {
                echo("No logkafka configurations.\n");
                return;
            }
            
            foreach ($hosts as $host)
            {
                $tmp_ret = AdminUtils::getLogCollectionConf($zkClient, $host);
                if ($tmp_ret['errno'] == 0) {
                    $lk_host_confs = $tmp_ret['data'];
                } else {
                    echo $tmp_ret['errmsg'];
                }
                    
                $tmp_ret = AdminUtils::getLogCollectionState($zkClient, $host);
                if ($tmp_ret['errno'] == 0) {
                    $lk_host_stats = $tmp_ret['data'];
                } else {
                    echo $tmp_ret['errmsg'];
                }

                if (empty($lk_host_confs)) {
                    echo("No logkafka configurations for host($host).\n");
                    continue;
                }

                if (array_key_exists('log_path', $configs) && !empty($configs['log_path']) ) {
                    AdminUtils::listConfigByHostAndPath($zkClient, $lk_host_confs, $lk_host_stats, $host, $configs['log_path']);
                } else {
                     $paths = array_keys($lk_host_confs);
                     foreach ($paths as $path) {
                         AdminUtils::listConfigByHostAndPath($zkClient, $lk_host_confs, $lk_host_stats, $host, $path);
                     }
                }
            }
        }
    }/*}}}*/

    static function listConfigByHostAndPath($zkClient, $lk_host_confs, $lk_host_stats, $hostname, $log_path)
    {/*{{{*/
        $ret = array();
        if (empty($lk_host_confs)) $lk_host_confs = array();
        if (empty($lk_host_stats)) $lk_host_stats = array();
        if (array_key_exists($log_path, $lk_host_confs))
        {
            $ret['conf'] = $lk_host_confs[$log_path];
            if (array_key_exists($log_path, $lk_host_stats))
                $ret['stat'] = $lk_host_stats[$log_path];

            echo("\n");
            echo("hostname: $hostname\n");
            echo("log_path: $log_path\n");

            print_r($ret);
        }
        else
        {
            echo("\n");
            echo("No logkafka configurations for hostname($hostname), log_path($log_path).\n");
        }
    }/*}}}*/

    static function getLogCollectionState($zkClient, $hostname)
    {/*{{{*/
        $state = array();
        $path = self::LOG_COLLECT_CLIENT_PATH.'/'.$hostname;
        $state = array();
        if ($zkClient->exists($path))
        {
            $data = $zkClient->get($path);
            if ($data !== NULL)
            {
                $state = json_decode($data, true);
            }
        }

        return array('errno' => 0, 'errmsg' => "", 'data' => $state);
    }/*}}}*/

    static function createPath($zkClient, $path)
    {/*{{{*/
        $dirs = explode('/', $path);
        $path = '';
        foreach ($dirs as $dir)
        {
            if (empty($dir)) continue;
            $path = "$path/$dir";
            if ($zkClient->exists($path) === false)
            {
                if(NULL == ($zkClient->create($path, '', self::$acl)))
                    return false;
            }
        }

        return true;
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
            $ret = array('valid'=>false, 'data'=>'第一个字符必须是/');
            echo $ret['data'], PHP_EOL;
            return $ret;
        }

        if (preg_match('/\/{2,}/', $filepath)) {  // no continuous slash
            $ret = array('valid'=>false, 'data'=>'不可以有连续的/');
            echo $ret['data'], PHP_EOL;
            return $ret;
        }

        $filepath_arr = explode('/', $filepath); 
        foreach ($filepath_arr as $item)
        {
            if (empty($item)) continue;
            if (!self::isFilenameValid($item)) { // illegal part
                $ret = array('valid'=>false, 'data'=>'不能包含特殊字符');
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

    static public function checkZkState($zkClient)
    {/*{{{*/
        $ext = new ReflectionExtension('zookeeper');
        $ver = $ext->getVersion();
        if (version_compare($ver, '0.2.0') < 0) {
             //throw new LogConfException("Required version of extension($name) is at least $version_min!");
             return;
        }

        $state = getState($zkClient);
        if (ZOK != $state) {
            throw new LogConfException("zookeeper connection state wrong: $state");
        }
    }/*}}}*/

    static public function getLogCollectionConf($zkClient, $hostname)
    {/*{{{*/
        $path = self::LOG_COLLECT_CONFIG_PATH.'/'.$hostname;
        $data = $zkClient->get($path);
        if ($data === NULL)
        {
            return array('errno' => 1, 'errmsg' => "$path doesn't exist!", 'data' => '');
        }

        $info = array();
        $info = json_decode($data, true);

        return array('errno' => 0, 'errmsg' => "", 'data' => $info);
    }/*}}}*/

    static public function getLogkafkaHosts($zkClient)
    {/*{{{*/
        $path = self::LOG_COLLECT_CONFIG_PATH;
        $data = $zkClient->getChildren($path);
        if ($data === NULL)
        {
            return array('errno' => 1, 'errmsg' => "$path doesn't exist!", 'data' => '');
        }

        return array('errno' => 0, 'errmsg' => "", 'data' => $data);
    }/*}}}*/
}/*}}}*/
?>
