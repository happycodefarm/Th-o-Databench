// parser

function generate() {
	document.getElementById("dump").innerHTML = "";
	var conditionText = "";
	var conditions;
	
	readTextFile("conditions.txt").then(function(response) {
		//console.log(response);
		conditionText = response;
		conditionText = conditionText.replace(/^\s*[\r\n]/gm, "");
		conditions = conditionText.split('\n');
		console.log(conditions);
	}, function(Error) {
   		console.log(Error);
	});
	
	getData().then(function(response) {
		
		var data = response.data[response.data.length-1];
		
		var d = Date.parse(data.date);
		var date = new Date(d);
		data.date = date;
		data.hour = data.date.getHours();
		data.minute = data.date.getMinutes();
		data.day = data.date.getDay();
		data.month = data.date.getMonth();
		data.year = data.date.getFullYear();
		
		console.log(data);
		for (var i = 0; i < conditions.length; i++) {
			var t = conditions[i];//"[peopleCount==2] le {bleu|rouge|vert}, est une chouette couleur.";

			var rVar = new RegExp(/\[([^\]]+)]/g);
			var rRand = new RegExp(/{([^}]+)}/g);
			var rFormat = new RegExp(/#([^#]+)#/g);
			var v = t.replace(rVar,variables); 
			var r = v.replace(rRand,randoms)+"#"; 
			var f = r.replace(rFormat,format);
			var d = f.replace(/\$/g,"data.");
		
			//console.log(data);
			console.log(d);
			var result = null;
			
			if (eval(d) === undefined) continue;
			
			if (result != null) {
				document.getElementById("dump").innerHTML = document.getElementById("dump").innerHTML + result;
			}
			console.log(i + ":\t " + result);
		}
	
	}, function(Error) {
   		console.log("get data error: "+Error);
  	});
}

function variables(a,b) {
	return("if("+b+") result=#"); 
}

function randoms(a,b) {
	var tt = b.split('|');
	var r = Math.random()* tt.length;
	return tt[Math.floor(r)]; 
}

function format(a,b) {
	return ("\""+b+"\";");
}


function getData() {
	return new Promise(function(resolve, reject) {
		var xmlHttp = null;

		xmlHttp = new XMLHttpRequest();
		xmlHttp.open("GET",'databench.php?dump=true&last=true', true);
		xmlHttp.setRequestHeader("Content-type", "application/json"); // json header
		xmlHttp.setRequestHeader("If-Modified-Since", "Sat, 1 Jan 2000 00:00:00 GMT"); // IE Cache Hack
		xmlHttp.setRequestHeader("Cache-Control", "no-cache"); // idem
		
		
		xmlHttp.onerror = function() {
      		consoleText.content = "Network error";
      		consoleText.fillColor = 'red';
      		// window.setTimeout(function(){
			// consoleText.content = "Idle."
			// consoleText.fillColor = 'grey'
			// }, 2000)
      		reject(Error('There was a network error.'));
    	}
    	
    	xmlHttp.onload = function() {
      		if (xmlHttp.status === 200) {
      			var json = null;

				try {
					json = JSON.parse(xmlHttp.responseText);
				} catch (err) {
					console.log("error json parse "+ err);
					console.log(xmlHttp.responseText);
					document.getElementById("dump").innerHTML = "JSON parse Error"
					reject(Error('JSON parse error.'));
				}
				
				if (json.error == "ok") {
					console.log("ok")
					resolve(json);
				} else {
					console.log("bad")
					reject(Error('bad bad bad.'))
				}
      		} else {
      			reject(Error('Network error:' + xmlHttp.statusText))
      		}
      	}
      	
      	xmlHttp.send();
	});
}
// end parser

function sendData() {
	// recuperation des donnés de la page html
	var humidity = document.getElementById('humidity').value;
	var soundVolume = document.getElementById('soundVolume').value;
	var peopleCount = document.getElementById('peopleCount').value;
	var windSpeed= document.getElementById('windSpeed').value;
	var temperature = document.getElementById('temperature').value;
	var raining = document.getElementById('raining').value;
	var places = document.getElementById('places').value;

	var JSONMarker = {
		humidity:humidity,
		soundVolume:soundVolume,
		peopleCount:peopleCount,
		windSpeed:windSpeed,
		temperature:temperature,
		raining:raining,
		places:places
	};

	// console.log("Sending path: " + JSONMarker.paths)
	// console.log("Sending group: " + JSONMarker.group)

	var JSONString = JSON.stringify(JSONMarker);
	console.log("Sending : " + JSONString);

	var xmlHttp = null;

	xmlHttp = new XMLHttpRequest();
	xmlHttp.open("POST",'databench.php?feed=true', true);
	xmlHttp.setRequestHeader("Content-type", "application/json"); // json header
	xmlHttp.setRequestHeader("If-Modified-Since", "Sat, 1 Jan 2000 00:00:00 GMT"); // IE Cache Hack
	xmlHttp.setRequestHeader("Cache-Control", "no-cache"); // idem
	xmlHttp.send(JSONString);

	xmlHttp.onreadystatechange=function() {
		if(xmlHttp.readyState == 4){
			var json = null;

			try {
				json = JSON.parse(xmlHttp.responseText);
			} catch (err) {
				console.log("error json parse "+ err);
				console.log(xmlHttp.responseText);
				return;
			}

			console.log(json);
			
			if (json.error == "ok") {
				console.log("ok");
				
			} else {
				console.log("bad");
				
			}
		}
	}

}

function dumpData() {
	var xmlHttp = null;

	xmlHttp = new XMLHttpRequest();
	xmlHttp.open("GET",'databench.php?dump=true', true);
	xmlHttp.setRequestHeader("Content-type", "application/json"); // json header
	xmlHttp.setRequestHeader("If-Modified-Since", "Sat, 1 Jan 2000 00:00:00 GMT"); // IE Cache Hack
	xmlHttp.setRequestHeader("Cache-Control", "no-cache"); // idem
	xmlHttp.send();

	xmlHttp.onreadystatechange=function() {
		if(xmlHttp.readyState == 4){
			var json = null;

			try {
				json = JSON.parse(xmlHttp.responseText);
			} catch (err) {
				console.log("error json parse "+ err);
				console.log(xmlHttp.responseText);
				document.getElementById("dump").innerHTML = "JSON parse Error";
				
				return;
			}

			console.log(json);
			
			if (json.error == "ok") {
				console.log("ok");
				document.getElementById("dump").innerHTML = xmlHttp.responseText;
			
			} else {
				console.log("bad");
				document.getElementById("dump").innerHTML = "Error";
			
			}
		}
	}
}

function readTextFile(file) {
	return new Promise(function(resolve, reject) {
		var rawFile = new XMLHttpRequest();
		rawFile.open("GET", file, false);
		
		
		rawFile.onerror = function() {
      		reject(Error('readTextFile network error:' + rawFile.statusText));
    	}
    	
    	rawFile.onload = function() {
      		if (rawFile.status === 200) {
				resolve(rawFile.responseText);
			} else {
				console.log("bad");
				reject(Error('readTextFile network error:' + rawFile.statusText));
			}
      	}
		rawFile.send(null);
	 });
}