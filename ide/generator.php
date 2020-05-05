<?php
/*
 * This file is part of the Aspekt.
 *
 * (c) airSlate Inc. <support@airslate.com>
 *
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 */

function generateFunction(\ReflectionFunction $function)
{
    $template = 'function ' . $function->getShortName() . '(';

    foreach ($function->getParameters() as $i => $parameter) {
        if ($i >= 1) {
            $template .= ', ';
        }

        if ($typehint = $parameter->getClass()) {
            $template .= $typehint->getName() . ' ';
        } elseif ($typehint = $parameter->getType()) {
            $template .= $typehint . ' ';
        }

        $template .= '$' . $parameter->getName();
        if ($parameter->isDefaultValueAvailable()) {
            $template .= ' = ' . $parameter->getDefaultValue();
        }
    }

    $template .= ') {}';

    return $template;
}

function generatMethod(\ReflectionMethod $method)
{
    $template = '';

    if ($method->isPrivate()) {
        return $template;
    }

    if ($method->getDocComment()) {
        $template .= '    ' . $method->getDocComment() . PHP_EOL;
   }

   $template .= sprintf(
       '    %s function ',
       implode(' ', \Reflection::getModifierNames($method->getModifiers()))
    );

   if ($method->returnsReference()) {
       $template .= '&';
   }

   $template .= $method->getName() . '(';
   foreach ($method->getParameters() as $i => $parameter) {
       if ($i >= 1) {
           $template .= ', ';
       }

       if ($typehint = $parameter->getClass()) {
           $template .= $typehint->getName() . ' ';
       } elseif ($typehint = $parameter->getType()) {
           $template .= $typehint . ' ';
       }

       $template .= '$' . $parameter->getName();
       if ($parameter->isDefaultValueAvailable()) {
           $template .= ' = ' . $parameter->getDefaultValue();
       }
   }

   $template .= ') {}';

    return $template;
}

function generateProperty(\ReflectionProperty $property)
{
    $template = '';
    $template .= '    ' . '$' . $property->getName() . ';';

    return $template;
}

function generateConstant(\ReflectionClassConstant $constant)
{
    $template = sprintf('    const %s = %d;', $constant->getName(), $constant->getValue());

    return $template;
}

function generateClass(\ReflectionClass $class)
{
    $template = '';

    $definition = sprintf(
        '%s class %s',
        implode(' ', \Reflection::getModifierNames($class->getModifiers())),
        $class->getShortName()
     );

     $template .= trim($definition);

    if ($parent = $class->getParentClass()) {
        $template .= ' extends \\' . $parent->getName();
    }

    $template .= PHP_EOL . '{' . PHP_EOL;

    foreach ($class->getProperties() as $property) {
        $template .= generateProperty($property) . PHP_EOL;
    }

    foreach ($class->getReflectionConstants() as $constant) {
        $template .= generateConstant($constant) . PHP_EOL;
    }

    foreach ($class->getMethods() as $method) {
        $template .= generatMethod($method) . PHP_EOL;
    }

    $template .= '}' . PHP_EOL;

    return $template;
}

function generateExtension(\ReflectionExtension $extensions)
{
    $code = '';

    $classes = $extensions->getClasses();
    $functions = $extensions->getFunctions();

    foreach ($classes as $class) {
        $code .= generateClass($class) . PHP_EOL;
    }

    foreach ($functions as $function) {
        $code .= generateFunction($function) . PHP_EOL;
    }

    return $code;
}

$head =<<<'HEAD'
/*
 * This file is part of the Aspekt.
 *
 * (c) airSlate Inc. <support@airslate.com>
 *
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 */
HEAD;

$code = '<?php' . PHP_EOL;
$code .= $head;
$code .= PHP_EOL . PHP_EOL;
$code .= "namespace Aspekt;";
$code .= PHP_EOL . PHP_EOL;
$code .= generateExtension(new \ReflectionExtension('aspekt'));

echo $code;
