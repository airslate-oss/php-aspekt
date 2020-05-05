--TEST--
Testing invalid callback given to Aspekt\Interceptor::addBefore
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\Interceptor;

class AspektTest {
    public function foo() {
        return 'intest';
    }
}

$interceptor = new Interceptor();
$interceptor->addBefore('AspektTest::foo()', ['Non', 'Existent']);

$test = new AspektTest();
echo $test->foo();
?>
--EXPECTF--
Warning: Aspekt\Interceptor::addBefore() expects parameter 2 to be a valid callback, class 'Non' not found in %s on line %d

Fatal error: Aspekt\Interceptor::addBefore() expects a string for the pointcut as a first argument and a callback as a second argument in %s on line %d
