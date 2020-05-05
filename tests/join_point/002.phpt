--TEST--
Should throw error when call process() in invalid context
--FILE--
<?php
use Aspekt\JoinPoint;
use Aspekt\Interceptor;

function catch_me() {
    return 1;
}

$interceptor = new Interceptor();
$interceptor->addBefore('catch_me()', function (JoinPoint $jp) {
    $jp->process();
});

catch_me();
?>
--EXPECTF--
Fatal error: Aspekt\Joinpoint::process() is only available when the advice was added with Aspekt\Interceptor::addAround() in %s002.php on line %d
