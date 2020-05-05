--TEST--
Call Interceptor::addAround after function call (exact name)
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\JoinPoint;
use Aspekt\Interceptor;

function test() {
    return 'intest';
}

test();

$interceptor = new Interceptor();
$interceptor->addAround(
    'test()',
    function (JoinPoint $jp) {
        return "before - {$jp->process()} - after";
    }
);

echo test();
?>
--EXPECT--
before - intest - after
