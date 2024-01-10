
let myip = '192.168.4.1';

let files_tree_json = [{"path":"", "dir":1,"name":"Example Files", "size":"0"}, {"path":"/tests/textFile.txt", "dir":0,"name":"textFile.txt", "size":"123"}, {"path":"/tests/Othernet Satcalc by Tynet.eu min.html", "dir":0,"name":"Othernet Satcalc by Tynet.eu min.html", "size":"13123"}, {"path":"/tests/ee1b-wikipedia-Ajla_Tomljanović.html.tbz2", "dir":0,"name":"ee1b-wikipedia-Ajla_Tomljanović.html.tbz2", "size":"146461"}];

let wsHandle;
let plays = false;
let midiTimeout;
let lastMidiBuf;

let synth;

function createSynth() {
    synth = new Tone.PolySynth(Tone.Synth, {
        envelope: {
            attack: 0.02,
            decay: 0.1,
            sustain: 0.3,
            release: 1
        },
    }).toDestination()
    synth.maxPolyphony = 64
    synth.toDestination();
    synth.sync();
    synth.volume.value = -12;
}

let lastNotePlayed = 0;
let lastNoteStart = 0;

function midiNoteToFreq (note){
    return Math.pow(2, ((note - 69) / 12)) * 440;
}

function playNote(time, note) {
    let freq = midiNoteToFreq(note[0]);
    let start = (note[1]-lastNoteStart)+time+0.0001;
    
    let vel = note[2]/127;
    let dur;
    if (note[3] == 0) {
        dur = note[3]+0.1;
    } else {
        dur = note[3];
    }
    synth.triggerAttackRelease(freq, dur, start, vel);
    lastNotePlayed = start;
    console.log("Note: "+start+" - "+note[1]+"-"+lastNoteStart+"-"+time);
}

function startMidi(){
    createSynth();
    Tone.start();
    Tone.Transport.start();
}
function stopMidi(){
    Tone.Transport.stop();
    Tone.Transport.cancel();
    synth.dispose()
    createSynth();
    timeSoFar = 0;
}

function addSongMidi(song){
    // if something is still playing (tone.now < timeSofar) use it
    let timeAtAdd;
    if (Tone.Transport.seconds > lastNotePlayed) {
        timeAtAdd = Tone.Transport.seconds;
    } else {
        timeAtAdd = lastNotePlayed;
    }
    console.log("AtAdd: "+timeAtAdd+" - transport time: "+Tone.Transport.seconds+" - lastplayed: "+lastNotePlayed+" - laststart: "+lastNoteStart); 
    // if there is no last note start, set it to current note
    if (lastNoteStart == 0 || lastNoteStart > song[0][1]) {
        lastNoteStart = song[0][1];
    }
    
    let lastNote = [0,0,0,0];
    song.forEach(elm => {
        if (elm[0] != 0) {
            playNote(timeAtAdd, elm);
            lastNote = elm;
        }
    });
    lastNoteStart = lastNote[1];
    console.log("Last Note Start: "+lastNoteStart);
}

// helper functions
function getExt(path) {
    return (path.match(/(?:.+..+[^\/]+$)/ig) != null) ? path.split('.').slice(-1): 'file';
}

function toggleActive(id) {
    document.getElementById(id).classList.toggle("is-active");
}

function toggleModal(id) {
    document.getElementById(id).classList.toggle("is-active");
    document.getElementById('html').classList.toggle("is-clipped");
}

function toggleTab(me, tabClass, contentClass, idToOpen) {
    document.querySelectorAll(contentClass).forEach(function(el) {
        el.style.display = 'none';
    });
    document.getElementById(idToOpen).style.display = 'block';
}

// Init ajax
const xhr = new XMLHttpRequest();
xhr.responseType = "json";

xhr.addEventListener("load", e => {
    if (xhr.status === 200) {
        filestree(xhr.response);
    }
});

xhr.addEventListener("error", e => {
    // alert("Cant connect, use template");
    filestree(files_tree_json);
});

xhr.onreadystatechange = function() { // Call a function when the state changes.
    if (this.readyState === XMLHttpRequest.DONE && this.status === 404) {
        // Request finished. Do processing here.
        filestree(files_tree_json);
    }
}

