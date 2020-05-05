--TEST--
Basic traits test
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

trait TestTrait {
    public function run() {
        echo 'test', PHP_EOL;
    }
}

class TestClass {
    use TestTrait;
}

$interceptor = new Interceptor();
$interceptor->addAround(
    'TestTrait::run()',
    function (JoinPoint $jp) {
        echo 'before', PHP_EOL;
        $jp->process();
        echo 'after', PHP_EOL;
    }
);

$test = new TestClass();
$test->run();
?>
--EXPECTF--
before
test
after
