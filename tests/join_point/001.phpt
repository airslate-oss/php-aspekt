--TEST--
JoinPoint::getMethodName may be called more than once
--FILE--
<?php
use Aspekt\JoinPoint;
use Aspekt\Interceptor;

class A {
    private $data = 'value';

    public function test()
    {
        return "test {$this->data}";
    }
}

$advice = function (JoinPoint $jp) {
    echo $jp->getMethodName() . '|' . $jp->getMethodName();
};

$interceptor = new Interceptor();
$interceptor->addAround('A::test()', $advice);

$test = new A();
$test->test();
?>
--EXPECT--
test|test
