--TEST--
Try using finfo_open
--SKIPIF--
<?php
include __DIR__ . '/../skipif.inc';
if (!extension_loaded('fileinfo')) die("skip extension fileinfo is not loaded");
?>
--FILE--
<?php
$finfo = finfo_open(FILEINFO_MIME_TYPE);
echo finfo_file($finfo, __FILE__);
?>
--EXPECT--
text/x-php
