CREATE TABLE `arduino` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `numero` varchar(30) NOT NULL COMMENT 'code du arduino',
  `commentaire` varchar(1000) DEFAULT NULL COMMENT 'description du arduino',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;


CREATE TABLE `temperature` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `id_arduino` int(11) NOT NULL DEFAULT '0' COMMENT 'lien du arduino',
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'heure acquisition',
  `valeur` varchar(10) NOT NULL COMMENT 'temperature',
  PRIMARY KEY (`id`),
  KEY `FK_Arduino` (`id_arduino`),
  CONSTRAINT `FK_Arduino` FOREIGN KEY (`id_arduino`) REFERENCES `arduino` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=latin1;
