<?php

namespace Arduino\Controller;

use Silex\Application;
use Symfony\Component\HttpFoundation\Request;
use Arduino\Domain\Temperature;

class HomeController {

    /**
     * Home page controller.
     *
     * @param Application $app Silex application
     */
    public function indexAction(Application $app, $idArduino = 0, $display = 'day') {
        $arduinos = $app['dao.arduino']->findAll();

        $json = null;

        if ($idArduino != 0){
            $temperatures = $app['dao.temperature']->findAllForArduino($idArduino, $display);
            $labels = Array();
            $serie = Array();

            foreach($temperatures as $temp){
                $labels[] = $temp->getTime();
                $serie[] = $temp->getValeur();
            }

            $data['labels'] = $labels;
            $data['series'] = Array($serie);

            $json = json_encode($data);
        }

        return $app['twig']->render('index.html.twig', array('idArduino' => $idArduino, 'display' => $display, 'json' => $json, 'arduinos' => $arduinos));
    }

    /**
     * insert data
     *
     * @param Request $request Incoming request
     * @param Application $app Silex application
     * @param integer $idArduino Arduinon id
     * @param string $valeur valeur
     * @param timestamp $time date
     *
     */
    public function addTemperatureAction(Request $request, Application $app, $idArduino, $valeur=-99, $time=null){
        if (!$time){
            $time = date("Y-m-d H:i:s");
        }
        if ($valeur == -99){
            return $app->json('No value', 400);  // 400 = Bad Request
        }

        // Build and save the new temperature
        $temperature = new Temperature();
        $temperature->setIdArduino($idArduino);
        $temperature->setTime($time);
        $temperature->setValeur($valeur);
        $app['dao.temperature']->save($temperature);

        return $app->json(true, 201);  // 201 = Created
    }

}
