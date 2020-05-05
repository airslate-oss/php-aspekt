--TEST--
Try using finfo_open (may cause segmentation fault)
--SKIPIF--
<?php
include __DIR__ . '/../skipif.inc';
if (!extension_loaded('fileinfo')) die("skip extension fileinfo is not loaded");
?>
--FILE--
<?php
class FinfoWrapper {
    public function open() {
        return finfo_open(FILEINFO_MIME_TYPE);
    }
}

$wrapper = new FinfoWrapper();
$wrapper->open();

echo 'PASS';
?>
--EXPECT--
PASS
