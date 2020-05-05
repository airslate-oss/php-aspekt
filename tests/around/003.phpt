--TEST--
Just an around method test (version with return)
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\JoinPoint;
use Aspekt\Interceptor;

class AspektTest {
    public function foo() {
        return 'intest';
    }
}

$interceptor = new Interceptor();
$interceptor->addAround(
    'AspektTest::foo()',
    function (JoinPoint $jp) {
        return "before - {$jp->process()} - after";
    }
);

$test = new AspektTest();
echo $test->foo();
?>
--EXPECT--
before - intest - after
