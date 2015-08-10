<?php

namespace Arduino\DAO;

use Arduino\Domain\Temperature;

class TemperatureDAO extends DAO
{
    /**
     * Return a list of all temperatures, sorted by date.
     *
     * @return array A list of all temperatures.
     */
    public function findAll() {
        $sql = "select * from temperature order by time asc";
        $result = $this->getDb()->fetchAll($sql);

        // Convert query result to an array of domain objects
        $temperatures = array();
        foreach ($result as $row) {
            $tempId = $row['id'];
            $temperatures[$tempId] = $this->buildDomainObject($row);
        }
        return $temperatures;
    }

    public function findAllForArduino($idArduino, $display){
        if ($display == 'day'){
            $debut = time() - 86400; // (-24h)
        }elseif($display == 'month'){
            $debut = time() - 2419200; // (-4semaines)
        }

        $sql = "select * from temperature where id_arduino = $idArduino order by time asc";
        $result = $this->getDb()->fetchAll($sql);
        //var_dump($sql);die();

        // Convert query result to an array of domain objects
        $temperatures = array();
        foreach ($result as $row) {
            $tempId = $row['id'];
            $temperatures[$tempId] = $this->buildDomainObject($row);
        }
        return $temperatures;
    }


    /**
     * Creates an Temperature object based on a DB row.
     *
     * @param array $row The DB row containing Temperature data.
     * @return \MicroCMS\Domain\Temperature
     */
    protected function buildDomainObject($row) {
        $temp = new Temperature();
        $temp->setId($row['id']);
        $temp->setIdArduino($row['id_arduino']);
        $temp->setTime($row['time']);
        $temp->setValeur($row['valeur']);
        return $temp;
    }


    /**
     * Returns an temperature matching the supplied id.
     *
     * @param integer $id
     *
     * @return \Arduino\Domain\Temperature|throws an exception if no matching temperature is found
     */
    public function find($id) {
        $sql = "select * from temperature where id=?";
        $row = $this->getDb()->fetchAssoc($sql, array($id));

        if ($row)
            return $this->buildDomainObject($row);
        else
            throw new \Exception("No temperature matching id " . $id);
    }

    /**
     * Saves an temperature into the database.
     *
     * @param \Arduino\Domain\Temperature $temperature The temperature to save
     */
    public function save(Temperature $temperature) {
        $tempData = array(
            'id_arduino' => $temperature->getIdArduino(),
            'time' => $temperature->getTime(),
            'valeur' => $temperature->getValeur(),
            );

        if ($temperature->getId()) {
            // The temperature has already been saved : update it
            $this->getDb()->update('temperature', $tempData, array('id' => $temperature->getId()));
        } else {
            // The temperature has never been saved : insert it
            $this->getDb()->insert('temperature', $tempData);
            // Get the id of the newly created temperature and set it on the entity.
            $id = $this->getDb()->lastInsertId();
            $temperature->setId($id);
        }
    }

    /**
     * Removes an temperature from the database.
     *
     * @param integer $id The temperature id.
     */
    public function delete($id) {
        // Delete the temperature
        $this->getDb()->delete('temperature', array('id' => $id));
    }


}
