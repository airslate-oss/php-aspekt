--TEST--
Getting class name in a static method
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

class AspektTest {
    public static function foo() {
        echo 'intest', PHP_EOL;
    }
}

$interceptor = new Interceptor();
$interceptor->addBefore('AspektTest::foo()', function (JoinPoint $jp) {
    var_dump($jp->getClassName());
    echo 'before', PHP_EOL;
});

AspektTest::foo();
?>
--EXPECT--
string(10) "AspektTest"
before
intest
