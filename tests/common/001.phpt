--TEST--
Print extension info
--SKIPIF--
<?php include __DIR__ . '/../skipif.inc'; ?>
--FILE--
<?php
$ext = new ReflectionExtension('aspekt');
$ext->info();
?>
--EXPECTF--
aspekt


A modern aspect-oriented PHP extension with rich features for the new level of software development.
aspekt support => enabled
Author => Serghei Iakovlev <%s@%s>
Version => %d.%d.%d
Build Date => %s %d 20%d %d:%d:%d

Directive => Local Value => Master Value
aspekt.enable => On => On
