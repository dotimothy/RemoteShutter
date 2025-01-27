
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
        z-index: 0;
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
    input[type=checkbox] {
        margin: 0.5em;
        font-size: 2.5em;
        transform: scale(2);
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
        width: 85%;
        padding-bottom: 1em;
        line-height: 1em;
    }
    #controls {
        left: 7.5%;
        top: 15%;
        height: 40%;
    }
    #results {
        bottom: 15%;
        left: 7.5%;
        height: 25%;
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
    #results label, #results input {
        padding: 0.25em;
    }
    #status {
        padding: 0px;
        line-height: 3em;
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
    #blocker {
        background-color: black;
        z-index: 1;
        position: fixed;
        width: 100%;
        height: 100%;
    }
</style>

<script>
    let welcome;
    let switchStatus = false; //false: single capture, true: intervalometer
    let blocker;
    let bulb;
    let bulbDiv;
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
    let sleepButton;

    window.onload = function() {
        // Setting Variables
        bulb = document.getElementById('bulb');
        bulbDiv = document.getElementById('bulbDiv');
        blocker = document.getElementById('blocker'); 
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
        sleepButton = document.getElementById('sleepButton');

        // Preload Previous Session Variables
        loadSession();

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
            switchModes(switchStatus);
        });

        sleepButton.addEventListener('click', () => {
            blocker.hidden = false;
            document.body.requestFullscreen();
        });

        blocker.onclick = function () {
            blocker.hidden = true;
            document.exitFullscreen();
        }

        bulb.addEventListener('change', function() {
            sessionStorage['bulb'] = bulb.checked;
        });

        timer.addEventListener('change', function() {
            sessionStorage['timer'] = timer.value;
        });

        exposureTimeInput.addEventListener('change', function() {
            sessionStorage['exposureTime'] = exposureTimeInput.value;
        });

        intervalTimeInput.addEventListener('change', function() {
            sessionStorage['intervalTime'] = intervalTimeInput.value;
        });

        numImagesInput.addEventListener('change', function() {
            sessionStorage['numImages'] = numImagesInput.value;
        });


        // Finished Initializing Remote Shutter Web App
        updateStatus('Initialized Remote Shutter');

        // Show the UI
        if(!welcome) {
            alert('Welcome to the ESP32 Remote Shutter! Capture with your DSLR Wirelessly in Two Modes: Single Capture or Intervalometer!!!\n\nSleep Mode for Power Saving by turning the screen completely off. Tap anywhere on screen to awaken!');
            sessionStorage['welcome'] = true;
        }
        blocker.hidden = true; 
    }

    function loadSession() {
        welcome = sessionStorage.getItem('welcome') !== null && sessionStorage.getItem('welcome') == 'true';
        if(sessionStorage.getItem('switchStatus') !== null && sessionStorage.getItem('switchStatus') == 'true') { // Shutter Mode
            switchModes(switchStatus);
        }
        bulb.checked = sessionStorage.getItem('bulb') !== null && sessionStorage.getItem('bulb') == 'true'; //Auto Check Bulb from Memory
        if(sessionStorage.getItem('timer') !== null) { //Remember Timer Value
            timer.value = Number(sessionStorage.getItem('timer'));
        }
        if(sessionStorage.getItem('exposureTime') !== null) {
            exposureTimeInput.value = Number(sessionStorage.getItem('exposureTime'));
        }
        if(sessionStorage.getItem('intervalTime') !== null) {
            intervalTimeInput.value = Number(sessionStorage.getItem('intervalTime'));
        }
        if(sessionStorage.getItem('numImages') !== null) {
            numImagesInput.value = Number(sessionStorage.getItem('numImages'));
        }

        console.log(`Loaded Session:`);
        console.log(sessionStorage);
    }

    function getHostname() {
        hostIP = window.location.hostname;
    }

    function hideControls() {
        singleCapture.hidden = true;
        singleButton.hidden = true;
        intervalometer.hidden = true;
        intervalButton.hidden = true;
        switchButton.hidden = true;
        bulbDiv.hidden = true;
    }

    function showControls () {
        singleCapture.hidden = switchStatus;
        singleButton.hidden = switchStatus;
        intervalometer.hidden = !switchStatus;
        intervalButton.hidden = !switchStatus;
        controls.hidden = false;
        switchButton.hidden = false;
        bulbDiv.hidden = false; 
    }

    function switchModes() {
        singleCapture.hidden = !switchStatus;
        singleButton.hidden = !switchStatus;
        intervalometer.hidden = switchStatus;
        intervalButton.hidden = switchStatus;
        switchStatus = !switchStatus;
        sessionStorage['switchStatus'] = switchStatus;
        let logString = `Set Remote Shutter to ${switchStatus ? "Intervalometer" : "Single Capture"} Mode`;
        updateStatus(logString);
    }

    function fillProgess() {
        inProgress.innerHTML = '<h2>Capture Settings:</h2> <br>';
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
        },captureTime);
        on.send();
    }

    function single() {
        hideControls();
        fillProgess();
        if(bulb.checked) {
            let logStr = `Processing Single Capture in Bulb Mode...`; 
            let prevCaptureTime = captureTime;
            captureTime = Math.max(timer.value,captureTime); 
            updateStatus(logStr);
            clearInterval(interval);
            capture(); 
            setTimeout(function () {
                logStr = `${httpCode} - Single Capture ${httpCode== 200 ? "Success!" : "Failed."}`;
                updateStatus(logStr);
                httpCode = 0;
                clearProgess();
                showControls();
            },captureTime+50);
            captureTime = prevCaptureTime;
        }
        else {
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
                    },captureTime+50);
            }
            },0);
        }
    }

    function multiple() {
        intervalTime = Number(intervalTimeInput.value); 
        exposureTime = Number(exposureTimeInput.value);
        numImages = Number(numImagesInput.value);
        period = Math.max(intervalTime + exposureTime,captureTime);
        let ts = Date.now();
        let delta = 0;
        let counter = 0;
        let logStr = '';
        hideControls();
        fillProgess();
        if(bulb.checked) {
            let prevCaptureTime = captureTime;
            captureTime = period;
            interval = setInterval(function() {
                logStr = `Processing Multi Capture (${counter+1}/${numImages}) in Bulb Mode...`;
                updateStatus(logStr);
                capture(false);
                if(counter == numImages || httpCode != 200) {
                    logStr = `${httpCode} - (${counter}/${numImages}) Multi Capture ${httpCode == 200 ? "Success!" : "Failed."}`;
                    updateStatus(logStr);
                    clearProgess();
                    showControls();
                    clearInterval(interval);
                }
                if(httpCode == 200) {
                    counter++;
                }
            },captureTime+50);
            captureTime = prevCaptureTime;
        }
        else {
            interval = setInterval(function() {
                let delay = period;
                delta = delay - (Date.now() - ts); // in ms
                if(delta <= 0) {
                    logStr = `Processing Multi Capture (${counter+1}/${numImages})...`;
                    updateStatus(logStr);
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
                    },captureTime+50);
                    ts = Date.now();
                }
                else {
                    let deltaStr = ((delta - delta % 100) / 1000.0).toFixed(1);
                    logStr = `Multi Capture (${Math.min(counter+1,numImages)}/${numImages}) in ${deltaStr} s`;
                    updateStatus(logStr);
                }
            },0);
        }
    }

    function updateStatus(statusString) {
        status.innerHTML = `<h2>Status: ${statusString}</h2>`;
    }
</script>

<body>
    <div id="blocker"></div>
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
            <input type="number" id="exposureTime" name="exposureTime" min="0" max="captureTime00" value="1000"></input>
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
        <button id="switchButton">Switch</button>
        <button id="sleepButton">Sleep</button>     
        <br>
        <div id="bulbDiv">
            <label for="bulb">Bulb Mode:</label>
            <input type="checkbox" id="bulb"></input>
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
        