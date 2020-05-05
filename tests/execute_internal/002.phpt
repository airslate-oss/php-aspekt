--TEST--
Try using new finfo()
--SKIPIF--
<?php
include __DIR__ . '/../skipif.inc';
if (!extension_loaded('fileinfo')) die("skip extension fileinfo is not loaded");
?>
--FILE--
<?php
class FinfoFactory {
    public function create() {
        return new finfo(FILEINFO_MIME_TYPE);
    }
}
$factory = new FinfoFactory();
echo $factory->create()->file(__FILE__);
?>
--EXPECTF--
text/x-%s
