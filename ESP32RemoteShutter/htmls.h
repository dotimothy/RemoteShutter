
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
    }
    body {
        background-color: black;
        text-align: center;
        align-items: center;
    }
    h1 {
        font-size: 4em;
    }
    h2, button {
        font-size: 3em; 
    }
    h3,label,input {
        font-size: 2em;
    }
    input {
        color: black;
    }
    button {
        opacity: 100%;
        transition: 0.125s;
    }
    button:hover {
        opacity: 50%;
        transition: 0.125s;
    }
    button:active {
        transform: translateY(5px);
        transition: 0.125s;
    }
    #controls, #results {
        background-color: rgb(50,50,50);
        position: absolute;
        width: 80%;
        padding: 1px;
        padding-bottom: 1em;
        line-height: 1em;
    }
    #controls {
        left: 10%;
        top: 15%;
        height: 40%;
    }
    #results {
        bottom: 15%;
        left: 10%;
    }
    #controls button, #results button {
        background-color: yellow;
        color: black;
        margin: 0.25em;
    }
    #controls button {
        bottom: 0;
    }
    #controls label, #controls input {
        line-height: 1.25em;
        margin: 0.25em; 
    }
    #status {
        padding: 0px;
        line-height: 4em;
    }
    footer {
        position: absolute; 
        bottom: 0%;
        left: 0; 
        width: 100%; 
        background-color: black;
        align-items: center; 
    }
    #begin {
        opacity: 100%;
        z-index: 1;
    }
    #inProgress {
        line-height: 0.5em;
        margin: 0 auto;
    }
</style>

