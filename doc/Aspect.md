# Aspekt

## Contents
- [Introduction](#introduction)
  - [Aspekt extension history](#aspekt-extension-history)
- [Installation](#installation)
  - [Windows](#windows)
  - [Linux / macOS](#linux--macos)
- [A small taste](#a-small-taste)
- [AOP Vocabulary and Aspekt capabilities](#aop-vocabulary-and-aspekt-capabilities)
- [Why or should I use AOP?](#why-or-should-i-use-aop)
- [Aspekt in action](#aspekt-in-action)
  - [Interceptor](#interceptor)
    - [`Aspekt\Interceptor::addBefore`](#aspektinterceptoraddbefore)
    - [`Aspekt\Interceptor::addAfter`](#aspektinterceptoraddafter)
    - [`Aspekt\Interceptor::addAfterReturning`](#aspektinterceptoraddafterreturning)
    - [`Aspekt\Interceptor::addAfterThrowing`](#aspektinterceptoraddafterthrowing)
    - [`Aspekt\Interceptor::addAround`](#aspektinterceptoraddaround)
  - [Joinpoint complete reference](#joinpoint-complete-reference)
    - [`Aspekt\Joinpoint::getKindOfAdvice`](#aspektjoinpointgetkindofadvice)
    - [`Aspekt\Joinpoint::getArguments`](#aspektjoinpointgetarguments)
    - [`Aspekt\Joinpoint::setArguments`](#aspektjoinpointsetarguments)
    - [`Aspekt\Joinpoint::getReturnedValue`](#aspektjoinpointgetreturnedvalue)
    - [`Aspekt\Joinpoint::setReturnedValue`](#aspektjoinpointsetreturnedvalue)
    - [`Aspekt\Joinpoint::process`](#aspektjoinpointprocess)
    - [`Aspekt\Joinpoint::getPointcut`](#aspektjoinpointgetpointcut)
    - [`Aspekt\Joinpoint::getObject`](#aspektjoinpointgetobject)
    - [`Aspekt\Joinpoint::getClassName`](#aspektjoinpointgetclassname)
    - [`Aspekt\Joinpoint::getMethodName`](#aspektjoinpointgetmethodname)
    - [`Aspekt\Joinpoint::getFunctionName`](#aspektjoinpointgetfunctionname)
  - [Pointcuts syntax](#pointcuts-syntax)
    - [Basics](#basics)
    - [Access modifiers](#access-modifiers)
    - [Wildcards](#wildcards)
    - [Simple selectors examples](#simple-selectors-examples)
      - [Functions](#functions)
      - [Methods](#methods)
    - [Selectors using wildcards examples](#selectors-using-wildcards-examples)
    - [Selectors using super wildcards examples](#selectors-using-super-wildcards-examples)
- [FAQ](#faq)
- [License](#license)

## Introduction

Aspekt is a modern aspect-oriented PHP extension with rich features for the new level of software development.
It enables you to use [Aspect Oriented Programming](https://en.wikipedia.org/wiki/Aspect-oriented_programming) in PHP,
without the need to compile or proceed to any other intermediate step before publishing your code.

The Aspekt extension is designed to be the easiest way you can think of for integrating AOP to PHP.

AOP aims to allow separation of cross-cutting concerns (cache, log, security, transactions, etc).

### Aspekt extension history

The Aspekt extension (formerly known as AOP extension) is a project which started a long time ago, even if its
development is quite very new. It was first expected to be a fully PHP developed library, as part of a dependency
injection framework. The Aspect Oriented Programming implementation would have taken the form of auto generated proxies.

That was before [Julien Salleyron](https://github.com/juliens), the lead developer of the project, wanted to take it to
the next level while writing the AOP core features as a PHP’s extension.

[Gérald Croës](https://github.com/geraldcroes) also belongs to the initial team, mainly in charge of the documentation
and discussions around the extension’s API. The latest release of the extension as part of this team
[was dated](https://pecl.php.net/package/AOP/0.2.2b1) November 2012. Since then, the extension has ceased to develop.

At the end of 2018, this extension received a second life as a fork with new name Aspekt. The extension has been
completely rewritten in order to use modern technologies, as well as Zend Engine 3. The lead developer of the project
has become [Serghei Iakovlev](https://github.com/sergeyklay).

## Installation

Common dependencies are:

- PHP >= 7.0
- PHP development headers and tools
- PHP extensions enabled: `pcre`, `SPL`, `standard`

**NOTE:** You will need the PHP development headers. If PHP was manually installed, these should be available by
default. Otherwise, you will need to fetch them from a repository.

### Windows

Prerequisite packages are:

- [Visual C++](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads) >= 14.
  You’ll need `nmake.exe`, `cl.exe` and company
- PHP SDK 2.0 for PHP >= 7.0
- PHP Developer Pack for PHP >= 7.0
- [re2c](http://re2c.org/)

### Linux / macOS

Prerequisite packages are:

- [gcc](https://gcc.gnu.org/) >= 4.4 or [clang](https://clang.llvm.org/) >= 3.0
- [GNU make](https://www.gnu.org/software/make) >= 3.81
- [automake](https://www.gnu.org/software/automake)
- [autoconf](https://www.gnu.org/software/autoconf/autoconf.html)
- [re2c](http://re2c.org/)
- `libpcre3`
- The `build-essential` package when using `gcc` on Ubuntu (and likely in other distributions as well)

**Compilation from the source:**

```shell script
# Clone the repository on your computer.
git clone https://github.com/pdffiller/php-aspekt.git
cd php-aspekt

# Prepare the package.
phpize

# Configure package.
./configure

# Compile the package.
make

# Before the installation, check that it works properly.
make test

# Install Aspekt. This operation may require sudo privileges.
make install
```

**If you have specific PHP versions running:**
```shell script
# Clone the repository on your computer.
git clone https://github.com/pdffiller/php-aspekt.git
cd php-aspekt

# Prepare the package using custom phpize location.
/usr/local/bin/phpize

# Configure package to use proper PHP version.
./configure --with-php-config=/usr/local/bin/php-config

# Compile the package.
make

# Before the installation, check that it works properly.
make test

# Install Aspekt. This operation may require sudo privileges.
make install
```

Finally, add the following line to your php.ini to enable Aspekt:

```ini
[Aspekt]
extension = aspekt.so
aspekt.enable = 1
```

## A small taste

Let’s assume the following class:

```php
namespace Acme;

class AdminService
{
    /**
     * Some stuff only the admin should do.
     */
    public function doAdminStuff1()
    {
        echo 'Calling ', __METHOD__, PHP_EOL;
    }

    /**
     * Yet another stuff only the admin should do.
     */
    public function doAdminStuff2()
    {
        echo 'Calling ', __METHOD__, PHP_EOL;
    }
}
```

Now you want your code to be safe, you don’t want non admin users to be able to call `doAdminStuff1` or the
`doAdminStuff2` methods.

What are your solutions?

- Add some code to check the credentials IN you `AdminService` class. The drawback is that it will pollute your code,
  and your core service will be less readable.
- Let the clients have the responsibility to check the credentials when required. The drawbacks are that you will
  duplicate lots of code client side if you have to call the service from multiple places.
- Add some kind of credential proxy that will check the credentials before calling the actual service. The drawbacks
  are that you will have to write some extra code, adding another class on the top of your services.

Moreover, these solutions tend to increase in complexity while you are adding more cross-cutting concerns like caching
or logging.

That’s where Aspekt comes into action as you will be able to tell PHP to do some extra actions while calling your
`AdminService`'s insecure methods.

So let’s first write the rule needed to check if we can or cannot access the admin services.

```php
namespace Acme;

/**
 * An extremely simple ACL function.
 */
function advice_for_do_admin()
{
    if ((!isset($_SESSION['user_type'])) || ($_SESSION['user_type'] !== 'admin')) {
        throw new \Exception('Sorry, you should be an admin to do this');
    }
}
```

**NOTE:** We check the current PHP session to see if there is something telling us the current user is an admin
(of course we do realize that you may have more complex routines to do that, but we’ll keep this for the example).

Now, let’s use Aspekt to tell PHP to execute this method before any execution of admin methods.

```php
$interceptor = new Aspekt\Interceptor();

$interceptor->addBefore(
    'Acme\\AdminService->doAdmin*()',
    'Acme\\advice_for_do_admin'
);
```

Now, each time you’ll invoke a method of an object of the class `Acme\AdminService`, starting with `doAdmin`, Aspekt
will launch the function `Acme\advice_for_do_admin` before the called method.

That’s it. Simple ain’t it? Now le’s try the examples:

```php
// The session is started here and we added the above examples
// to configure Acme\AdminService and Acme\advice_for_do_admin function.
$service = new Acme\AdminService();

try {
    // Will raise an exception as nothing in the current session
    // tells us we are an administrator.
    $service->doAdminStuff1();
} catch (\Exception $e) {
    echo "You cannot access the service, you're not an admin";
}
```

```php
// The session is started here and we added the above examples
// to configure Acme\AdminService and Acme\advice_for_do_admin function.
$service = new Acme\AdminService();

// Again, this is ugly. Used only for the sake of the example.
$_SESSION['user_type'] = 'admin';

try {
    // Will raise an exception as nothing in the current session
    // tells us we are an administrator.
    $service->doAdminStuff1();
    $service->doAdminStuff2();
} catch (\Exception $e) {
    // Nothing will be caught here, we are an administrator.
}
```

## AOP Vocabulary and Aspekt capabilities

For details on the terminology, refer to AOP Terminology
[Wikipedia page](https://en.wikipedia.org/wiki/Aspect-oriented_programming#Terminology).

### Advice

An advice is a piece of code that can be executed. In our first example the function `Acme\advice_for_do_admin` is an
advice, it _could_ be executed.

In Aspekt an advice can be a trait, a callback, an anonymous function, a static method of a class, a method of a given
object or a closure.

### Join Points

Join Points are places where we can attach advices.

In Aspekt, a join point can be:

- Before any method or a function call
- After any method or a function call
- Around any method or a function call
- During the arousing of an exception of any method or a function
- After any method or a function call, should the method terminate normally or not (triggers an exception or not)


In our first example, we used a _before_ join point.

### Pointcut

Pointcuts are a way to describe whether or not a given join point will trigger the execution of an advice.

In Aspekt pointcuts can be configured with a quite simple and straightforward syntax.

In our first example the pointcut was `Acme\AdminService->doAdmin*()` and was configured to launch the advice before
the execution of the matching methods join points.

## Why or should I use AOP?

AOP is a whole different way of thinking for developing application. It is as different as object oriented programming
can be opposed to procedural programming.

Even if you don’t want to base your future development on this approach, you may find it very useful for debugging
purposes. Imagine a world where you can debug or get information on your code based only on information collected for
a given user, a given context, a given procedure. A world where you can hunt weird and old code execution without even
trying to update multiple and sparse PHP files, but just by adding advices on given conditions.

We are sure that this extension will soon be part of your future development workflow!

## Aspekt in action

### Interceptor

#### `Aspekt\Interceptor::addBefore`

Before kind of advice enables you to:

- Launch advice before the execution of a given function, without interrupting anything
- Launch advice before the execution of a given function, and to interrupt its execution while raising an exception
- Launch advice before the execution of a given function, and to update the targeted function’s arguments

##### Examples

**A simple advice execution**

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;
use Aspekt\Kind;

final class Highlighter
{
    public function highlight($contents)
    {
        echo '<p><pre>' . htmlspecialchars($contents) . '</pre></p>', PHP_EOL;
    }
}

// Creating the advice as a closure.
$advice = function (JoinPoint $jp) {
    echo '<em>Display an alert box:</em>', PHP_EOL;
};

$interceptor = new Interceptor();
$interceptor->addBefore('Highlighter->highlight()', $advice);

$highlighter = new Highlighter();
$highlighter->highlight('alert("Hello! I am an alert box!!");');
```

will output:

```html
<em>Display an alert box:</em>
<p><pre>alert(&quot;Hello! I am an alert box!!&quot;);</pre></p>
```

**An advice that can interrupt the execution of a function (method)**

```php
use Aspekt\Interceptor;

final class Highlighter
{
    public function highlight($file)
    {
        highlight_file($file);
    }
}

/**
 * The advice is a simple function.
 */
function assert_highlight_file_enabled()
{
    $name = 'highlight_file';

    if (!function_exists($name) ||
        (new ReflectionFunction($name))->isDisabled()
    ) {
        throw new \AssertionError(
            sprintf(
                'The "%s" function is not enabled.',
                $name
            )
        );
    }
}

$interceptor = new Interceptor();
$interceptor->addBefore('Highlighter->highlight()', 'assert_highlight_file_enabled');

$highlighter = new Highlighter();

try {
    $highlighter->highlight('example.js');
} catch (Error $error) {
    echo $error->getMessage(), PHP_EOL;
}
```

If you have not enabled `highlight_file` function, the code above will output:

```
The "highlight_file" function is not enabled.
```

**An advice that can update the advice function’s arguments**

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

final class Highlighter
{
    public function print($contents)
    {
        echo $contents, PHP_EOL;
    }
}

/**
 * The advice is a simple function.
 */
function decorate(JoinPoint $jp)
{
    $args = $jp->getArguments();

    if (null !== $args[0]) {
        $args[0] = '<pre>'.  htmlspecialchars($args[0]) . '</pre>';
        $jp->setArguments($args);
    }
}

$interceptor = new Interceptor();
$interceptor->addBefore('Highlighter->print()', 'decorate');

$highlighter = new Highlighter();
$highlighter->print('alert("Hello! I am an alert box!!");');
```

will output:

```html
<pre>alert(&quot;Hello! I am an alert box!!&quot;);</pre>
```

**NOTE:** Since 5.5 PHP has a built-in [OPcache](http://php.net/manual/en/book.opcache.php) enabled by default. That
means, your code may not be loaded and parsed on each request in case if there are no file changes.

If you want to, you can accept in your advice an `Aspekt\JoinPoint` object that will gives your advice more information
on what exactly happened with help of `Aspekt\JoinPoint::getKindOfAdvice`.

#### `Aspekt\Interceptor::addAfter`

After kind of advice enables you to:

- Do stuff after the matched joinpoint
- Replace the return of the adviced function
- Throw an exception in case of an incorrect / unwanted return

The `Aspekt\Interceptor::addAfter` is a superset of:
- `Aspekt\Interceptor::addAfterThrowing`
- `Aspekt\Interceptor::addAfterReturning`

##### Examples

**Doing stuff after the triggered joinpoint.**

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

class Greeting
{
    public function hello(): string
    {
        return 'Hello World!';
    }
}

// Creating the advice as a closure.
$advice = function (JoinPoint $jp) {
    echo 'Hello from the Advice!', PHP_EOL;
};

$interceptor = new Interceptor();
$interceptor->addAfter('Greeting->hello()', $advice);

// Testing.
$greeting = new Greeting();
echo $greeting->hello();
```

will output:

```
Hello from the Advice!
Hello World!
```

Here you can see that the advice is called right after the execution of the triggered joinpoint
(`return 'Hello World!'`), but before anything else can occur (`echo $greeting->hello()`).

**Updating the return value of a triggered joinpoint**

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

class Greeting
{
    public function hello(): string
    {
        return 'Hello World!';
    }
}

// Creating the advice as a closure.
$advice = function (JoinPoint $jp) {
    $return = $jp->getReturnedValue();
    $jp->setReturnedValue(\str_replace('World', 'buddy', $return));
};

$interceptor = new Interceptor();
$interceptor->addAfter('Greeting->hello()', $advice);

// Testing.
$greeting = new Greeting();
echo $greeting->hello();
```

will output:

```
Hello buddy!
```

Fine! Let’s ask Aspekt to raise an exception if a call to `file_get_contents` returns `FALSE` (error). This may not be
a best practice as it could add overhead to native PHP functions, but such a practice can be useful if you’re using an
old PHP library that is not using exceptions as a mean to raise errors.

**Throwing an exception in case of unwanted returned value**

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

// Creating the advice as a closure.
$advice = function (JoinPoint $jp) {
    $args = $jp->getArguments();
    if (false === $jp->getReturnedValue()) {
        throw new \RuntimeException("Cannot read from file '{$args[0]}'");
    }
};

$interceptor = new Interceptor();
$interceptor->addAfter('file_get_contents()', $advice);

try {
    // Testing.
    @file_get_contents('The-file-that-does-not-exist.txt');
} catch (\Exception $e) {
    echo $e->getMessage(), PHP_EOL;
}
```

will output:

```
Cannot read from file 'The-file-that-does-not-exist.txt'
```

#### `Aspekt\Interceptor::addAfterReturning`

`Aspekt\Interceptor::addAfterReturning` links advices that becomes active after the target normally returns from
execution (no exception).

#### `Aspekt\Interceptor::addAfterThrowing`

`Aspekt\Interceptor::addAfterThrowing` links advices that becomes active if the target raise an (uncaught) exception.

#### `Aspekt\Interceptor::addAround`

Around kind of advice enables you to:

- Completely replace the matched joinpoint (including raising exceptions)
- Do stuff around (before and / or) after the joinpoint, including catching exceptions
- Replacing arguments of the matching joinpoint (as of the before kind of advice)
- Replacing the return of the matching joinpoint (as of the after kind of advice)
- And of course a mix of all of the above

##### Examples

**Updating the matching join point without any consideration of the triggered joinpoint**

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

class Greeting
{
    public function hello($name)
    {
        echo "Hello {$name}! ";
    }
}

/**
 * The advice is a static method of a given class.
 */
class Goodbye
{
    public static function advice(JoinPoint $jp)
    {
        echo "Goodbye!";
    }
}

$interceptor = new Interceptor();

// If we used addAfter instead of addAround the output would be Hello John! Goodbye!.
$interceptor->addAround('Greeting->hello()', ['Goodbye', 'advice']);

$greeting = new Greeting();
$greeting->hello('John');
```

will output:

```
Goodbye!
```

**Taking into account some considerations of the matched joinpoint**

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

class Greeting
{
    public function hello($name)
    {
        echo "Hello {$name}! ";
    }
}

/**
 * The advice is a simple method of an object.
 */
class Goodbye
{
    public function advice(JoinPoint $jp)
    {
        $args = $jp->getArguments();
        echo "Goodbye {$args[0]}!";
    }
}

$interceptor = new Interceptor();
$goodbye = new Goodbye();

$interceptor->addAround('Greeting->hello()', [$goodbye, 'advice']);

$greeting = new Greeting();
$greeting->hello('John');
```

will output:

```
Goodbye John!
```

**Around the triggered joinpoint**

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

class MathOperations
{
    public function divide(int $number, int $divideBy)
    {
        if (0 == $divideBy) {
            throw new DivisionByZeroError(
                'Detected an attempt to divide a number by zero.'
            );
        }

        echo $number / $divideBy;
    }
}

// Creating the advice as a closure.
$advice = function (JoinPoint $jp) {
    $args = $jp->getArguments();
    echo " {$args[0]} by {$args[1]} equals [";

    try {
        // Asks for the joinpoint to be processed as normal.
        echo $jp->process();
    } catch (DivisionByZeroError $e) {
        echo 'Infinity';
    }

    echo ']';
};

$interceptor = new Interceptor();
$interceptor->addAround('MathOperations->divide()', $advice);

$services = new MathOperations();
$services->divide(4, 2);
$services->divide(4, 0);
```

will output:

```
4 by 2 equals [2] 4 by 0 equals [Infinity]
```

**An advice that can update the advice function’s arguments**

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

class Greeting
{
    public function hello(string $name = null)
    {
        echo "Hello {$name}! ";
    }
}

/**
 * The advice is a simple function.
 */
function advice(JoinPoint $jp)
{
    $args = $jp->getArguments();

    if (null === $args[0]) {
        $args[0] = 'Ben';
        $jp->setArguments($args);
    }

    $jp->process();
}

$interceptor = new Interceptor();
$interceptor->addAround('Greeting->hello()', 'advice');

$greeting = new Greeting();
$greeting->hello(null);
```

will output:

```
Hello Ben!
```

**Updating the return value of a triggered joinpoint**

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

class Email
{
    public function signature()
    {
        return "Best regards,\nSerghei";
    }
}

// Creating the advice as a closure.
$advice = function (JoinPoint $jp) {
    $jp->process();

    $jp->setReturnedValue(
        \str_replace('Best', 'Kind', $jp->getReturnedValue())
    );
};

$interceptor = new Interceptor();
$interceptor->addAround('Email->signature()', $advice);

$email = new Email();
echo $email->signature();
```

will output:

```
Kind regards,
Serghei
```

### Joinpoint complete reference

An instance of `Aspekt\JoinPoint` will always be passed to your advice. This object contains several information,
such as the pointcut which triggered the joinpoint, the arguments, the returned value (if available), the raised
exception (if available), and will enable you to run the expected method in case you are "around" it.

#### `Aspekt\Joinpoint::getKindOfAdvice`

This will tell in which condition your advice was launched:

| Constant                             | Description                                                                        |
| :---                                 | :---                                                                               |
| `Aspekt\Kind::AROUND`                | Around a given call, may it be function or method access (read / write). |
| `Aspekt\Kind::BEFORE`                | Before a given call, may it be function or method access (read / write). |
| `Aspekt\Kind::AFTER`                 | After a given call, may it be function or method access (read / write).  |
| `Aspekt\Kind::AROUND_METHOD`         | Around a method call (method of an object).                                        |
| `Aspekt\Kind::BEFORE_METHOD`         | Before a method call (method of an object).                                        |
| `Aspekt\Kind::AFTER_METHOD`          | After a method call (method of an object).                                         |
| `Aspekt\Kind::AROUND_FUNCTION`       | Around a function call (not a method call).                                        |
| `Aspekt\Kind::BEFORE_FUNCTION`       | Before a function call (not a method call).                                        |
| `Aspekt\Kind::AFTER_FUNCTION`        | After a function call (not a method call).                                         |

#### `Aspekt\Joinpoint::getArguments`

Will return the triggering method arguments as an indexed array. The resulting array will give values when the
triggering method expected values, and references where the triggering method expected references.

##### Examples

**Get the triggering method arguments as an indexed array.**

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

function callMe($name, &$reference)
{
    printf(
       "Inside the method execution, " .
       "value of name is '%s' and value of reference is '%s'.\n",
       $name,
       $reference
    );
}

$advice = function (JoinPoint $jp) {
    $args = $jp->getArguments();

    // Won’t update the original $name parameter as it is a value.
    $args[0] = 'new name';
    // Will update the original `$reference` parameter as it is a reference.
    $args[1] = 'updated $reference';
};

$interceptor = new Interceptor();
$interceptor->addBefore('callMe()', $advice);

$name = 'name';
$reference = 'reference';

callMe($name, $reference);

printf(
    "After the method execution, " .
    "value of name is '%s' and value of reference is '%s'.\n",
    $name,
    $reference
 );
```

will output:

```
Inside the method execution, value of name is 'name' and value of reference is 'reference'.
After the method execution, value of name is 'name' and value of reference is 'updated $reference'.
```

#### `Aspekt\Joinpoint::setArguments`

Enables you to replace all the arguments the triggering method will receive. Beware that if you want to keep references
you will have to explicitly pass them back to setArguments.


##### Examples

*Get the triggering method arguments as an indexed array*

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

function callMe($name, &$reference, &$reference2)
{
    printf(
       "Inside the method execution\n" .
       "name: '%s', reference: '%s', reference2: '%s'\n\n",
       $name,
       $reference,
       $reference2
    );

    $name = "M - ${name}";
    $reference = "M - ${reference}";
    $reference2 = "M - ${reference2}";
}

$advice = function (JoinPoint $jp) {
    $args = $jp->getArguments();

    $args[0] = "NEW {$args[0]}";
    $args[1] = "NEW {$args[1]}";
    $args[2] = "NEW {$args[2]}";

    $newArgs = [
        $args[0],
        // The reference is kept.
        &$args[1],
        // $newArgs carry a copy of $args[2], the advice won’t be able to update it’s value.
        $args[2],
    ];

   $jp->setArguments($newArgs);
};

$interceptor = new Interceptor();
$interceptor->addBefore('callMe()', $advice);

$name = 'name';
$reference = 'reference';
$reference2 = 'reference2';

callMe($name, $reference, $reference2);

printf(
    "After the method execution\n" .
    "name: '%s', reference: '%s', reference2: '%s'\n\n",
    $name,
    $reference,
    $reference2
 );
```

will output:

```php
Inside the method execution
name: 'NEW name', reference: 'NEW reference', reference2: 'NEW reference2'

After the method execution
name: 'name', reference: 'M - NEW reference', reference2: 'NEW reference2'
```

As a rule of thumb, if you don’t want to mind about references, keep the arguments in the resulting array to update
their values and give the array back back to `Aspekt\Joinpoint::setArguments`:

```php
$advice = function (JoinPoint $jp) {
    $args = $jp->getArguments();

    $args[0] = "NEW {$args[0]}";
    $args[1] = "NEW {$args[1]}";
    $args[2] = "NEW {$args[2]}";

   $jp->setArguments($args);
};
```

will output:

```
Inside the method execution
name: 'NEW name', reference: 'NEW reference', reference2: 'NEW reference2'

After the method execution
name: 'name', reference: 'M - NEW reference', reference2: 'M - NEW reference2'
```

**NOTE:** You should only use `Aspekt\Joinpoint::setArguments` while processing advice of kind before and around,
otherwise it might be confusing to update values of the arguments after the execution of the triggering method.

#### `Aspekt\Joinpoint::getReturnedValue`

Will give you the returned value of the triggering method. It will only be populated in advice of the kind _after_.
In every other kind of advice this method will be null.

If the triggering method returns a reference and you want to update the given reference you will have to explicitly ask
for the reference while calling `Aspekt\Joinpoint::getReturnedValue`.

##### Examples

**Get the returned value of the triggering method**

```php
use Aspekt\Interceptor;
use Aspekt\JoinPoint;

final class Writer
{
    protected $text;

    public function __construct()
    {
        $this->text = 'some text';
    }

    public function &getText(): string
    {
        return $this->text;
    }

    public function printText()
    {
        echo $this->text, PHP_EOL;
    }
}

$advice = function (JoinPoint $jp) {
    // We’re asking explicitly for the reference.
    $result = &$jp->getReturnedValue();
    // Updating the value of the reference.
    $result = 'This is the new text';
};

$interceptor = new Interceptor();
$interceptor->addAfter('Writer->getText()', $advice);

$writer = new Writer();

echo $writer->getText(), PHP_EOL;
$writer->printText();
```

will output:

```
This is the new text
This is the new text
```

If you do the same without the use of reference the value of `Writer::$text` won’t be updated, e.g.:

**Get the returned value of the triggering method without the use of reference**

```php
$advice = function (JoinPoint $jp) {
    // We’re not asking explicitly for the reference.
    $result = $jp->getReturnedValue();
    // The returned value of the triggering method won’t be updated.
    $result = 'This is the new text';
};
```

will output:

```
some text
some text
```

**NOTE:** Of course if the triggering method doesn’t return a reference asking or not for the reference won’t make any
difference.

#### `Aspekt\Joinpoint::setReturnedValue`

Enables you to define the resulting value of the triggering method. This function makes sense for advice of kind after,
around, exception and final.

If you are assigning a returned value to a method that was expected to return a reference the original reference will be
lost and won’t be replaced. To replace the content of an original reference just proceed as explained in the
`Aspekt\Joinpoint::getReturnedValue` documentation.

#### `Aspekt\Joinpoint::process`

This method allows you to explicitly launch the triggering method operation (read or write).

`Aspekt\Joinpoint::process` will only be available for advice of kind around. Any call to process in advice of other
kinds will raise an error with a message like:

```
Aspekt\Joinpoint::process() is only available when the advice was added with Aspekt\Interceptor::addAround()
```

#### `Aspekt\Joinpoint::getPointcut`

Returns the pointcut (as a `string`) that triggered the joinpoint.

#### `Aspekt\Joinpoint::getObject`

Returns the object of the triggered joinpoint. If the joinpoint does not belongs to an object, will return `NULL`.

#### `Aspekt\Joinpoint::getClassName`

Returns the object’s class name of the triggered joinpoint. If the joinpoint does not belongs to a class,
will return `NULL`.

#### `Aspekt\Joinpoint::getMethodName`

Returns the name of the method of the triggered joinpoint.

If the joinpoint was triggered by a function operation it will raise an error.

#### `Aspekt\Joinpoint::getFunctionName`

Returns the name of the function of the triggered joinpoint.

If the joinpoint was triggered by a method operation it will raise an error.

### Pointcuts syntax

#### Basics

Selectors will enables you to describe with a very simple syntax functions, methods that should be considered for
raising the execution of a given advice.

At their simplest form selectors will be given the name of the function itself including its namespace, followed by
parenthesis:

- **`functionName()`**: Will raise advice for every call of the function `functionName`
- **`Acme\Functions\functionName()`**: Will raise advice for every call of the function functionName in the namespace
  `Acme\Functions`, but won’t be triggered if you’re calling only a method named `functionName` in an another namespace

Of course you can specify a method of an object of a given class name by separating the class name and the method name
by `->`:

- **`MyService->myMethod()`**: Will be triggered while calling the method `myMethod` of any instance of the class
  `MyService`
- **`Acme\Services\MyService->myMethod()`**: Will be triggered while calling the method myMethod of any instance of the
  class `MyService` in the namespace `Acme\Services\MyService`

#### Access modifiers

There is a specific keyword you can use to tell Aspekt to consider only methods that are `public`, `protected` or
`private`:

- **`public MyClass->myMethod()`**: Will be triggered while calling _public_ methods named `myMethod`
- **`public | protected MyClass->myMethod()`**: Will be triggered while calling _public_ or  _protected_ methods named
  `myMethod`

#### Wildcards

- **`'*'`**: Match anything inside a name but stops when it encounters a `/`
- **`'**'`**: Match anything, the scope includes the paths (`/`)

#### Simple selectors examples

##### Functions

End the selector with parenthesis `(` `)`:

- **`functionName()`**: Represent any call of a function called `functionName` in the root namespace
- **`namespaceName\\functionName()`**: Represent any call of a function called `functionName` in the `namespaceName`
  namespace

##### Methods

Start your selector with a `ClassName` (interfaces or traits will also work, inheritance is taken into account):

- **`ClassName->methodName()`**: Represent any call of a method called `methodName` from an instance (or not) of a class
  `ClassName` in the root namespace
- **`namespaceName\\ClassName->methodName()`**: Represents any call of a method called `methodName` from an instance
  (or not) of a class `ClassName` located in the namespace `namespaceName`

**Note:** You can use both `::` and `->` as a separator for class methods
(e.g. `Class->method()` equals `Class::method()`).

#### Selectors using wildcards examples

- **`startingFunctionName*()`**: Represent any call of a function who’s name starts with `startingFunctionName` in the
  root namespace
- **`*endingFunctionName()`**: Represent any call of a function who’s name ends with `endingFunctionName` in the root
  namespace
- **`*\\functionName()`**: Represent any call of a function called `functionName` in any single level namespace
- **`*\\\*\\functionName()`**: Represent any call of a function called `functionName` in any two level namespace
- **`StartingClassName*->methodName()`**: Represent any call of a method called `methodName` from an instance (or not)
  of a class who's name start with `StartingClassName` in the root namespace
- **`*EndingClassName->methodName()`**: Represent any call of a method called `methodName` from an instance (or not) of
  a class who's name end with `EndingClassName` in the root namespace

#### Selectors using super wildcards examples

- **`*\\::admin*()`**: Represents every call of a method _starting_ by `admin` of any class in any namespace
- **`*\\()`**: Represents every call of any method in any namespace

## FAQ

- **Q:** In which order will my pointcuts / advice will be resolved?
- **A:** The advices are executed in the registration order.

---

- **Q:** Is it possible to enable / disable the execution of aspects during runtime?
- **A:** Yes it is. You can use the ini directive aspekt.enable to do so:
  ```php
  use Aspekt\Interceptor;
  use Aspekt\JoinPoint;

  // Aspekt is enabled here.
  ini_set('aspekt.enable', '1');

  function say_hello() {
     echo "Hello! We're in the ", __FUNCTION__, '. ', PHP_EOL;
  }

  // Creating the advice as a closure.
  $advice = function (JoinPoint $jp) {
     echo "We're in the advice. ", PHP_EOL;
  };

  $interceptor = new Interceptor();
  $interceptor\->addAfter('say_hello()', $advice);

  say_hello();

  // Aspekt is now disabled.
  ini_set('aspekt.enable', '0');

  say_hello();

  // But you can still register new aspects.
  $interceptor\->addAfter('s*()', $advice);

  say_hello();

  // Aspekt is now enabled.
  ini_set('aspekt.enable', '1');

  say_hello();
  ```

  will output:
  ```
  Hello! We're in the say_hello.
  We're in the advice.
  Hello! We're in the say_hello.
  Hello! We're in the say_hello.
  Hello! We're in the say_hello.
  We're in the advice.
  We're in the advice.
  ```
## License

[airSlate](https://airslate.com/) and any contributors to this project each grants you a license, under its respective
copyrights, to the Aspekt and other content in this repository under the MIT License, see the LICENSE file for more
information. <br>

This project is fully reworked and separately developed version of
[Salleyron Julien's AOP PHP extension](https://github.com/AOP-PHP/AOP). The copyright of previous versions of this work
belongs to Salleyron Julien. For more see the
[LICENSE-PHP](https://github.com/pdffiller/php-aspekt/blob/master/LICENSE-PHP) license.

`SPDX-License-Identifier: MIT`
