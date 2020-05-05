--TEST--
Call Aspekt\Interceptor::addAfterThrowing triggers only when functions / methods throws an exception
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\Interceptor;

function doStuff() {
    echo __FUNCTION__, PHP_EOL;
}

function doStuffException() {
    echo __FUNCTION__;
    throw new Exception('Exception doStuffException');
}

class Stuff {
    public function doStuff() {
        echo __METHOD__, PHP_EOL;
    }

    public function doStuffException() {
        echo __METHOD__;
        throw new Exception('Exception doStuffException');
   }

   public static function doStuffStatic() {
        echo __METHOD__, PHP_EOL;
   }

   public static function doStuffStaticException() {
        echo __METHOD__;
        throw new Exception('Exception doStuffStaticException');
   }
}

$interceptor = new Interceptor();
$interceptor->addAfterThrowing('doStuff*()', function() {
    echo '[after]';
});

$interceptor->addAfterThrowing('Stuff->doStuff*()', function() {
    echo '[after]';
});

doStuff();

try {
    doStuffException();
} catch (Exception $e) {
    echo '[caught]', PHP_EOL;
}

$stuff = new Stuff();
$stuff->doStuff();

try {
   $stuff->doStuffException();
} catch (Exception $e) {
   echo '[caught]', PHP_EOL;
}

Stuff::doStuffStatic();

try {
   Stuff::doStuffStaticException();
} catch (Exception $e) {
   echo '[caught]', PHP_EOL;
}
?>
--EXPECT--
doStuff
doStuffException[after][caught]
Stuff::doStuff
Stuff::doStuffException[after][caught]
Stuff::doStuffStatic
Stuff::doStuffStaticException[after][caught]
