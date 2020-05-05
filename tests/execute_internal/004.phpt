--TEST--
Using Interceptor::addAround for system functions
--SKIPIF--
<?php
include __DIR__ . '/../skipif.inc';
if (!extension_loaded('fileinfo')) die("skip extension fileinfo is not loaded");
?>
--FILE--
<?php
use Aspekt\JoinPoint;
use Aspekt\Interceptor;

$interceptor = new Interceptor();
$interceptor->addAround(
    'microtime()',
    function (JoinPoint $jp) {
        return 'you should go to bed';
    }
);

echo microtime();
?>
--EXPECT--
you should go to bed
