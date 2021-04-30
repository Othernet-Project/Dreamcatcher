let myip = '192.168.4.1';

let files_tree_json = [{"path":"", "dir":1,"name":"Example Files", "size":"0"}, {"path":"/tests/textFile.txt", "dir":0,"name":"textFile.txt", "size":"123"}, {"path":"/tests/Othernet Satcalc by Tynet.eu min.html", "dir":0,"name":"Othernet Satcalc by Tynet.eu min.html", "size":"13123"}, {"path":"/tests/ee1b-wikipedia-Ajla_Tomljanović.html.tbz2", "dir":0,"name":"ee1b-wikipedia-Ajla_Tomljanović.html.tbz2", "size":"146461"}];

//Changes locale
function changeLocale(locale)
{
  jsI18n.setLocale(locale)
  jsI18n.processPage()
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
    };

// get filetree json from backend 
function getfilestree(path) {
    console.log(path);
    xhr.open("GET", path + '/', true);
    xhr.send();    
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
                viewFile(element.path, fileExt);
            };
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
    document.getElementById('stats_sdfree').innerText = Math.round(sdFree*100)/100 + ' MB';
    document.getElementById('stats_sdused').innerText = Math.round(jsnStats.used / (1024*1024)*100)/100  + ' MB';

    document.getElementById('stats_snr').innerText = jsnStats.snr;
    document.getElementById('stats_rssi').innerText = jsnStats.rssi;

    document.getElementById('stats_crc').innerText = jsnStats.crc;
    document.getElementById('stats_header').innerText = jsnStats.header;
    document.getElementById('stats_pkts').innerText = jsnStats.received;

    const progress = jsnStats.packet / jsnStats.packets * 100;
    document.getElementById('stats_progress').innerText = Math.round(progress*100)/100 + '%';
    document.getElementById('stats_file').innerText = jsnStats.filename;

    document.getElementById('stats_lnben').innerText = (jsnStats.ldo&0x20)>0?'YES':'NO';
    document.getElementById('stats_lnbir').innerText = (jsnStats.ldo&0x01)>0?'YES':'NO';
    document.getElementById('stats_lnbcon').innerText = (jsnStats.ldo&0x02)>0?'YES':'NO';
    document.getElementById('stats_lnbv').innerText = jsnStats.volt;
    document.getElementById('stats_offset').innerText = jsnStats.offset;    
}

// save Reeiver Settings
function saveReceiver() {
    let freq = document.getElementById('rcv_freq').value;
    let bw = document.getElementById('rcv_bw').value;
    let sf = document.getElementById('rcv_sf').value;
    let cr = document.getElementById('rcv_cr').value;
    let lnb = document.getElementById('lnb_volt').checked;
    let lo = document.getElementById('lo_en').checked;
    let diseq = document.getElementById('diseq').checked;
    let loid = document.getElementById('rcv_loid').value;

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
            document.getElementById('tag_saveRcv').classList.remove('is-hidden');
            setTimeout(function(){ document.getElementById('tag_saveRcv').classList.add('is-hidden'); }, 5000);
        }
    };
    http.send(params);
}

// send WIFI Creds to backend
function postWifiSettings(path, ssid, pass, ap, auth) {
    const http = new XMLHttpRequest();

    console.log(path);
    var params = 'ssid=' + ssid + '&pass=' + pass + '&ap=' + ap + '&auth=' + auth;
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
    };
    http.send(params);
    return true;
}

// save WIFI AP
function saveWifiAp() {
    let ssid = document.getElementById('ap_ssid').value;
    let pwd = document.getElementById('ap_pwd').value;
    let auth = document.getElementById('ap_auth').value;
    if(postWifiSettings('', ssid, pwd, 1, auth)){
        document.getElementById('tag_saveAp').classList.remove('is-hidden');
        setTimeout(function(){ document.getElementById('tag_saveAp').classList.add('is-hidden'); }, 5000);
    }
}

// save WIFI Client
function saveWifiClient() {
    let ssid = document.getElementById('sta_ssid').value;
    let pwd = document.getElementById('sta_pwd').value;
    if(postWifiSettings('', ssid, pwd, 0, 0)){
        document.getElementById('tag_saveSta').classList.remove('is-hidden');
        setTimeout(function(){ document.getElementById('tag_saveSta').classList.add('is-hidden'); }, 5000);
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
        };
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

// get File and open it in the Viewer
async function viewFile(filepath, fileExt) {
    document.getElementById('md_fileview_status').classList.add('loader');
    let result = 'File not Supported / Error while reading';

    let iframeElement = document.getElementById('data');
    iframeElement.src = "about:blank";

    if (fileExt == 'tbz2') {
        console.log('tbz2 File');
        result = await getTbz2(filepath);
    }
    if (fileExt == 'txt' || fileExt == 'json' || fileExt == 'html') {
        console.log('Text File');
        result = await getText(filepath);
    }

    // Set the iframe's new HTML
    iframeElement.contentWindow.document.open();
    iframeElement.contentWindow.document.write(result);
    iframeElement.contentWindow.document.close();
    document.getElementById('md_fileview_status').classList.remove('loader');
}

// ON Window loaded
document.addEventListener('DOMContentLoaded', event => {
    getfilestree('/files');

    document.getElementById('info_ip').innerText = myip;
    try {        
        document.getElementById('rcv_freq').value = init_freq;
        document.getElementById('rcv_bw').value = init_bw;
        document.getElementById('rcv_sf').value = init_sf;
        document.getElementById('rcv_cr').value = init_cr;
        document.getElementById('lnb_volt').checked = init_lnb;
        document.getElementById('lo_en').checked = init_lo;
        document.getElementById('diseq').checked = init_diseq;
        document.getElementById('rcv_loid').value = init_loid;
    } catch (error) {}

    let webSocket = new WebSocket('ws://' + window.location.host + '/ws');
    webSocket.onopen = function (event) {
        document.getElementById('con_status').classList = 'tag is-light is-success';
        document.getElementById('con_status').innerText = 'Connected';
        webSocket.send("get stats");
    };

    webSocket.onmessage = function (event) {
        updateStats(JSON.parse(event.data));
        setTimeout(function(){ webSocket.send("get stats"); }, 5000);
    };

    webSocket.onerror = function (event) { 
        document.getElementById('con_status').classList = 'tag is-light is-danger';
        document.getElementById('con_status').innerText = 'Connection Error';
    };
});