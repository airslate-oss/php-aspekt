--TEST--
An around method test without call to process
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
        return 'nocall';
    }
);

$test = new AspektTest();
echo $test->foo();
?>
--EXPECT--
nocall
