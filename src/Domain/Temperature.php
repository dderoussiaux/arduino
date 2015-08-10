<?php

namespace Arduino\Domain;

class Temperature
{
    /**
     * id
     *
     * @var integer
     */
    private $id;

    /**
     * idArduino
     *
     * @var \Arduino\Domain\Arduino
     */
    private $idArduino;

    /**
     * time
     *
     * @var timestamp
     */
    private $time;

    /**
     * valeur
     *
     * @var float
     */
    private $valeur;


    public function getId() {
        return $this->id;
    }

    public function setId($id) {
        $this->id = $id;
    }

    public function getIdArduino(){
        return $this->idArduino;
    }

    public function setIdArduino($idArduino){
        $this->idArduino = $idArduino;
    }

    public function getTime(){
        return $this->time;
    }

    public function setTime($time){
        $this->time = $time;
    }

    public function getValeur(){
        return $this->valeur;
    }

    public function setValeur($valeur){
        $this->valeur = $valeur;
    }

}