function synchronousRequest(url) {
    const syncXhr = new XMLHttpRequest();
    syncXhr.open('GET', url, false);
    syncXhr.send(null);
    if (syncXhr.status === 200) {
       return syncXhr.responseText;
    } else {
       throw new Error('Request failed: ' + syncXhr.statusText);
    }
 }

// sort an Array (arr) of Objects by a property (key)
function sortArrayOfObjects(arr, key, reverse = false) {
    arr.sort((a, b) => {
        if (a[key] < b[key]) {
        return -1;
        }
        if (a[key] > b[key]) {
        return 1;
        }
        return 0;
    });
    if (reverse) arr.reverse()
    return arr;
}

// get newest messages from backend
async function getMessages() {
    let folders = JSON.parse(synchronousRequest('/files/messages/'))
    folders = sortArrayOfObjects(folders, 'name', true)

    let files = []
    for (const folder of folders) {
        if(folder.name != "Up" && folder.dir == 1){ 
            let newFiles = JSON.parse(synchronousRequest(folder.path+'/'))
            files = files.concat(newFiles)
            if (files.length >= 4) break
        }
    }

    files = sortArrayOfObjects(files, 'name', true)
    files = files.slice(1, 5)
    
    let msgList = '<table class="table is-fullwidth"><thead><tr><th style="width:7rem">Call</th><th>Message</th></tr></thead>'
    for (const msgFile of files) {
        //check if tbz2 file or not, if not use txt
        let ext = getExt(msgFile.name);
        ext = ext.toString().toLowerCase();

        let newData = ''
        if (ext != 'tbz2') {
            newData = await getText(msgFile.path)
            let lines = newData.split('\n')
            for (const line of lines) {
                msgList += '<tr class="has-background-primary"><td></td><td>' + line + '</td></tr>'
            }
        } else {
            newData = await getTbz2(msgFile.path)

            let dateAndTime = msgFile.name.substring(0, msgFile.name.indexOf("."))
            let date = dateAndTime.substring(0, dateAndTime.indexOf("_"))
            let time = dateAndTime.substring(dateAndTime.indexOf("_")+1)
            time = time.replace('_', ':')
            time = time.replace('_tags', '')

            let special = (dateAndTime.indexOf("_tags") == -1) ? '' : ' - special Tags'

            msgList += '<tr class="has-background-black"><td></td><td>' + date + ' ' + time + special + '</td></tr>'
            msgList += await getAprsMessageList(newData)
        }

    }
    msgList += '</table>'
    
    //const msgList = await getAprsMessageList(data)
    document.getElementById('messages').innerHTML = msgList
    // generate smaller list of last 10 messages for Dashboard
    document.getElementById('messages_dash').innerHTML = msgList

    //refresh messages every 60s
    setTimeout(() => { getMessages() }, 60000);
}

// get filetree json from backend 
function getfilestree(path) {
    console.log(path);
    xhr.open("GET", path + '/', true);
    try {
        xhr.send();    
    } catch (error) {
        console.log(error);
    }
}

// build DOM for filetree
function filestree(json) {
    let tree = document.getElementById("filestree");
    tree.innerHTML = '';

    console.log(json);
    json.forEach(element => {
        let card = document.createElement("div");
        let inner = document.createElement("div");
        let name = document.createElement("span");
        let info = document.createElement("p");
        let wrapper = document.createElement("span");
        let iconwrap = document.createElement("span");
        card.classList = 'column is-6';
        inner.classList = 'notification';
        iconwrap.classList = 'icon is-large';
        wrapper.classList = 'icon-text';
        name.classList = 'p-3 is-size-5';
        name.textContent = element.name;
        info.classList = 'pt-3';

        let a = document.createElement("a");
        if (element.dir) {
            // use icon for UP and Refresh, esle Folder
            if (element.name == 'Up') {
                iconwrap.innerHTML = '<svg fill="none" stroke="currentColor" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 10l7-7m0 0l7 7m-7-7v18"></path></svg>';
            } else if (element.name == 'Refresh'){
                iconwrap.innerHTML = '<svg fill="none" stroke="currentColor" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 4v5h.582m15.356 2A8.001 8.001 0 004.582 9m0 0H9m11 11v-5h-.581m0 0a8.003 8.003 0 01-15.357-2m15.357 2H15"></path></svg>';
            } else {
                iconwrap.innerHTML = ' <svg fill="none" stroke="currentColor" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M3 7v10a2 2 0 002 2h14a2 2 0 002-2V9a2 2 0 00-2-2h-6l-2-2H5a2 2 0 00-2 2z"></path></svg>';
            }
            info.innerText = 'folder';

            a.href = '#';
            a.target = "_self";
            a.link = element.path;
            a.onclick = function (event) {
                event.preventDefault();
                getfilestree(element.path);
            };
        } else{
            iconwrap.innerHTML = '<svg fill="none" stroke="currentColor" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M7 21h10a2 2 0 002-2V9.414a1 1 0 00-.293-.707l-5.414-5.414A1 1 0 0012.586 3H7a2 2 0 00-2 2v14a2 2 0 002 2z"></path></svg>';
            //a.href = element.path;
            //a.target = "_blank";
            let fileExt = getExt(element.name);
            a.onclick = function (event) {
                event.preventDefault();
                toggleModal('md_fileview');
                viewFile(element.path, fileExt, element.name);
            }
            info.innerText = fileExt + ' - ' + element.size + ' bytes';
        }

        wrapper.appendChild(iconwrap);
        wrapper.appendChild(name);
        inner.appendChild(wrapper);
        inner.appendChild(info);
        a.appendChild(inner);
        card.appendChild(a);
        tree.appendChild(card);
    });
}

