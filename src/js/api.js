var api = module.exports;

api.REQUEST_TYPE_SEND_QUESTION = 10;
api.REQUEST_TYPE_GET_QUESTIONS = 20;
api.REQUEST_TYPE_SUBMIT_ANSWER = 30;

var websocket;
var API_ROOT = "45.55.209.50:6942/api/";
var newDataFunction, getQuestionsFunction, sendQuestionResultFunction, submitAnswerResultFunction, connectionFunction;

var loggingEnabled = false;

function log(message){
    if(loggingEnabled){
        console.log("[API] " + message);
    }
}

api.setLoggingEnabled = function(enabled){
    loggingEnabled = enabled;
    console.log("[API] Turned logging " + (enabled ? "on." : "off."));
}

api.apiRequest = function(requestJSON)
{
		requestJSON.watchtoken = Pebble.getWatchToken();
    var jsonString = JSON.stringify(requestJSON);
    log("Sending data " + jsonString);
    websocket.send(jsonString);
}

api.connect = function(newData, getQuestions, sendQuestionResult, submitAnswerResult, connection) {
    websocket = new WebSocket("ws://" + API_ROOT + Pebble.getWatchToken());

    newDataFunction = newData;
    getQuestionsFunction = getQuestions;
    sendQuestionResultFunction = sendQuestionResult;
    submitAnswerResultFunction = submitAnswerResult;
    connectionFunction = connection;

    websocket.onopen = function() {
        log("Connected to API.");
        connectionFunction(true);
    };

    websocket.onmessage = function(message) {
        var message = JSON.parse(message.data);

        log("Got message " + JSON.stringify(message));

        newDataFunction(message);
        switch(message.requestType){
            case api.REQUEST_TYPE_GET_QUESTIONS:
                getQuestionsFunction(message.questions);
                break;
            case api.REQUEST_TYPE_SEND_QUESTION:
                sendQuestionResultFunction(message.sendResult);
                break;
            case api.REQUEST_TYPE_SUBMIT_ANSWER:
                submitAnswerResultFunction(message.answerResult);
                break;
        }
    };

    websocket.onclose = function(){
        log("Connection to API has been closed.");
        connectionFunction(false);
    };
};