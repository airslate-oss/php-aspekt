--TEST--
Performing a dependency check and throwing an exception
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\JoinPoint;
use Aspekt\Interceptor;

final class Highlighter
{
    public function highlight($file)
    {
        bogus_highlighter($file);
    }
}

function assert_bogus_highlighter_enabled(JoinPoint $jp)
{
    $name = 'bogus_highlighter';

    if (!function_exists($name) ||
        (new ReflectionFunction($name))->isDisabled()
    ) {
        throw new AssertionError(
            sprintf(
                'The "%s" function is not enabled.',
                $name
            )
        );
    }
}

$interceptor = new Interceptor();
$interceptor->addBefore(
    'Highlighter->highlight()',
    'assert_bogus_highlighter_enabled'
);

$highlighter = new Highlighter();
$highlighter->highlight('example.js');

?>
--EXPECTF--
Fatal error: Uncaught AssertionError: The "bogus_highlighter" function is not enabled. in %s002.php:%d
Stack trace:
#0 %s002.php(%d): assert_bogus_highlighter_enabled(Object(Aspekt\JoinPoint))
#1 {main}
  thrown in %s002.php on line %d