// update stats on UI
function updateStats(jsnStats) {
    document.getElementById('stats_fw').innerText = jsnStats.fw;
    document.getElementById('stats_hw').innerText = jsnStats.hw;

    document.getElementById('stats_cpu').innerText = jsnStats.cpu;
    document.getElementById('stats_cpubar').value = jsnStats.cpu;
    document.getElementById('stats_heap').innerText = Math.round(jsnStats.heap /1024*100)/100 + ' KB';
    document.getElementById('stats_minheap').innerText = Math.round(jsnStats.minheap / 1024*100)/100 + ' KB';
    document.getElementById('stats_psram').innerText = Math.round(jsnStats.psram / 1024*100)/100 + ' KB';

    var sdFree = (jsnStats.max - jsnStats.used) / (1024*1024);
    document.getElementById('stats_sdcap').innerText = Math.round(jsnStats.max / (1024*1024)*100)/100 + ' MB';
    document.getElementById('stats_sdfree').innerText = Math.round(sdFree*100)/100 + ' MB';;
    document.getElementById('stats_sdused').innerText = Math.round(jsnStats.used / (1024*1024)*100)/100  + ' MB';

    document.getElementById('stats_snr').innerText = jsnStats.snr;
    document.getElementById('stats_rssi').innerText = jsnStats.rssi;

    document.getElementById('stats_crc').innerText = jsnStats.crc;
    document.getElementById('stats_header').innerText = jsnStats.header;
    document.getElementById('stats_pkts').innerText = jsnStats.received;
    document.getElementById('stats_bitrate').innerText = jsnStats.bitrate;

    let progress = jsnStats.packet / jsnStats.packets * 100;
    if (!isFinite(progress)) {
        progress = 0
    }
    document.getElementById('stats_progress').innerText = Math.round(progress*100)/100 + ' %';
    document.getElementById('stats_progressbar').value = Math.round(progress*100)/100;
    document.getElementById('stats_file').innerText = jsnStats.filename;

    document.getElementById('stats_lnben').innerText = (jsnStats.ldo&0x20)>0?'YES':'NO';
    document.getElementById('stats_lnbir').innerText = (jsnStats.ldo&0x01)>0?'YES':'NO';
    document.getElementById('stats_lnbcon').innerText = (jsnStats.ldo&0x02)>0?'YES':'NO';
    document.getElementById('stats_lnbv').innerText = jsnStats.volt;      
    
    document.getElementById('stats_time').innerText = jsnStats.tstamp;    
}

// Receiver Preset/Increment Freq functions
function setReceiverPresetUS() {
    document.getElementById('rcv_freq').value = 2308100000;
}

function setReceiverPresetEU() {
    document.getElementById('rcv_freq').value = 1907000000;
}

function setReceiverAddTick() {
    var freq = parseInt(document.getElementById('rcv_freq').value);
    document.getElementById('rcv_freq').value = freq + 10000;
}

function setReceiverRemoveTick() {
    var freq = parseInt(document.getElementById('rcv_freq').value);
    document.getElementById('rcv_freq').value = freq - 10000;
}

