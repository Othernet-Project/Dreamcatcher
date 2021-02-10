let myip = '192.168.4.1';

(function() {
    'use strict';

    // fake data for simulating files tree in case sd card is not connected and/or server returns error 404
    let files_tree_json = [{"path":"/files", "dir":1,"name":"Refresh", "size":"0"}, {"path":"/files/2020-07-29-19-26-18.004-VBoxSVC-57597.log", "dir":0,"name":"2020-07-29-19-26-18.004-VBoxSVC-57597.log", "size":"237"}, {"path":"/files/tmp", "dir":1,"name":"tmp", "size":"0"}, {"path":"/files/test path", "dir":1,"name":"test path", "size":"0"}, {"path":"/files/test3.py", "dir":0,"name":"test3.py", "size":"120570"}, {"path":"/files/test5.py", "dir":0,"name":"test5.py", "size":"120570"}, {"path":"/files/test7.py", "dir":0,"name":"test7.py", "size":"120570"}, {"path":"/files/test.py", "dir":0,"name":"test.py", "size":"120570"}, {"path":"/files/test4.py", "dir":0,"name":"test4.py", "size":"120570"}, {"path":"/files/test6.py", "dir":0,"name":"test6.py", "size":"120570"}, {"path":"/files/test8.py", "dir":0,"name":"test8.py", "size":"120570"}, {"path":"/files/opaks", "dir":1,"name":"opaks", "size":"0"}, {"path":"/files/test9.py", "dir":0,"name":"test9.py", "size":"120570"}, {"path":"/files/tinyxml2.h", "dir":0,"name":"tinyxml2.h", "size":"56710"}, {"path":"/files/test1.py", "dir":0,"name":"test1.py", "size":"120570"}, {"path":"/files/test2.py", "dir":0,"name":"test2.py", "size":"120570"}];

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

    function getfilestree(path)
    {
        console.log(path);
        xhr.open("GET", path + '/', true);
        xhr.send();    
    }


    function filestree(json){
        let tree = document.getElementById("filestree");
        tree.innerHTML = "<th>Name</th><th>Type</th><th>Size</th>";
    
        console.log(json);
        json.forEach(element => {
            let tr = document.createElement("tr");
            tr.className = 'filestree';
            let a = document.createElement("a");
            if (element.dir) {
                a.href = '#';
                a.target = "_self";
                a.link = element.path;
                a.onclick = function (event) {
                    event.preventDefault();
                    getfilestree(element.path);
                };
            } else{
                a.href = element.path;
                a.target = "_blank";
            }
            a.text = element.name;
            let td1 = document.createElement("td");
            td1.appendChild(a);
            let td2 = document.createElement("td");
            td2.innerHTML = element.dir?"directory":"file";
            let td3 = document.createElement("td");
            td3.innerHTML = element.size;
            td1.className = 'filestree';
            td2.className = 'filestree1';
            td3.className = 'filestree1';

            tr.appendChild(td1);
            tr.appendChild(td2);
            tr.appendChild(td3);
            tree.appendChild(tr);
        });
    }
    
    function postwifiap(path, ssid, pass, ap, auth)
    {
        const http = new XMLHttpRequest();
    
        console.log(path);
        var params = 'ssid=' + ssid + '&pass=' + pass + '&ap=' + ap + '&auth=' + auth;
        var url = '/wifi';
        http.open("POST", url, true);
        // Send the proper header information along with the request
        http.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    
        http.onreadystatechange = function() {//Call a function when the state changes.
            if(http.readyState == 4 && http.status == 200) {
                console.log('credentials set', http.responseText);
            }
        }
        http.send(params);
    }
    
    function postsettings(freq, bw, sf, cr)
    {
        const http = new XMLHttpRequest();
    
        var params = 'freq=' + freq + '&bw=' + bw + '&sf=' + sf + '&cr=' + cr;
        var url = '/settings';
        http.open("POST", url, true);
        // Send the proper header information along with the request
        http.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    
        http.onreadystatechange = function() {//Call a function when the state changes.
            if(http.readyState == 4 && http.status == 200) {
                console.log('settings set', http.responseText);
            }
        }
        http.send(params);
    }
    
    document.addEventListener('DOMContentLoaded', event => {

        getfilestree('/files');

        let stabtn = document.getElementById("sta_btn");
        let apbtn = document.getElementById("ap_btn");
        let enable_ap = document.getElementById("en_ap");
        let enable_sta = document.getElementById("en_sta");
        let settingsSend = document.getElementById("settingsSend");

        // Get the modal
        let modalstats = document.getElementById("myModal");
        let modalwifi = document.getElementById("myModal1");
        let modaltree = document.getElementById("myModal2");
        let spinner = document.getElementById("spinner");
        let settingsForm = document.getElementById("settingsForm");
        
        // Get the button that opens the modal
        let settingsBtn = document.getElementById("settingsBtn");
        let btnwifi = document.getElementById("wifi");
        let btntree = document.getElementById("tree");
        let btnformat = document.getElementById("format");

        // Settings fields
        let FREQ = document.querySelector("#freq1");
        let BW = document.querySelector("#bw1");
        let SF = document.querySelector("#sf1");
        let CR = document.querySelector("#cr1");

        try {
            FREQ.value = init_freq;
            BW.value = init_bw;
            SF.value = init_sf;
            CR.value = init_cr;           
        } catch (error) {}

        btnformat.onclick = function() {
            if(confirm("Do you want to format SD crad?\nIt will takes about 5 minutes!!!")){
                spinner.display = 'block';
                const http = new XMLHttpRequest();
                var url = '/format';
                http.open("POST", url, true);
            
                http.onreadystatechange = function() {//Call a function when the state changes.
                    if(http.readyState == 4 && http.status == 200) {
                        console.log('sd card formatted', http.responseText);
                        spinner.display = 'none';
                    }
                }
                http.send();
            }
        }
        settingsBtn.onclick = function() {
            settingsForm.style.display = "block";
        }
        btnwifi.onclick = function() {
            modalwifi.style.display = "block";
        }
        btntree.onclick = function() {
            modaltree.style.display = "block";
        }
        enable_ap.onclick = function(ev) {
            console.log('AP', ev);
            let content = document.getElementById("ap_content");
            if(ev.target.checked) content.style.display = "block";
            else content.style.display = "none";
        }
        enable_sta.onclick = function(ev) {
            let content = document.getElementById("sta_content");
            if(ev.target.checked) content.style.display = "block";
            else content.style.display = "none";
        }
        // Get the <span> element that closes the modal
        // let span1 = document.getElementsByClassName("close1")[0];
        let span2 = document.getElementById("close2");
        let span3 = document.getElementById("close3");
        let span4 = document.getElementById("close4");
        
        // When the user clicks on <span> (x), close the modal
        // span1.onclick = function() {
        //     modalstats.style.display = "none";
        // }
        span2.onclick = function() {
            modalwifi.style.display = "none";
        }
        span3.onclick = function() {
            modaltree.style.display = "none";
        }
        span4.onclick = function() {
            settingsForm.style.display = "none";
        }
        
        // When the user clicks anywhere outside of the modal, close it
        window.onclick = function(event) {
            if (event.target == modalstats) {
                modalstats.style.display = "none";
            }
            if (event.target == modalwifi) {
                modalwifi.style.display = "none";
            }
            if (event.target == modaltree) {
                modaltree.style.display = "none";
            }
            if (event.target == settingsForm) {
                settingsForm.style.display = "none";
            }
        }

        stabtn.addEventListener('click', function() {
            const ssid = document.querySelector("#sta_ssid").value;
            const pass = document.querySelector("#sta_pass").value;
            postwifiap('', ssid, pass, 0, 0);
        });

        apbtn.addEventListener('click', function() {
            const ssid = document.querySelector("#ap_ssid").value;
            const pass = document.querySelector("#ap_pass").value;
            var auth = document.getElementById("auth");
            postwifiap('', ssid, pass, 1, auth.value);
        });

        settingsSend.addEventListener('click', function() {
            console.log(FREQ.value, BW.value, SF.value, CR.value);
            postsettings(FREQ.value, BW.value, SF.value, CR.value);
        });

        // WEBSOCKET
        let webSocket = new WebSocket('ws://' + myip + '/ws');
        webSocket.onopen = function (event) {
            webSocket.send("get stats");
        };

        webSocket.onmessage = function (event) {
            updateStats(event.data);
            webSocket.send("get stats");
        };
    
    });

    function updateStats(data) {
        var msg = JSON.parse(data);
        try {
            let ldo = document.getElementById("ldo");
            ldo.checked = (msg.ldo&0x20)>0?true:false;
            let inrange = document.getElementById("range");
            inrange.checked = (msg.ldo&0x01)>0?true:false;
            let cable = document.getElementById("cable");
            cable.checked = (msg.ldo&0x02)>0?true:false;
        } catch (error) {}
        try {
            let volt = document.getElementById("volt");
            volt.innerText = msg.volt + 'V';
        } catch (error) {}


        try {
            let hw = document.getElementById("hw");
            hw.innerText = msg.hw;    
        } catch (error) {}
        try {
            let fw = document.getElementById("fw");
            fw.innerText = msg.fw;
        } catch (error) {}
        try {
            let received = document.getElementById("received");
            received.innerText = msg.received;            
        } catch (error) {}
        try {
            let rssi = document.getElementById("rssi");
            rssi.innerText = msg.rssi;
        } catch (error) {}
        try {
            let snr = document.getElementById("snr");
            snr.innerText = msg.snr;
        } catch (error) {}
        try {
            let crc = document.getElementById("crc");
            crc.innerText = msg.crc;
        } catch (error) {}
        try {
            let header = document.getElementById("header");
            bitrate.innerText = msg.bitrate;
        } catch (error) {}
        try {
            let bitrate = document.getElementById("bitrate");
            header.innerText = msg.header;
        } catch (error) {}
        try {
            let cpu = document.getElementById("cpu");
            cpu.innerText = msg.cpu + '%';
        } catch (error) {}
        try {
            let ram1 = document.getElementById("ram1");
            ram1.innerText = msg.heap;
        } catch (error) {}
        try {
            let ram2 = document.getElementById("ram2");
            ram2.innerText = msg.minheap;
        } catch (error) {}
        try {
            let ram3 = document.getElementById("ram3");
            ram3.innerText = msg.psram;
        } catch (error) {}
        try {
            let name = document.getElementById("file");
            name.innerHTML = '<a target="_blank" href="/files/' + msg.filepath + '/' + msg.filename + '">' + msg.filename + '</a>';
        } catch (error) {}
    }


})();
