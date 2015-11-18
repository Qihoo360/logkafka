<?php
require_once(dirname(__FILE__)."/../base/Monitor.php");

/* class name *MUST* be the same with file name prefix */
class MonitorDefault extends Monitor
{
    /* Space for user-defined arbitrary data */
    private $data_ = array(
        'name' => 'zheolong',
        'mail' => 'zheolong@gmail.com',
        'phone' => 'xxx-xxxx-xxxx'
    );

    /* User-defined monitor function */
    public function mon($zookeeper_connect, $hostname, $path_pattern, $collectionInfo)
    {
        echo("\n");
        echo("hostname: $hostname\n");
        echo("log_path: $path_pattern\n");
        print_r($collectionInfo);

        if (empty($collectionInfo)) return;

        if (!array_key_exists('conf', $collectionInfo)) return;

        $conf = $collectionInfo['conf'];

        if (empty($conf)) return;

        if (!array_key_exists('stat', $collectionInfo))
        {
            $message = "[Logkafka][$zookeeper_connect][$hostname][$path_pattern][Not collecting, please check logkafka error logs]";
            $this->alarm_($this->data_, $message);
            return;
        }

        $stat = $collectionInfo['stat'];

        if (empty($stat)) {
            $message = "[Logkafka][$zookeeper_connect][$hostname][$path_pattern][Not collecting, please check logkafka error logs]";
            $this->alarm_($this->data_, $message);
            return;
        }

        if (array_key_exists('lagging_max_bytes', $conf)) {
            $lagging_max_bytes = (int)$conf['lagging_max_bytes'];
            if ($lagging_max_bytes != 0)
            {
                $filesize = (int)$stat['filesize'];
                $filepos = (int)$stat['filepos'];
                $lagging_bytes = $filesize - $filepos; 
                if ($lagging_bytes > 0)
                {
                    $message = "[Logkafka][$zookeeper_connect][$hostname][$path_pattern][Lagging $lagging_bytes bytes, please check logkafka error logs]";
                    $this->alarm_($this->data_, $message);
                    return;
                }
            }
        }
    }

    private function alarm_($data, $message)
    {
        $user = $this->data_['name'];
        $mail = $this->data_['mail'];
        $phone = $this->data_['phone'];

        $this->sendMail_($mail, $message);
        $this->sendSMS_($phone, $message);
    }

    private function sendMail_($mail, $message)
    {
        echo "Send mail to $mail: $message\n";
    }

    private function sendSMS_($phone, $message)
    {
        echo "Send sms to $phone: $message\n";
    }
}
?>