function receiverAddAndSaveTick() {
    setReceiverAddTick()
    saveReceiver()
}

function receiverRemoveAndSaveTick() {
    setReceiverRemoveTick()
    saveReceiver()
}

// save Reeiver Settings
function saveReceiver() {
    let freq = document.getElementById('rcv_freq').value;
    let bw = document.getElementById('rcv_bw').value;
    let sf = document.getElementById('rcv_sf').value;
    let cr = document.getElementById('rcv_cr').value;
    let lnb = document.getElementById('lnb_volt').checked;
    //let lo = document.getElementById('lo_en').checked;
    let lo = false;
    let diseq = document.getElementById('diseq').checked;
    //let loid = document.getElementById('rcv_loid').value;
    let loid = 0;

    const http = new XMLHttpRequest();
    var params = 'freq=' + freq + '&bw=' + bw + '&sf=' + sf + '&cr=' + cr + '&lnb=' + lnb + '&lo=' + lo + '&diseq=' + diseq + '&loid=' + loid;
    console.log(params);
    var url = '/settings';
    http.open("POST", url, true);
    // Send the proper header information along with the request
    http.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');

    http.onreadystatechange = function() {//Call a function when the state changes.
        if(http.readyState == 4 && http.status == 200) {
            console.log('settings set', http.responseText);
            // update other UI elements that use Receiver data
            document.getElementById('stats_freq').innerText = freq/1000000 + ' mhz';
            document.getElementById('stats_lora').innerText = 'BW ' + getHumanReadableBW(parseInt(bw)) + ' - SF ' + sf + ' - CR ' + getHumanReadableCR(parseInt(cr));
            
            document.getElementById('tag_saveRcv').classList.remove('is-hidden');
            setTimeout(function(){ document.getElementById('tag_saveRcv').classList.add('is-hidden'); }, 5000);
        }
    }
    http.send(params);
}

// send WIFI Creds to backend
function postWifiSettings(path, ssid, pass, ap, auth, tlm) {
    const http = new XMLHttpRequest();

    console.log(path);
    var params = 'ssid=' + ssid + '&pass=' + pass + '&ap=' + ap + '&auth=' + auth + '&tlm=' + tlm;
    console.log(params);
    var url = '/wifi';
    http.open("POST", url, true);
    // Send the proper header information along with the request
    http.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');

    http.onreadystatechange = function() {//Call a function when the state changes.
        if(http.readyState == 4 && http.status == 200) {
            console.log('credentials set', http.responseText);
            return true;
        }
    }
    http.send(params);
    return true;
}

// save WIFI AP
function saveWifiAp() {
    let ssid = document.getElementById('ap_ssid').value;
    let pwd = document.getElementById('ap_pwd').value;
    let auth = document.getElementById('ap_auth').value;
    if(postWifiSettings('', ssid, pwd, 1, auth, 0)){
        document.getElementById('tag_saveAp').classList.remove('is-hidden');
        setTimeout(function(){ document.getElementById('tag_saveAp').classList.add('is-hidden'); }, 5000);
    }
}

// save WIFI Client
function saveWifiClient() {
    let ssid = document.getElementById('sta_ssid').value;
    let pwd = document.getElementById('sta_pwd').value;
    let tlm = document.getElementById('sta_tlm').checked;
    if(postWifiSettings('', ssid, pwd, 0, 0, tlm)){
        document.getElementById('tag_saveSta').classList.remove('is-hidden');
        setTimeout(function(){ document.getElementById('tag_saveSta').classList.add('is-hidden'); }, 5000);
    }
}

// send WIFI Creds to backend
function postDCSSSettings(path, ub, pol, freq) {
    const http = new XMLHttpRequest();

    console.log(path);
    var params = 'ub=' + ub + '&pol=' + pol + '&freq=' + freq;
    console.log(params);
    var url = '/dcss';
    http.open("POST", url, true);
    // Send the proper header information along with the request
    http.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');

    http.onreadystatechange = function() {//Call a function when the state changes.
        if(http.readyState == 4 && http.status == 200) {
            console.log('dcss set', http.responseText);
            return true;
        }
    }
    http.send(params);
    return true;
}

