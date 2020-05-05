--TEST--
Just an around method test (version with 2 Pointcuts)
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
        return "{".$jp->process()."}";
    }
);

$interceptor->addAround(
    'AspektTest::foo()',
    function (JoinPoint $jp) {
        return "[".$jp->process()."]";
    }
);


$test = new AspektTest();
echo $test->foo();
?>
--EXPECT--
{[intest]}
