--TEST--
Implementing fluent interface
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
use Aspekt\JoinPoint;
use Aspekt\Interceptor;

interface FluentInterface {}

class UserFluentDemo implements FluentInterface {
    protected $name;
    protected $surname;
    protected $password;

    public function setName($name) {
        echo "Set user name to ", $name, PHP_EOL;
        $this->name = $name;
    }

    public function setSurname($surname) {
        echo "Set user surname to ", $surname, PHP_EOL;
        $this->surname = $surname;
    }

    public function setPassword($password) {
        echo "Set user password to ", $password, PHP_EOL;
        $this->password = $password;
    }
}

$interceptor = new Interceptor();
$interceptor->addAfterReturning(
    'FluentInterface->set*()',
    function(JoinPoint $jp) {
        if (null === $jp->getReturnedValue()) {
            $jp->setReturnedValue($jp->getObject());
        }
    }
);

$userEntity = new UserFluentDemo();
$result = $userEntity
    ->setName('John')
    ->setSurname('Doe')
    ->setPassword('enigma');

echo $result === $userEntity ? 'OK' : 'FAIL';

?>
--EXPECT--
Set user name to John
Set user surname to Doe
Set user password to enigma
OK