// save WIFI AP
function sendDCSS() {
    let ub = document.getElementById('dcss_ub').value;
    let pol = document.querySelector('input[name="dcss_pol"]:checked').id;
    let freq = document.getElementById('dcss_freq').value;
    if(postDCSSSettings('', ub, pol, freq)){
        document.getElementById('tag_saveDcss').classList.remove('is-hidden');
        setTimeout(function(){ document.getElementById('tag_saveDcss').classList.add('is-hidden'); }, 2000);
    }
}

//format SD Card
function formatSd() {
    if(confirm("Do you want to format SD card?\nIt will take about 5 minutes!")){
        document.getElementById('btn_formatsd').classList.add('is-loading');
        const http = new XMLHttpRequest();
        var url = '/format';
        http.open("POST", url, true);
        
        http.onreadystatechange = function() {//Call a function when the state changes.
            if(http.readyState == 4 && http.status == 200) {
                console.log('sd card formatted', http.responseText);
            }
            document.getElementById('btn_formatsd').classList.remove('is-loading');
        }
        http.send();
    }
}

//Remove Logfiles
function clearLogs() {
    if(confirm("Do you want to Clear all Logfiles?")){
        document.getElementById('btn_clearlogs').classList.add('is-loading');
        const http = new XMLHttpRequest();
        var url = '/clearlogs';
        http.open("POST", url, true);
        
        http.onreadystatechange = function() {//Call a function when the state changes.
            if(http.readyState == 4 && http.status == 200) {
                console.log('log cleared', http.responseText);
            }
            document.getElementById('btn_clearlogs').classList.remove('is-loading');
        }
        http.send();
    }
}

//Remove Temp Files
function clearTmp() {
    if(confirm("Do you want to Clear all Tmp files?")){
        document.getElementById('btn_cleartmp').classList.add('is-loading');
        const http = new XMLHttpRequest();
        var url = '/cleartmp';
        http.open("POST", url, true);
        
        http.onreadystatechange = function() {//Call a function when the state changes.
            if(http.readyState == 4 && http.status == 200) {
                console.log('tmp files cleared', http.responseText);
            }
            document.getElementById('btn_cleartmp').classList.remove('is-loading');
        }
        http.send();
    }
}

//Remove Logfiles
function reboot() {
    if(confirm("Do you want to Reboot the Device?")){
        document.getElementById('btn_reboot').classList.add('is-loading');
        const http = new XMLHttpRequest();
        var url = '/reboot';
        http.open("POST", url, true);
        
        http.onreadystatechange = function() {//Call a function when the state changes.
            if(http.readyState == 4 && http.status == 200) {
                console.log('rebooting Device', http.responseText);
            }
            document.getElementById('btn_reboot').classList.remove('is-loading');
        }
        http.send();
    }
}

//Factory Reset
function factoryReset() {
    if(confirm("Do you want to reset the device to factory defaults?\nAll your settings will be lost!\nFiles on the SD card will be preserved.")){
        document.getElementById('btn_freset').classList.add('is-loading');
        const http = new XMLHttpRequest();
        var url = '/freset';
        http.open("POST", url, true);
        http.onreadystatechange = function() {//Call a function when the state changes.
            if(http.readyState == 4 && http.status == 200) {
                console.log('device will reboot now', http.responseText);
            }
            document.getElementById('btn_freset').classList.remove('is-loading');
        }
        http.send();
    }
}

// get tbz2 file and return as text
async function getTbz2(filepath) {
    return fetch(filepath)
        .then(response => response.arrayBuffer())
        .then(data => {
            let arr = window.bz2.decompress(new Uint8Array(data));
            // remove file metadata - first 1024 bytes
            arr = arr.slice(1024, arr.length);
            return new TextDecoder('utf8').decode(arr); 
        });
}

// get text file and return it
async function getText(filepath) {
    return fetch(filepath)
        .then(response => response.text())
        .then(data => {
            return data;
        });
}

// get mp3 file and return it
async function getMp3(filepath) {
    return '<audio controls src="'+filepath+'" type="audio/mpeg">Your browser does not support the audio element.</audio>'
}

// get Image file and return it
async function getImage(filepath) {
    return '<img src="'+filepath+'" style="width: 100%">'
}

// get aprs meassges as list from parsing File/String
async function getAprsMessageList(aprsData) {
    let msgList = ''
    let lines = await aprsData.split('\n')
    lines.pop() //remove last line because it is always nul
    lines.forEach(line => {
        let parts = line.split(',')
        let call = '' + parts[0].match('[A-Z0-9]+-[0-9]+>')
        let match = '' + line.match(/q[A-Z0-9]{2},.*/)
        let msg = match.replace(/q[A-Z0-9]{2},/, '')
        msgList += '<tr><td>' + call.slice(0,-1) + '</td><td>' + msg + '</td></tr>' 
    });
    return msgList
}

