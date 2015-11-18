<?php
abstract class Monitor
{
    /* User-defined monitor function */
    abstract public function mon($zookeeper_connect, $hostname, $path_pattern, $collectionInfo);
}
?>
