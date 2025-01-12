
const char* index_html = R"(<!DOCTYPE html>
<!--- ESP32 Remote Shutter Front End -->
<!--- Author: Timothy Do -->

<html>

<head>
  <title>ESP32 Remote Shutter</title>
  <link rel="icon" href="data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 100 100%22><text y=%22.9em%22 font-size=%2290%22>📸</text></svg>">
  <meta charset="UTF-8"/>
</head>

<style>
    * {
        color: white;
        font-family: Roboto;
        opacity: 0%;
        transition: 5s;
    }
    *:hover, *:active {
        opacity: 100%;
        transition: 0.25s;
    }
    body {
        background-color: black;
        text-align: center;
        align-items: center;
        opacity: 0%;
        transition: 5s ease-out;
    }
    h1 {
        font-size: 4em;
    }
    h2 {
        font-size: 3em; 
    }
    h3 {
        font-size: 2.5em;
    }
    label,input {
        font-size: 2em;
    }
    input {
        color: black;
    }
    button {
        opacity: 100%;
        transition: 0.25s;
    }
    button:hover {
        opacity: 50%;
        transition: 0.25s;
    }
    button:active {
        transform: translateY(5px);
    }
    #controls, #results {
        background-color: rgb(50,50,50);
        width: 80%;
        margin: 0 auto;
        padding: 1px;
        padding-bottom: 1em;
        line-height: 0.5em;
    }
    #controls button, #results button {
        background-color: yellow;
        color: black;
        font-size: 2.5em;
        margin: 0.1em;
    }
    #controls label {
        line-height: 1.25em;
    }
    footer {
        text-align: center;
        align-items: center;
        bottom: 3%;
    }
</style>

<script>
    let singleCapture;
    let intervalometer;
    let status;
    let singleButton;
    let interval;
    let intervalButton;
    let switchButton;
    let httpCode;

    let switchStatus = false; //false: single capture, true: intervalometer

    window.onload = function() {
        // Setting Variables
        singleCapture = document.getElementById('singleCapture');
        intervalometer = document.getElementById('intervalometer');
        status = document.getElementById('status');
        singleButton = document.getElementById('singleButton');
        intervalButton = document.getElementById('intervalButton');
        switchButton = document.getElementById('switchButton');

        // Event Liesteers
        singleButton.addEventListener('click', () => {
            single();
        });

        switchButton.addEventListener('click', () => {
            switchModes();
        });

        // Finished Initializing Remote Shutter Web App
        updateStatus('Initialized Remote Shutter');
    }

    function switchModes() {
        let logString = `Set Remote Shutter to ${switchStatus ? "Intervalometer" : "Single Capture"} Mode`;
        updateStatus(logString);
        singleCapture.hidden = !switchStatus;
        intervalometer.hidden = switchStatus;
        switchStatus = !switchStatus;
    }

    function capture(period) {
        var xhttp = new XMLHttpRequest();
        xhttp.open("POST", "/capture", true);
        xhttp.setRequestHeader("Content-Type","text/plain");
        xhttp.onload = function() {
            httpCode = xhttp.status;
            if(httpCode == 200) {
                console.log('Capture Successful!');
            }
            else {
                console.log('Capture Error: ',httpCode);
            }
        }
        xhttp.send(period.toString());
        
        return httpCode;
    }

    function single() {
        httpCode = capture(500); 
        let logStr = `${httpCode} - Capture ${httpCode == 200 ? "Success!" : "Failed."}`;
        console.log(logStr);
        updateStatus(logStr);
    }

    function multiple(intervalTime,exposureTime,numImages) {
        let period = intervalTime + exposureTime;
        let i = 0;
        interval = setInterval(function() {
            
            i++;
            if(i == numImages) {
                clearInterval(interval);
            }

        },period)
    }

    function updateStatus(statusString) {
        status.innerHTML = `<h2>Status: ${statusString}</h2>`;
    }
</script>

<body>
    <h1>📷 ESP32 Remote Shutter 📸</h1>
    <div id="controls">
        <div id="singleCapture">
            <h2>1️⃣ Single Capture Mode </h2>
            <button id="singleButton">Capture</button>
        </div>
        <div id="intervalometer" hidden>
            <h2>🔄 Intervalometer </h2>
            <label for="expsoureTime">Exposure Time (ms):</label>
            <input type="number" id="exposureTime" name="exposureTime" min="0" max="3000"></input>
            <br>
            <label for="intervalTime">Interval Time (ms):</label>
            <input type="number" id="intervalTime" name="intervalTime" min="0" max="600000"></input>
            <br>
            <label for="numImages">Number of Images:</label>
            <input type="number" id="numImages" name="numImages" min="1", max="10000"></input>
            <br>
            <button id="intervalButton">Capture</button>
        </div>
    </div>
    <div id="results">
        <div id="status"></div>
        <button id="switchButton">Switch</button>
    </div>

</body>

<footer>
    <h2>Made with ⚡️ by Timothy Do</h2>
</footer>

</html>
)";
        