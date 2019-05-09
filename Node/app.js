var express = require("express");
var app = express();

// var mongoose = require("mongoose");
var bodyParser = require("body-parser");


app.set("view engine","ejs");
app.use(express.static("public"));
app.use(bodyParser.urlencoded({extended:true}));

var appAc = 3,appLight = 3,appWindow = 3,appCurtain = 3;

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

app.listen(3000,function(){
	console.log("Server is Listening on port 3000");
});