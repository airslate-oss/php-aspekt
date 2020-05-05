--TEST--
Just an around method test (version with echo)
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\JoinPoint;
use Aspekt\Interceptor;

class AspektTest {
    public function foo() {
        echo 'intest';
    }
}

$interceptor = new Interceptor();
$interceptor->addAround(
    'AspektTest::foo()',
    function (JoinPoint $jp) {
        echo 'before - ';
        $jp->process();
        echo ' - after';
    }
);

$test = new AspektTest();
$test->foo();
?>
--EXPECT--
before - intest - after
