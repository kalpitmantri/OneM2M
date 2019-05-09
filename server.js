  // Version 1.0

var express = require('express');
var bodyParser = require('body-parser');
var request = require('request');
var app = express();

app.set("view engine","ejs");
app.use(express.static("public"));
app.use(bodyParser.urlencoded({extended:true}));

var monitorIP = "127.0.0.1";
var monitorPort = 9999;

const port = 9999 || process.env.PORT;

var sensorThreshold = 0;

var monitorName = "SmartApp";
var sensors = ['TempIN','TempOUT','Dust','LDR_IN','LDR_OUT'];
var actuatorToTrigger = "LedActuator";
var isLedOn = false;

app.use(bodyParser.json());

// start http server
app.listen(port, function () {
	console.log("Listening on: " + monitorIP + ":" + monitorPort);
});

var appAc = 3,appLight = 3,appWindow = 3,appCurtain = 3;

// handle received http messages
app.post('/LDR_OUT', function (req, res) {
	var  vrq  = req.body["m2m:sgn"]["m2m:vrq"];
	if  (!vrq) {
		var sensorValue = req.body["m2m:sgn"]["m2m:nev"]["m2m:rep"]["m2m:cin"].con;
		console.log("LDR_OUT value : " + sensorValue);
		if(appCurtain == 1){
			console.log("Forced Curtain Open => Switching on LED");
			createCIN("[switchOn]","ledCurtain");
			// console.log("-------------------------");
		}
		else if(appCurtain == 0){
			console.log("Forced Curtain Close => Switching off LED");
			createCIN("[switchOff]","ledCurtain");
			// console.log("-------------------------");
		}
		else if(appCurtain == 2){
			console.log("Smartly Controlling the Curtain");
			if(sensorValue == 1){
				console.log("HighLightOutsite => Opening the curtain => Switching on LED")
				createCIN("[switchOn]","ledCurtain");
				// console.log("-------------------------");
			}
			else{
				console.log("LowLightOutside => Closing the Curtain => Switching off LED")
				createCIN("[switchOff]","ledCurtain");
				// console.log("-------------------------");
			}
		}
		else{
			console.log("Nothing to do");
		} 
	}
	else{
		console.log("XYZ");
	}
	res.sendStatus(200);	
});

app.post('/LDR_IN', function (req, res) {
	console.log("Inside /LDR_IN");
	var  vrq  = req.body["m2m:sgn"]["m2m:vrq"];
	if  (!vrq) {
		var sensorValue = req.body["m2m:sgn"]["m2m:nev"]["m2m:rep"]["m2m:cin"].con;
		console.log("LDR_IN value : " + sensorValue);

		if(appLight == 1){
			console.log("Forced Light ON => Switching on LED");
			createCIN("[switchOn]","ledLight");
			// console.log("-------------------------");
		}
		else if(appLight == 0){
			console.log("Forced Light Off => Switching off LED");
			createCIN("[switchOff]","ledLight");
			// console.log("-------------------------");
		}
		else if(appLight == 2){
			console.log("Smartly Controlling the Light");
			if(sensorValue == 1){
				console.log("High light in room => Turn Off Lights => Switching off LED")
				createCIN("[switchOff]","ledLight");
				// console.log("-------------------------");
			}
			else{
				console.log("Low Light in room => Turn On Lights => Switching on LED")
				createCIN("[switchOn]","ledLight");
				// console.log("-------------------------");
			}
		}
		else{
			console.log("Nothing to do");
		} 
	}
	else{
		console.log("XYZ");
	}
	res.sendStatus(200);	
});

var count = 0;
var map1 = new Map();

