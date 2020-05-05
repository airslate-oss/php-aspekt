--TEST--
Caching example with multiple objects with advices on
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

class AcmeService {
    /* Some complicated and slow method to data calculation... */
    public function complexCalculation(int $data) : string {
        $result = sprintf('A calculated value from the %s(%d)', __FUNCTION__, $data);
        printf('Call slow method %s(%d)' . PHP_EOL, __FUNCTION__, $data);

        return $result;
    }

    /* Another complicated and slow method to data calculation... */
    public function complexCalculation2(int $data) : string {
        $result = sprintf('A calculated value from the %s(%d)', __FUNCTION__, $data);
        printf('Call slow method %s(%d)' . PHP_EOL, __FUNCTION__, $data);

        return $result;
    }
}

$advice = function (JoinPoint $jp) {
    // In memory cache
    static $cache = [];

    // Get arguments passed to the called method
    $callArguments = $jp->getArguments();
    $data = $callArguments[0];

    // Get the called method name
    $calledMethodName = $jp->getMethodName();

    if (isset($cache[$calledMethodName][$data])) {
        // Yay! There is cached result. Do not perform slow methods
        return $cache[$calledMethodName][$data];
    } else {
        // The cache does not exist, we execute the original method
        $cache[$calledMethodName][$data] = $jp->process();
    }
};

$interceptor = new Interceptor();
$interceptor->addAround('AcmeService::complexCalculation*()', $advice);

// Call slow method once to populate in memory cache
$service1 = new AcmeService();
echo $service1->complexCalculation(1) . PHP_EOL;

// The same as above but for object #2
$service2 = new AcmeService();
echo $service2->complexCalculation2(1) . PHP_EOL;

// Now the cache is populated for both complexCalculation(1) and complexCalculation2(1)
echo $service1->complexCalculation2(1) . PHP_EOL;
echo $service2->complexCalculation(1) . PHP_EOL;

// Making another call on complexCalculation(2)
echo $service2->complexCalculation(2) . PHP_EOL;
echo $service1->complexCalculation(2) . PHP_EOL;
?>
--EXPECT--
Call slow method complexCalculation(1)
A calculated value from the complexCalculation(1)
Call slow method complexCalculation2(1)
A calculated value from the complexCalculation2(1)
A calculated value from the complexCalculation2(1)
A calculated value from the complexCalculation(1)
Call slow method complexCalculation(2)
A calculated value from the complexCalculation(2)
A calculated value from the complexCalculation(2)
