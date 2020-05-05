--TEST--
Doing stuff after the triggered joinpoint
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

class Greeting
{
    public function hello(): string
    {
        return 'Hello World!';
    }
}

$advice = function (JoinPoint $jp) {
    echo 'Hello from the Advice!', PHP_EOL;
};

$interceptor = new Interceptor();
$interceptor->addAfter('Greeting->hello()', $advice);

$greeting = new Greeting();
echo $greeting->hello();
?>
--EXPECTF--
Hello from the Advice!
Hello World!