app.post('/TempIN', function (req, res) {
	console.log("Inside /TempIN");
	var  vrq  = req.body["m2m:sgn"]["m2m:vrq"];
	if  (!vrq) {
		var sensorValue = req.body["m2m:sgn"]["m2m:nev"]["m2m:rep"]["m2m:cin"].con;
		console.log("TempIN value : " + sensorValue);
		if(count < 3){
			count++;
			map1.set("TempIN",sensorValue);
		}

		if(count == 3){
			count = 0;

			if(appAc == 1){
				console.log("Forced AC ON => Switching on AC");
				createCIN("[switchOn]","ledAC");
				// console.log("-------------------------");
				map1.clear();
			}
			else if(appAc == 0){
				console.log("Forced AC Off => Switching Off AC");
				createCIN("[switchOff]","ledAC");
				// console.log("-------------------------");
				map1.clear();
			}
			else if(appAc == 2){
				smartCooling(map1);
				map1.clear();
			}
			else{
				console.log("Nothing to do!!");
				// console.log("------------------");
				map1.clear();
			}
		}
	}
	else{
		console.log("XYZ");
	}
	res.sendStatus(200);	
});

app.post('/TempOUT', function (req, res) {
	console.log("Inside /TempOUT");
	var  vrq  = req.body["m2m:sgn"]["m2m:vrq"];
	if  (!vrq) {
		var sensorValue = req.body["m2m:sgn"]["m2m:nev"]["m2m:rep"]["m2m:cin"].con;
		console.log("TempOUT value : " + sensorValue);

		if(count<3){
			count++;
			map1.set("TempOUT",sensorValue);
		}

		if(count == 3){
			count = 0;

			if(appAc == 1){
				console.log("Forced AC ON => Switching on AC");
				createCIN("[switchOn]","ledAC");
				// console.log("-------------------------");
				map1.clear();
			}
			else if(appAc == 0){
				console.log("Forced AC Off => Switching Off AC");
				createCIN("[switchOff]","ledAC");
				// console.log("-------------------------");
				map1.clear();
			}
			else if(appAc == 2){
				smartCooling(map1);
				map1.clear();
			}
			else{
				console.log("Nothing to do!!");
				// console.log("------------------");
				map1.clear();
			}
		}
	}
	else{
		console.log("XYZ");
	}
	res.sendStatus(200);	
});

app.post('/Dust', function (req, res) {
	console.log("Inside /Dust");
	var  vrq  = req.body["m2m:sgn"]["m2m:vrq"];
	if  (!vrq) {
		var sensorValue = req.body["m2m:sgn"]["m2m:nev"]["m2m:rep"]["m2m:cin"].con;
		console.log("Dust value : " + sensorValue);
		if(count<3){
			count++;
			map1.set("Dust",sensorValue);
		}

		if(count == 3){
			count = 0;

			if(appAc == 1){
				console.log("Forced AC ON => Switching on AC");
				createCIN("[switchOn]","ledAC");
				// console.log("-------------------------");
				map1.clear();
			}
			else if(appAc == 0){
				console.log("Forced AC Off => Switching Off AC");
				createCIN("[switchOff]","ledAC");
				// console.log("-------------------------");
				map1.clear();
			}
			else if(appAc == 2){
				smartCooling(map1);
				map1.clear();
			}
			else{
				console.log("Nothing to do!!");
				// console.log("------------------");
				map1.clear();
			}
		}

	}
	else{
		console.log("XYZ");
	}
	res.sendStatus(200);	
});

// createAE();
function createAE(){
	var options = {
		uri: "http://127.0.0.1:8080/~/in-cse/in-name/",
		method: "POST",
		headers: {
			"X-M2M-Origin": "admin:admin",
			"Content-Type": "application/json;ty=2"
		},
		json: { 
			"m2m:ae":{
				"rn": monitorName + "Monitor",			
				"api":"org.demo.app",
				"rr":"true",
				"poa":["http://"+ monitorIP + ":" + monitorPort]
			}
		}
	};

	request(options, function (err, resp, body) {
		if(err){
			console.log("AE Creation error : " + err);
		}else {
			console.log("AE Creation :" + resp.statusCode);
			sensors.forEach(function (sensor,index){
				createSUB(sensor);
			});
		}
	});
}


