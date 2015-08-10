<?php

namespace Arduino\DAO;

use Arduino\Domain\Arduino;

class ArduinoDAO extends DAO
{
    /**
     * Return a list of all arduinos, sorted by date.
     *
     * @return array A list of all arduinos.
     */
    public function findAll() {
        $sql = "select * from arduino order by numero asc";
        $result = $this->getDb()->fetchAll($sql);

        // Convert query result to an array of domain objects
        $arduinos = array();
        foreach ($result as $row) {
            $arduinoId = $row['id'];
            $arduinos[$arduinoId] = $this->buildDomainObject($row);
        }
        return $arduinos;
    }

    public function findById($id){
        $sql = "select * from arduino where id = $id";

    }


    /**
     * Creates an Arduino object based on a DB row.
     *
     * @param array $row The DB row containing Arduino data.
     * @return \MicroCMS\Domain\Arduino
     */
    protected function buildDomainObject($row) {
        $arduino = new Arduino();
        $arduino->setId($row['id']);
        $arduino->setNumero($row['numero']);
        $arduino->setCommentaire($row['commentaire']);
        return $arduino;
    }

}
