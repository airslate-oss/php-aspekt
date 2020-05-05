--TEST--
Should be able to influence specific methods through the interface
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
<?php if (!extension_loaded("session")) die("skip session extension not available"); ?>
--INI--
session.save_handler=files
session.name=PHPSESSID
session.save_path=/tmp
--FILE--
<?php
ob_start();

use Aspekt\Interceptor;
use Aspekt\JoinPoint;

class FileSessionHandler implements SessionHandlerInterface {
    private $savePath;

    public function close() {
        return true;
    }

    public function destroy($id) {
        $file = "{$this->savePath}/sess_{$id}";
        if (file_exists($file)) {
            unlink($file);
        }

        return true;
    }

    public function gc($maxlifetime) {
        foreach (glob("{$this->savePath}/sess_*") as $file) {
            if (filemtime($file) + $maxlifetime < time() && file_exists($file)) {
                unlink($file);
            }
        }

        return true;
    }

    public function open($savePath, $sessionName) {
        if (!$savePath) {
            $savePath = md5(uniqid(microtime(true), true));
        }

        $this->savePath = __DIR__ . '/../_output/' . $savePath;

        if (!is_dir($this->savePath)) {
            mkdir($this->savePath, 0777, true);
        }

        return true;
    }

    public function read($id) {
        return (string)@file_get_contents("{$this->savePath}/sess_{$id}");
    }

    public function write($id, $data) {
        return file_put_contents("{$this->savePath}/sess_{$id}", $data) !== false;
    }
}

$handler = new FileSessionHandler();
$success = session_set_save_handler(
    [$handler, 'open'],
    [$handler, 'close'],
    [$handler, 'read'],
    [$handler, 'write'],
    [$handler, 'destroy'],
    [$handler, 'gc']
 );

// the following prevents unexpected effects when using objects as save handlers
session_register_shutdown();

session_start();
echo ($success ? 'successfully set session handler' : 'failed to set session handler') . "\n";
// proceed to set and retrieve values by key from $_SESSION

$handler->write('foo', 'bar');
var_dump($handler->read('foo'));

$interceptor = new Interceptor();
$interceptor->addAfterReturning(
    'SessionHandlerInterface::read()',
    function(JoinPoint $jp) {
        $jp->setReturnedValue(NULL);
    }
 );

 var_dump($handler->read('foo'));
?>
--EXPECT--
successfully set session handler
string(3) "bar"
NULL