function createSUB(sensor){
	var options = {
		uri: "http://127.0.0.1:8080/~/in-cse/in-name/" + sensor + "/DATA",
		method: "POST",
		headers: {
			"X-M2M-Origin": "admin:admin",
			"Content-Type": "application/json;ty=23"
		},
		json: {
			"m2m:sub": {
				"rn": "SUB_" + sensor + "Monitor",
				"nu": ["http://" + monitorIP + ":" + monitorPort + "/" + sensor],
				"nct": 2,
				"enc": {
					"net": 3
				}
			}
		}
	};

	request(options, function (err, resp, body) {
		if(err){
			console.log("SUB Creation error : " + err);
		}else{
			console.log("SUB Creation : " + resp.statusCode);
		}
	});
}

app.get("/",function(req,res){
    res.render('home',{valAc:-1,valLight:-1,valWindow:-1,valCurtain:-1,type:"HOME"});
});

app.post("/userAc",function(req,res){
    appAc = req.body.userAc;
	res.render('home',{valAc:appAc,valLight:appLight,valWindow:appWindow,valCurtain:appCurtain,type:"AC"});
});

app.post("/userLight",function(req,res){
  	appLight = req.body.userLight;
	res.render('home',{valAc:appAc,valLight:appLight,valWindow:appWindow,valCurtain:appCurtain,type:"Light"});
});

app.post("/userWindow",function(req,res){
  	appWindow = req.body.userWindow;
	res.render('home',{valAc:appAc,valLight:appLight,valWindow:appWindow,valCurtain:appCurtain,type:"Window"});
});

app.post("/userCurtain",function(req,res){
  	appCurtain = req.body.userCurtain;
	res.render('home',{valAc:appAc,valLight:appLight,valWindow:appWindow,valCurtain:appCurtain,type:"Curtain"});
});

function createCIN(commandName,actuator){
	var options = {
		uri: "http://127.0.0.1:8080/~/in-cse/in-name/" + actuator + "/COMMAND",
		method: "POST",
		headers: {
			"X-M2M-Origin": "admin:admin",
			"Content-Type": "application/json;ty=4"
		},
		json: {
			"m2m:cin":{
					"con": commandName
				}
			}
	};

	request(options, function (err, resp, body) {
		if(err){
			console.log("CIN Creation error : " + err);
		}else{
			console.log("CIN Creation : " + resp.statusCode);
		}
	});
}

function smartCooling(mp){
	var TO = mp.get("TempOUT");
	var TIN = mp.get("TempIN");
	var D = mp.get("Dust");

	var OH = 24;
	var OL = 21;

	if(TO < OH && TO > OL){
		if(D < 0.5){
			console.log("Low Dust Outside & Optimal Temperature Outside => Open Window & Turn Off AC");
			createCIN("[switchOn]","ledWindow");
			createCIN("[switchOff]","ledAC");
			// console.log("-------------------------");
		}
		else{
			console.log("High Dust Outside => Close Window & Turn ON AC");
			createCIN("[switchOff]","ledWindow");
			createCIN("[switchOn]","ledAC");
			// console.log("-------------------------");
		}
	} 
	else if(TIN > OH || TIN <OL){
			console.log("Temp Inside is Not Optimal => Close Window & Turn ON AC to set Optimal Temperature Inside");
			createCIN("[switchOff]","ledWindow");
			createCIN("[switchOn]","ledAC");
			// console.log("-------------------------");
	}
	else if(TIN > OL && TIN < OH){
			console.log("Temp Inside is Optimal => Turn AC Off");
			createCIN("[switchOff]","ledAC");
			// console.log("-------------------------");
	}
	else{
		console.log("Nothing To Do!!");
		// console.log("--------------------");
	}
}
