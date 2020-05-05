--TEST--
Using namespaced function as an advice for Interceptor::addAround
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
namespace Acme {
    use Aspekt\JoinPoint;

    class AspektTest {
        public function foo() {
            echo 'intest';
        }
    }

    function advice_for_foo(JoinPoint $jp) {
        echo 'before - ';
        $jp->process();
        echo ' - after';
    }
}

namespace {
    use Acme\AspektTest;
    use Aspekt\Interceptor;

    $interceptor = new Interceptor();
    $interceptor->addAround(
        'Acme\\AspektTest::foo()',
        'Acme\\advice_for_foo'
    );

    $test = new AspektTest();
    $test->foo();
}
?>
--EXPECT--
before - intest - after
