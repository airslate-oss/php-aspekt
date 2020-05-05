--TEST--
The autoloader is not used for selector
--FILE--
<?php
namespace Foo {
    class Bar {
        public function test() {
        }
    }

    class Baz {
        public function test() {
        }
    }
}

namespace {
    use Aspekt\JoinPoint;
    use Aspekt\Interceptor;

    spl_autoload_register(function ($class) {
        echo 'The autoloader was used for ', $class, PHP_EOL;
    });

    $advice = function (JoinPoint $jp) {
        echo 'The advice was used for ', $jp->getPointcut(), PHP_EOL;
    };

    $interceptor = new Interceptor();
    $interceptor->addAround('Foo\\Bar::test()', $advice);

    $b = new Foo\Baz();
    $b->test();

    $b = new Foo\Bar();
    $b->test();

    echo 'OK';
}
?>
--EXPECT--
The advice was used for Foo\Bar::test()
OK
