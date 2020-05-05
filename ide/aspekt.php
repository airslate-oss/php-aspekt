<?php
/*
 * This file is part of the Aspekt.
 *
 * (c) airSlate Inc. <support@airslate.com>
 *
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 */

namespace Aspekt;

class JoinPoint
{
    public function getArguments() {}
    public function setArguments(array $arguments) {}
    public function getException() {}
    public function getPointcut() {}
    public function process() {}
    public function getKindOfAdvice() {}
    public function getObject() {}
    public function &getReturnedValue() {}
    public function setReturnedValue($value) {}
    public function getClassName() {}
    public function getMethodName() {}
    public function getFunctionName() {}
    public function &getAssignedValue() {}
    public function setAssignedValue($value) {}
    public function getPropertyName() {}
    public function getPropertyValue() {}
}

class Interceptor
{
    public function addAround($pointcut, $advice) {}
    public function addBefore($pointcut, $advice) {}
    public function addAfter($pointcut, $advice) {}
    public function addAfterReturning($pointcut, $advice) {}
    public function addAfterThrowing($pointcut, $advice) {}
}

final class Kind
{
    const AROUND = 1;
    const BEFORE = 2;
    const AFTER = 4;
    const READ = 8;
    const WRITE = 16;
    const PROPERTY = 32;
    const METHOD = 64;
    const FUNCTION = 128;
    const AROUND_READ_PROPERTY = 41;
    const BEFORE_READ_PROPERTY = 42;
    const AFTER_READ_PROPERTY = 44;
    const AROUND_WRITE_PROPERTY = 49;
    const BEFORE_WRITE_PROPERTY = 50;
    const AFTER_WRITE_PROPERTY = 52;
    const AROUND_METHOD = 65;
    const BEFORE_METHOD = 66;
    const AFTER_METHOD = 68;
    const AROUND_FUNCTION = 129;
    const BEFORE_FUNCTION = 130;
    const AFTER_FUNCTION = 132;
}

