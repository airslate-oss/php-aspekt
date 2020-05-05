--TEST--
Call Aspekt\Interceptor::addAfterReturning triggers only when functions / methods ends properly
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\Interceptor;

function doStuff() {
    echo __FUNCTION__;
}

function doStuffException() {
    echo __FUNCTION__;
    throw new Exception('Exception doStuffException');
}

class Stuff {
    public function doStuff() {
        echo __METHOD__;
    }

    public function doStuffException() {
        echo __METHOD__;
        throw new Exception('Exception doStuffException');
   }

   public static function doStuffStatic() {
        echo __METHOD__;
   }

   public static function doStuffStaticException() {
        echo __METHOD__;
        throw new Exception('Exception doStuffStaticException');
   }
}

$interceptor = new Interceptor();
$interceptor->addAfterReturning('doStuff*()', function () {
    echo '[after]', PHP_EOL;
});

$interceptor->addAfterReturning('Stuff->doStuff*()', function () {
    echo '[after]', PHP_EOL;
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
doStuff[after]
doStuffException[caught]
Stuff::doStuff[after]
Stuff::doStuffException[caught]
Stuff::doStuffStatic[after]
Stuff::doStuffStaticException[caught]
