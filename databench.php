<?php
$GLOBALS['plot_db'] = db_connect();

$feed = isset($_REQUEST['feed']);
$dump = isset($_REQUEST['dump']);

$offset = $_REQUEST['offset'];
$limit = $_REQUEST['limit'];
$ip = $_REQUEST['ip'];
$last = $_REQUEST['last'];


if ($ip==true) {
	die($_SERVER['SERVER_ADDR']);
}

if ($feed == true) {
  	$json = array();
  	
  	$object = json_decode(file_get_contents('php://input'));
  	//echo file_get_contents('php://input');
  	$humidity = $object->{'humidity'};
  	$soundVolume = $object->{'soundVolume'};
	$peopleCount = 0;//$object->{'peopleCount'};
	$windSpeed = $object->{'windSpeed'};
	$temperature = $object->{'temperature'};
	$raining = $object->{'raining'};
	$places = $object->{'places'};// masque binaire
	
	$placesString = sprintf("%08s",decbin($places));
	
	$lastC = 0;
	$cCount = 0;
	
	for ($i = 0; $i<8; $i++) {
		$c = $placesString[$i];
		//echo $c;
		if ($c == "1" && $lastC == "0") { // rising
			$peopleCount++;
		} else if ($lastC == "1" && $c == "1") { // 
			$cCount++;
          	if ($cCount >=3) {
            	$peopleCount++;
              	$cCount = 0;
          	}
        } 
      	$lastC = $c;
	}
	
	
  	$request = "INSERT INTO benchData ('humidity', 'soundVolume', 'peopleCount', 'windSpeed', 'temperature','raining','places') VALUES ($humidity, $soundVolume, $peopleCount, $windSpeed, $temperature, $raining, $places);";
  	//error_log($request);
	//echo $request;
  	if ($GLOBALS['plot_db']->exec($request)) {
    	$json['error'] = "ok";
  	} else {
      	$json['error'] = "error";
  	}
	// echo(file_get_contents('php://input'));
// 	echo($request);
   	die(json_encode($json));
}

if ($dump == true) {
	
	// construction de la requette sql
	$request = "SELECT * FROM benchData";
	
	if ($limit != null) {
		$request .= " LIMIT ".$limit;
		
		if ($offset != null) {
			$request .= " OFFSET ".$offset;
		}
	} else if ($last != null) {
		$request .= "  ORDER BY id DESC LIMIT 1";
	}
	
  	$results = $GLOBALS['plot_db']->query($request);
	$dataBench = array();

 	while($res = $results->fetchArray(SQLITE3_ASSOC)){
		$datas = array();

		$datas['id'] = 				$res['id'];
		$datas['humidity'] =		$res['humidity'];
		$datas['peopleCount'] =		$res['peopleCount'];
		$datas['windSpeed'] =		$res['windSpeed'];
		$datas['temperature'] =		$res['temperature'];
		$datas['raining'] =		$res['raining'];
		$datas['places'] =		$res['places'];
		$datas['date'] =		$res['date'];
		
		$dataBench["data"][] = $datas;
	}
  	$dataBench['error'] = "ok";
	$GLOBALS['plot_db']->close();
	die(json_encode($dataBench));
}

function db_connect() {

  class DB extends SQLite3 {
    function __construct( $file ) {
      $this->open( $file,SQLITE3_OPEN_READWRITE | SQLITE3_OPEN_CREATE);
    }
  }

  $adb = new DB('databench.db');
  if ($adb->lastErrorMsg() != 'not an error') {
    error_log("Database Error: " . $adb->lastErrorMsg()."\n",3);
  }
  return $adb;
}

?>