// get File and open it in the Viewer
async function viewFile(filepath, fileExt) {
    document.getElementById('md_fileview_status').classList.add('loader');
    let result = 'File not Supported / Error while reading';

    let dataElement = document.getElementById('data_inline');
    let iframeElement = document.getElementById('data');
    iframeElement.src = "about:blank";
    fileExt = fileExt.toString().toLowerCase();

    if (fileExt == 'tbz2') {
        console.log('tbz2 File');
        result = await getTbz2(filepath);
    }
    if (fileExt == 'txt' || fileExt == 'json' || fileExt == 'html') {
        result = await getText(filepath);
    }
    if (fileExt == 'mp3') {
        console.log('MP3 File');
        result = await getMp3(filepath);
    }
    if (fileExt == 'jpg' || fileExt == 'jpeg' || fileExt == 'png') {
        console.log('jpg file');
        result = await getImage(filepath);
    }

    //display data in a special way by type
    if(filepath.includes('/log/')){
        dataElement.hidden = false
        iframeElement.hidden = true
        result = '<pre>' + result + '</pre>';
        dataElement.innerHTML = result
    } else if(filepath.includes('/messages/') && fileExt == 'tbz2'){
        dataElement.hidden = false
        iframeElement.hidden = true
        result = await getAprsMessageList(result);
        dataElement.innerHTML = result
    } else {
        dataElement.hidden = true
        dataElement.innerHTML = ''
        // Set the iframe's new HTML
        iframeElement.contentWindow.document.open();
        iframeElement.contentWindow.document.write(result);
        iframeElement.contentWindow.document.close();
        iframeElement.hidden = false
    }
   
    document.getElementById('md_fileview_status').classList.remove('loader');
}

function toggleMidiStream(){
    if (!plays) {
        console.log("Starting Realtime Midi Playback...");
        plays = true;
        document.getElementById('btn_play').innerText = "Stop";
        startMidi();
        wsHandle.send('get midi');
    } else {
        stopMidi();
        plays = false;
        document.getElementById('btn_play').innerText = "Play";
        clearTimeout(midiTimeout);
    }
}

function getHumanReadableBW(bw) {
    switch (bw) {
        case 13:
            return '200'
        case 14:
            return '400'
        case 15:
            return '800'
        default:
            return bw
    }
}

function getHumanReadableCR(cr) {
    switch (cr) {
        case 1:
            return '4/5'
        case 2:
            return '4/6'
        case 3:
            return '4/7'
        case 4:
            return '4/8'
        default:
            return cr
    }
}

// Requests current stats from Backend
function getStats() {
    let statsData = JSON.parse(synchronousRequest('/stats'))
    updateStats(statsData)
    setTimeout(function(){ getStats() }, 1000);
}

// ON Window loaded
document.addEventListener('DOMContentLoaded', event => {
    getfilestree('/files')
    getMessages()
    getStats()

    document.getElementById('info_ip').innerText = myip;
    try {        
        document.getElementById('rcv_freq').value = init_freq;
        document.getElementById('rcv_bw').value = init_bw;
        document.getElementById('rcv_sf').value = init_sf;
        document.getElementById('rcv_cr').value = init_cr;
        document.getElementById('lnb_volt').checked = init_lnb;
        //document.getElementById('lo_en').checked = init_lo;
        document.getElementById('diseq').checked = init_diseq;
        //document.getElementById('rcv_loid').value = init_loid;

        document.getElementById('stats_freq').innerText = init_freq/1000000 + ' mhz';
        document.getElementById('stats_lora').innerText = 'BW ' + getHumanReadableBW(init_bw) + ' - SF ' + init_sf + ' - CR ' + getHumanReadableCR(init_cr);
    } catch (error) {}

    document.getElementById('volume').addEventListener("input", function (e) {
        let val = document.getElementById('volume').value;
        let vol = -((val-10) * -3);
        document.getElementById('voltext').innerText = vol;
        synth.volume.value = vol;
    });

    wsHandle = new WebSocket('ws://' + window.location.host + '/ws');
    wsHandle.onmessage = function (event) {
        //console.log(event.data);
        let data = JSON.parse(event.data);

        if (data.type) {
            // not a status update
            if (data.type == 'midi') {
                if (JSON.stringify(lastMidiBuf) !== JSON.stringify(data.data)) {
                    // add mididata to playback
                    lastMidiBuf = data.data;
                    console.log(data.data);
                    addSongMidi(lastMidiBuf);
                }
                midiTimeout = setTimeout(function(){ wsHandle.send("get midi"); }, 250);
            } else {
                console.log("got something else");
            }
        }
    };
});

