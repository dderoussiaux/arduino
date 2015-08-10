<?php

// Doctrine (db)
$app['db.options'] = array(
    'driver'   => 'pdo_mysql',
    'charset'  => 'utf8',
    'host'     => 'localhost',
    'port'     => '3306',
    'dbname'   => 'arduino',
    'user'     => 'root',
    'password' => 'password',
);

// define log level
$app['monolog.level'] = 'WARNING';