<script>
    let ui;
    let footer;
    let title;
    let hostIP;
    let status;
    let httpCode = 0;
    let captureTime = 300;
    let inProgress;
    let singleCapture;
    let intervalometer;
    let singleButton;
    let timer;
    let interval;
    let intervalButton;
    let period;
    let exposureTimeInput; 
    let intervalTimeInput;
    let numImagesInput;
    let switchButton;
    let fullscreenButton;

    let switchStatus = false; //false: single capture, true: intervalometer

    window.onload = function() {
        // Setting Variables
        ui = document.getElementById('ui');
        footer = document.querySelector('footer');
        title = document.getElementById('title');
        status = document.getElementById('status');
        inProgress = document.getElementById('inProgress');
        singleCapture = document.getElementById('singleCapture');
        timer = document.getElementById('timer');
        intervalometer = document.getElementById('intervalometer');
        exposureTimeInput = document.getElementById('exposureTime');
        intervalTimeInput = document.getElementById('intervalTime');
        numImagesInput =  document.getElementById('numImages');
        singleButton = document.getElementById('singleButton');
        intervalButton = document.getElementById('intervalButton');
        switchButton = document.getElementById('switchButton');
        fullscreenButton = document.getElementById('fullscreenButton');

        //Get Hostname
        getHostname();

        // Set Title
        title.innerHTML = `📷 ESP32 Remote Shutter 📸 (Host: ${hostIP})`;

        // Event Liesteers
        singleButton.addEventListener('click', () => {
            single();
        });

        intervalButton.addEventListener('click', () => {
            multiple();
        })

        switchButton.addEventListener('click', () => {
            switchModes();
        });

        fullscreenButton.addEventListener('click', () => {
            toggleFullscreen();
        });

        // Finished Initializing Remote Shutter Web App
        updateStatus('Initialized Remote Shutter');

        // Show the UI
        alert('Welcome to the ESP32 Remote Shutter!');
        ui.hidden = false;
        footer.hidden = false;
    }

    function toggleFullscreen() {
        if(document.fullscreenElement) {
            document.exitFullscreen()
        } else {
            document.body.requestFullscreen();
        }
    }

    function getHostname() {
        hostIP = window.location.hostname;
    }

    function hideControls() {
        singleCapture.hidden = true;
        singleButton.hidden = true;
        intervalometer.hidden = true;
        intervalButton.hidden = true;
        fullscreenButton.hidden = true;
        switchButton.hidden = true;
    }

    function showControls () {
        singleCapture.hidden = switchStatus;
        singleButton.hidden = switchStatus;
        intervalometer.hidden = !switchStatus;
        intervalButton.hidden = !switchStatus;
        controls.hidden = false;
        switchButton.hidden = false;
        fullscreenButton.hidden = false;
    }

    function switchModes() {
        singleCapture.hidden = !switchStatus;
        singleButton.hidden = !switchStatus;
        intervalometer.hidden = switchStatus;
        intervalButton.hidden = switchStatus;
        switchStatus = !switchStatus;
        let logString = `Set Remote Shutter to ${switchStatus ? "Intervalometer" : "Single Capture"} Mode`;
        updateStatus(logString);
    }

    function fillProgess() {
        inProgress.innerHTML = '<h2>Processing Capture with the Following Settings:</h2> <br>';
        let mode = switchStatus ? "Intervalometer" : "Single Capture";
        inProgress.innerHTML += `<h3>Capture Mode: ${mode}</h3> <br>`;
        if(switchStatus) { // Intervalometer
            let exposureTime = exposureTimeInput.value;
            let intervalTime = intervalTimeInput.value; 
            let numImages = numImagesInput.value; 
            inProgress.innerHTML += `<h3>Exposure Time: ${exposureTime} ms</h3> <br>`;
            inProgress.innerHTML += `<h3>Interval Time: ${intervalTime} ms</h3> <br>`;
            inProgress.innerHTML += `<h3>Total Period: ${period} ms</h3> <br>`;
            inProgress.innerHTML += `<h3>Number of Images: ${numImages}</h3>`;
        } else { //Single Capture
            let timerValue = timer.value;
            inProgress.innerHTML += `<h3>Timer: ${timerValue}  ms`;
        }
        inProgress.hidden = false;
    }

    function clearProgess() {
        inProgress.innerHTML = '';
        inProgress.hidden = true; 
    }

    function capture(verbose=false) {
        let on = new XMLHttpRequest();
        on.open("GET", "/on.html");
        on.onload = setTimeout(function() {
            if(on.status == 200) {
                let off = new XMLHttpRequest();
                off.open("GET","/off.html");
                off.onload = function () {
                    httpCode = off.status;
                    if(verbose) {
                        let logStr = `${off.status} - Turn Off Request ${off.status== 200 ? "Success!" : "Failed."}`;
                        console.log(logStr);
                        updateStatus(logStr);
                    }
                }
                off.send();
            }
            else {
                httpCode = on.status;
                return;
            }
            if(verbose) {
                let logStr = `${on.status} - Turn On Request ${on.status== 200 ? "Success!" : "Failed."}`;
                console.log(logStr);
                updateStatus(logStr);
            }
        },300);
        on.send();
    }

    function single() {
        hideControls();
        fillProgess();
        let delay = Number(timer.value);
        let ts = Date.now();
        let delta = 0;
        interval = setInterval(function() {
            delta = delay - (Date.now() - ts); // in ms
            let deltaStr = ((delta - delta % 100) / 1000.0).toFixed(1);
            let logStr = `Single Capture in ${deltaStr} s`; 
            updateStatus(logStr);
            if(delta <= 0) {
                let logStr = `Processing Single Capture...`; 
                updateStatus(logStr);
                clearInterval(interval);
                capture(); 
                setTimeout(function () {
                    logStr = `${httpCode} - Single Capture ${httpCode== 200 ? "Success!" : "Failed."}`;
                    updateStatus(logStr);
                    httpCode = 0;
                    clearProgess();
                    showControls();
                },300);
        }
        },0);
    }

    function multiple() {
        intervalTime = Number(intervalTimeInput.value); 
        exposureTime = Number(exposureTimeInput.value);
        numImages = Number(numImagesInput.value);
        period = Math.max(intervalTime + exposureTime,captureTime);
        let ts = Date.now();
        let delta = 0;
        let counter = 0;
        hideControls();
        fillProgess();
        interval = setInterval(function() {
            let delay = period;
            let logStr = '';
            delta = delay - (Date.now() - ts); // in ms
            console.log(delay);
            if(delta <= 0) {
                logStr = `Processing Multi Capture (${counter+1}/${numImages})...`;
                capture(false);
                counter++;
                setTimeout(function() {
                    if(counter == numImages || httpCode != 200) {
                        logStr = `${httpCode} - (${counter}/${numImages}) Multi Capture ${httpCode == 200 ? "Success!" : "Failed."}`;
                        updateStatus(logStr);
                        clearProgess();
                        showControls();
                        clearInterval(interval);
                    }
                },300);
                ts = Date.now();
            }
            else {
                let deltaStr = ((delta - delta % 100) / 1000.0).toFixed(1);
                logStr = `Multi Capture (${counter}/${numImages}) in ${deltaStr} s`;
                updateStatus(logStr);
            }
        },0);
    }

    function updateStatus(statusString) {
        status.innerHTML = `<h2>Status: ${statusString}</h2>`;
    }
</script>

<body>
    <div id="ui" hidden>
        <h1 id="title"></h1>
        <div id="controls">
            <div id="singleCapture">
                <h2>1️⃣ Single Capture Mode </h2>
                <label for="timer">Timer (ms):</label>
                <input type="number" id="timer" name="timer" min="0"value="0"></input>
                <br>
            </div>
            <div id="intervalometer" hidden>
                <h2>🔄 Intervalometer </h2>
                <label for="expsoureTime">Exposure Time (ms):</label>
                <input type="number" id="exposureTime" name="exposureTime" min="0" max="30000" value="1000"></input>
                <br>
                <label for="intervalTime">Interval Time (ms):</label>
                <input type="number" id="intervalTime" name="intervalTime" min="0", value="1000"></input>
                <br>
                <label for="numImages">Number of Images:</label>
                <input type="number" id="numImages" name="numImages" min="1",  value="1"></input>
            </div>
            <div id="inProgress"></div>
        </div>
        <div id="results">
            <div id="status"></div>
            <button id="singleButton">Capture</button>
            <button id="intervalButton" hidden>Capture</button>
            <button id="fullscreenButton">Fullscreen</button>
            <button id="switchButton">Change Mode</button>
            
            <h2>
        </div>
    </div>

</body>

<footer hidden>
    <h2>Made with ⚡️ by Timothy Do</h2>
</footer>

</html>
)";

const char* on_html = R"(Remote Shutter Turned On!)";

const char* off_html = R"(Remote Shutter Turned Off!)";
        