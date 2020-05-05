--TEST--
Overload arguments call (try in the first around declared)
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\JoinPoint;
use Aspekt\Interceptor;

class AspektTest {
    public function foo($data) {
        echo "My arguments is '{$data}'", PHP_EOL;
    }
}

$test = new AspektTest();
$test->foo('hello');

$interceptor = new Interceptor();

$interceptor->addAround('AspektTest::foo()', function (JoinPoint $jp) {
    echo "== NO OVERLOAD ==\n";
    var_dump($jp->getArguments());
    $jp->process();
});

$interceptor->addAround('AspektTest::foo()', function (JoinPoint $jp) {
    echo "== OVERLOAD ==\n";
    var_dump($jp->getArguments());
    $jp->setArguments(['overload']);
    $jp->process();
});

$test->foo('hello');
?>
--EXPECT--
My arguments is 'hello'
== NO OVERLOAD ==
array(1) {
  [0]=>
  string(5) "hello"
}
== OVERLOAD ==
array(1) {
  [0]=>
  string(5) "hello"
}
My arguments is 'overload'
