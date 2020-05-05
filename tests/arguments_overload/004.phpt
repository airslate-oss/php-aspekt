--TEST--
Overload variadic argument
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\JoinPoint;
use Aspekt\Interceptor;

function test($p1, $p2 = 200, ...$params) {
    $a = 100;

    print_r(get_defined_vars());
    print_r(func_get_args());
    return $p2 + $a;
}

$interceptor = new Interceptor();
$interceptor->addBefore('test()', function (JoinPoint $jp) {
    $args = $jp->getArguments();
    $args[1] = 300;
    $args[2] = 'extra arg 1';
    $args[3] = 'extra arg 2';

    $jp->setArguments($args);
});

echo test('hello world', new stdClass, [123456789]);
?>
--EXPECT--
Array
(
    [p1] => hello world
    [p2] => 300
    [params] => Array
        (
            [0] => extra arg 1
            [1] => extra arg 2
        )

    [a] => 100
)
Array
(
    [0] => hello world
    [1] => 300
    [2] => extra arg 1
    [3] => extra arg 2
)
400
