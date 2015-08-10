<?php

// Home page
$app->get('/', "Arduino\Controller\HomeController::indexAction");
$app->get('/{idArduino}', "Arduino\Controller\HomeController::indexAction");
$app->get('/{idArduino}/{display}', "Arduino\Controller\HomeController::indexAction");

// Add a new temperature
$app->match('/temperature/add/{idArduino}/{valeur}', "Arduino\Controller\HomeController::addTemperatureAction");
$app->match('/temperature/add/{idArduino}/{valeur}/{time}', "Arduino\Controller\HomeController::addTemperatureAction");
