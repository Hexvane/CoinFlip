var api = require("./api");
var MessageQueue = require("./messagequeue");

function newData(rawJSON){
    //Do something here with the raw JSON from the server if you wish for some reason...
}

//Gets a JSON object with the questions objects
function getQuestions(questionsJSON) {
    //Send a message to the Pebble telling it to wipe the current questions
    MessageQueue.sendAppMessage({
        requestType: api.REQUEST_TYPE_GET_QUESTIONS
    });
    //Send each question
    for(var i = 0; i < questionsJSON.length; i++){
        MessageQueue.sendAppMessage(questionsJSON[i]);
    }
}

//This is called when the send question result is fetched
function sendQuestionResult(resultJSON) {
    MessageQueue.sendAppMessage(resultJSON);
}

//This is called when there is a result for the answer send
function sendAnswerResult(resultJSON) {
    MessageQueue.sendAppMessage(resultJSON);
}

//Whenever the websocket connection changes, this is called with a boolean of its status
function connectionChanged(connected) {
    console.log("Connection status changed. Connected: " + connected);
    //If connected, send a request for the current questions
    if(connected){
        api.apiRequest({
            requestType: api.REQUEST_TYPE_GET_QUESTIONS,
            watchtoken: Pebble.getWatchToken()
        });
    }
    else{
        //Otherwise if disconnected, try to connect to the API again
        connectToAPI();
    }
}

function connectToAPI(){
    //Connect to the API service
    api.connect(newData, getQuestions, sendQuestionResult, sendAnswerResult, connectionChanged);
}

//Send anything from the watch to the API
Pebble.addEventListener("appmessage", function(e){
    console.log("Got a payload: " + JSON.stringify(e.payload));
    api.apiRequest(e.payload);
});

//When the phone is ready
Pebble.addEventListener("ready", function(e){
    //Turn on logging
    api.setLoggingEnabled(true);
    //Connect to the API
    connectToAPI();
});