// BZ2 Library Minified
/*! bz2 (C) 2019-present SheetJS LLC */"use strict";!function(){const t=[0,79764919,159529838,222504665,319059676,398814059,445009330,507990021,638119352,583659535,797628118,726387553,890018660,835552979,1015980042,944750013,1276238704,1221641927,1167319070,1095957929,1595256236,1540665371,1452775106,1381403509,1780037320,1859660671,1671105958,1733955601,2031960084,2111593891,1889500026,1952343757,2552477408,2632100695,2443283854,2506133561,2334638140,2414271883,2191915858,2254759653,3190512472,3135915759,3081330742,3009969537,2905550212,2850959411,2762807018,2691435357,3560074640,3505614887,3719321342,3648080713,3342211916,3287746299,3467911202,3396681109,4063920168,4143685023,4223187782,4286162673,3779000052,3858754371,3904687514,3967668269,881225847,809987520,1023691545,969234094,662832811,591600412,771767749,717299826,311336399,374308984,453813921,533576470,25881363,88864420,134795389,214552010,2023205639,2086057648,1897238633,1976864222,1804852699,1867694188,1645340341,1724971778,1587496639,1516133128,1461550545,1406951526,1302016099,1230646740,1142491917,1087903418,2896545431,2825181984,2770861561,2716262478,3215044683,3143675388,3055782693,3001194130,2326604591,2389456536,2200899649,2280525302,2578013683,2640855108,2418763421,2498394922,3769900519,3832873040,3912640137,3992402750,4088425275,4151408268,4197601365,4277358050,3334271071,3263032808,3476998961,3422541446,3585640067,3514407732,3694837229,3640369242,1762451694,1842216281,1619975040,1682949687,2047383090,2127137669,1938468188,2001449195,1325665622,1271206113,1183200824,1111960463,1543535498,1489069629,1434599652,1363369299,622672798,568075817,748617968,677256519,907627842,853037301,1067152940,995781531,51762726,131386257,177728840,240578815,269590778,349224269,429104020,491947555,4046411278,4126034873,4172115296,4234965207,3794477266,3874110821,3953728444,4016571915,3609705398,3555108353,3735388376,3664026991,3290680682,3236090077,3449943556,3378572211,3174993278,3120533705,3032266256,2961025959,2923101090,2868635157,2813903052,2742672763,2604032198,2683796849,2461293480,2524268063,2284983834,2364738477,2175806836,2238787779,1569362073,1498123566,1409854455,1355396672,1317987909,1246755826,1192025387,1137557660,2072149281,2135122070,1912620623,1992383480,1753615357,1816598090,1627664531,1707420964,295390185,358241886,404320391,483945776,43990325,106832002,186451547,266083308,932423249,861060070,1041341759,986742920,613929101,542559546,756411363,701822548,3316196985,3244833742,3425377559,3370778784,3601682597,3530312978,3744426955,3689838204,3819031489,3881883254,3928223919,4007849240,4037393693,4100235434,4180117107,4259748804,2310601993,2373574846,2151335527,2231098320,2596047829,2659030626,2470359227,2550115596,2947551409,2876312838,2788305887,2733848168,3165939309,3094707162,3040238851,2985771188],o=[0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535,131071,262143,524287,1048575,2097151,4194303,8388607,16777215,33554431,67108863,134217727,268435455,536870911,1073741823,-2147483648];function e(t){const o=[];for(let e=0;e<t.length;e+=1)o.push([e,t[e]]);o.push([t.length,-1]);const e=[];let r=o[0][0],n=o[0][1];for(let t=0;t<o.length;t+=1){const s=o[t][0],f=o[t][1];if(n)for(let t=r;t<s;t+=1)e.push({code:t,bits:n,symbol:void 0});if(r=s,n=f,-1===f)break}e.sort((t,o)=>t.bits-o.bits||t.code-o.code);let s=0,f=-1;const l=[];let c;for(let t=0;t<e.length;t+=1){const o=e[t];f+=1,o.bits!==s&&(f<<=o.bits-s,c=l[s=o.bits]={}),o.symbol=f,c[f]=o}return{table:e,fastAccess:l}}function r(t,o){if(o<0||o>=t.length)throw RangeError("Out of bound");const e=t.slice();t.sort((t,o)=>t-o);const r={};for(let o=t.length-1;o>=0;o-=1)r[t[o]]=o;const n=[];for(let o=0;o<t.length;o+=1)n.push(r[e[o]]++);let s;const f=t[s=o],l=[];for(let o=1;o<t.length;o+=1){const o=t[s=n[s]];void 0===o?l.push(255):l.push(o)}return l.push(f),l.reverse(),l}const n={decompress:function(n,s=!1){let f=0,l=0,c=0;const i=t=>{if(t>=32){const o=t>>1;return i(o)*(1<<o)+i(t-o)}for(;c<t;)l=(l<<8)+n[f],f+=1,c+=8;const e=o[t],r=l>>c-t&e;return l&=~(e<<(c-=t)),r};if(16986!==i(16))throw new Error("Invalid magic");if(104!==i(8))throw new Error("Invalid method");let h=i(8);if(!(h>=49&&h<=57))throw new Error("Invalid blocksize");h-=48;let u=new Uint8Array(1.5*n.length),a=0,p=-1;for(;;){const h=i(48),w=0|i(32);if(54156738319193!==h){if(25779555029136===h){i(7&c);break}throw new Error("Invalid bz2 blocktype")}{if(i(1))throw new Error("do not support randomised");const h=i(24),d=[],g=i(16);for(let t=32768;t>0;t>>=1){if(!(g&t)){for(let t=0;t<16;t+=1)d.push(!1);continue}const o=i(16);for(let t=32768;t>0;t>>=1)d.push(!!(o&t))}const b=i(3);if(b<2||b>6)throw new Error("Invalid number of huffman groups");const m=i(15),y=[],E=Array.from({length:b},(t,o)=>o);for(let t=0;t<m;t+=1){let t=0;for(;i(1);)if((t+=1)>=b)throw new Error("MTF table out of range");const o=E[t];for(let o=t;o>0;E[o]=E[--o]);y.push(o),E[0]=o}const v=d.reduce((t,o)=>t+o,0)+2,A=[];for(let t=0;t<b;t+=1){let t=i(5);const o=[];for(let e=0;e<v;e+=1){if(t<0||t>20)throw new Error("Huffman group length outside range");for(;i(1);)t-=2*i(1)-1;o.push(t)}A.push(e(o))}const k=[];for(let t=0;t<d.length-1;t+=1)d[t]&&k.push(t);let I,z,O=0,C=0,R=0,U=0;const $=[];for(;;){(O-=1)<=0&&(O=50,C<=y.length&&(I=A[y[C]],C+=1));for(const t in I.fastAccess)if(Object.prototype.hasOwnProperty.call(I.fastAccess,t)&&(c<t&&(l=(l<<8)+n[f],f+=1,c+=8),z=I.fastAccess[t][l>>c-t])){l&=o[c-=t],z=z.code;break}if(z>=0&&z<=1)0===R&&(U=1),R+=U<<z,U<<=1;else{{const t=k[0];for(;R>0;R-=1)$.push(t)}if(z===v-1)break;{const t=k[z-1];for(let t=z-1;t>0;k[t]=k[--t]);k[0]=t,$.push(t)}}}const j=r($,h);let x=0;for(;x<j.length;){const o=j[x];let e=1;if(x<j.length-4&&j[x+1]===o&&j[x+2]===o&&j[x+3]===o?(e=j[x+4]+4,x+=5):x+=1,a+e>=u.length){const t=u;(u=new Uint8Array(2*t.length)).set(t)}for(let r=0;r<e;r+=1)s&&(p=p<<8^t[255&(p>>24^o)]),u[a]=o,a+=1}if(s){const t=-1^p;if(t!==w)throw new Error(`CRC mismatch: ${t} !== ${w}`);p=-1}}}return u.subarray(0,a)}};"undefined"!=typeof window?window.bz2=n:module.exports=n}();
