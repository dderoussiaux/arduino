<?php

namespace Arduino\Domain;

class Arduino
{
    /**
     * id
     *
     * @var integer
     */
    private $id;

    /**
     * numero
     *
     * @var string
     */
    private $numero;

    /**
     * commentaire
     *
     * @var string
     */
    private $commentaire;


    public function getId() {
        return $this->id;
    }

    public function setId($id) {
        $this->id = $id;
    }

    public function getNumero(){
        return $this->numero;
    }

    public function setNumero($numero){
        $this->numero = $numero;
    }

    public function getCommentaire(){
        return $this->commentaire;
    }

    public function setCommentaire($commentaire){
        $this->commentaire = $commentaire;
    }

}
