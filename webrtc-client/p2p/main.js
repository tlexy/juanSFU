'use strict';

let localVideo = document.querySelector('#localVideo');
let remoteVideo = document.querySelector('#remoteVideo');

let remote_uid = "";

const SIGNAL_TYPE_JOIN = "join";
const SIGNAL_TYPE_RESP_JOIN = "resp-join"; 
const SIGNAL_TYPE_LEAVE = "leave";
const SIGNAL_TYPE_RESP_PUBLISH = "resp-publish";
const SIGNAL_TYPE_RESP_PULL = "resp-pullstream";

// function initLocalStream2(stream)
// {
//     console.log("step 1")
//     this.local_stream = stream;
//     localVideo.srcObject = this.local_stream;
// }

class ZalRtcPeer {
    constructor(roomId, localUid, ws) {
        this.room_id = roomId;
        this.local_uid = localUid;
        this.remote_uid = -1;
        this.pc = null;
        this.local_stream = null;
        this.remote_stream = null;
        this.ws_connection = ws;
        this.local_desc = null;
    }

    CreatePeerConnection() 
    {
        let conf = {
            bundlePolicy: "max-bundle",
            rtcpMuxPolicy: "require",
            iceTransportPolicy: "all"
        };
        this.pc = new RTCPeerConnection(conf);

       // this.pc.onicecandidate = this.handleIceCandidate.bind(this);
        this.pc.ontrack = this.handleRemoteStreamAdd.bind(this);

        this.local_stream.getTracks().forEach(track => this.pc.addTrack(track, this.local_stream));
    }

    //加入房间
    SendJoin() {
        let jsonMsg = {
            'cmd': 'join',
            'roomId': this.room_id,
            'uid': this.local_uid,
        };
        let msg = JSON.stringify(jsonMsg);
        this.ws_connection.send(msg);
    }

    sendSdpOffer() 
    {
        var jsonMsg = {
            'cmd': 'publish',
            'roomId': this.room_id,
            'uid': this.local_uid,
            'sdp': JSON.stringify(this.local_desc)
        };
        if (remote_uid.length > 0)
        {
            jsonMsg = {
                'cmd': 'pullstream',
                'roomId': this.room_id,
                'uid': this.local_uid,
                'remote_uid': this.remote_uid,
                'sdp': JSON.stringify(this.local_desc)
            };
        }
        var message = JSON.stringify(jsonMsg);
        this.ws_connection.send(message);
        console.info("send offer message: " + message);
    }

    doCreateOffer(session) 
    {
        this.local_desc = session;
        this.pc.setLocalDescription(session).then(this.sendSdpOffer.bind(this)).catch(function (e) {
            console.error("doCreateOffer offer setLocalDescription failed: " + e);
        });
    }

    CreateOffer(remoteUid) 
    { 
        this.remote_uid = remoteUid;
        if (this.pc == null)
        {
            this.CreatePeerConnection();
        }
        let oo = {
            "offerToReceiveAudio": false,
            "offerToReceiveVideo": false
        }

        let oo2 = {
            "offerToReceiveAudio": true,
            "offerToReceiveVideo": true
        }
        
        if (remoteUid > 0)
        {
            oo = oo2;
        }
        this.pc.createOffer(oo).then(this.doCreateOffer.bind(this)).catch(function(e){
            console.error("handleCreateOfferError: " + e);
        });
    }

    SetRemoteSdp(sdp)
    {
        //var desc = JSON.parse(sdp);
        this.pc.setRemoteDescription(sdp);
    }

    addIceCandidate(candis)
    {
        let candidate = JSON.parse(candis);
        this.pc.addIceCandidate(candidate)
            .catch(function(e){
                console.error("addIceCandidate failed: " + e);
            });
    }

    handleRemoteStreamAdd(ev)
    {
        console.info("handleRemoteStreamAdd");
        this.remote_stream = ev.streams[0];
        remoteVideo.srcObject = this.remote_stream;//this.local_stream;//
    }

    initLocalStream(stream)
    {
        console.log("step 1")
        this.local_stream = stream;
        localVideo.srcObject = this.local_stream;
        this.SendJoin();//流初始化发送加入房间信令
    }

    OpenLocalStream()
    {
        let fb = this.initLocalStream.bind(this);
        navigator.mediaDevices.getUserMedia({
            audio: true,
            video: true
        })
        .then(fb)//this.initLocalStream
        .catch(function(e){
            alert("getUserMedia error: " + e.name);
        });
    }
}

//管理类
class ZalRtc
{
    constructor(wsAddr)
    {
        this.ws_connection = null;
        this.ws_addr = wsAddr;
        this.room = new Map();
        this.room_id = -1;
        this.local_uid = -1;
    }

    CreateToServer() 
    {
        this.ws_connection = new WebSocket(this.ws_addr);
        this.ws_connection.onopen = function () {
            console.log("websocket open.");
        }
        this.ws_connection.onerror = function (ev) {
            console.log("websocket error.");
        }

        this.ws_connection.onclose = function (ev) {
            console.log("websocket close.");
        }

        let fb = this.OnServerMsg.bind(this);
        this.ws_connection.onmessage = fb;
    }

    //websocket处理函数
    OnServerMsg(ev) 
    {
        //console.log("onMessage: " + ev.data);
        let json = JSON.parse(ev.data);
        switch (json.cmd) {
            case SIGNAL_TYPE_RESP_JOIN:
                this.handleResponseJoin(json);
                break;
            case SIGNAL_TYPE_RESP_PUBLISH:
                this.handleResponsePublish(json);
                break;
            case SIGNAL_TYPE_RESP_PULL:
                this.handleResponsePullStream(json);
                break;
        }
    }

    publishStream()
    {
        let ZalRtcPeerObj = this.room.get(this.local_uid);
        if (ZalRtcPeerObj)
        {
            ZalRtcPeerObj.CreateOffer(-1);
        }
        else 
        {
            console.log("handleRemoteNewPeer peer obj is null");
        }
    }

    pullStream()
    {
        let ZalRtcPeerObj = this.room.get(this.local_uid);
        if (ZalRtcPeerObj)
        {
            ZalRtcPeerObj.CreateOffer(remote_uid);
        }
        else 
        {
            console.log("handleRemoteNewPeer peer obj is null");
        }
    }

    //加入房间成功
    handleResponseJoin(json) 
    {
        console.log("step 2");
        let uid = json.uid;
        let roomid = json.roomId;
        if (!this.room.has(uid) || roomid != this.room_id)
        {
            alert("resp, uid: " + uid + ", roomid: " + roomid);
            return;
        }
        console.log("room size: " + this.room.size + ", uid: " + uid);
        if (json.members.length > 0)
        {
            remote_uid = json.members[0]["uid"];
            console.log("remote uid: " + remote_uid);
        }
    }

    handleResponsePublish(json)
    {
        console.log("handleResponsePublish");
        console.log(json.sdp.sdp);
        let roomid = json.roomId;
        if (roomid != this.room_id) {
            console.error("handleResponsePublish, roomid error: " + roomid);
            return;
        }
        let ZalRtcPeerObj = this.room.get(json.uid);
        if (typeof (ZalRtcPeerObj) == "undefined") {
            console.error("handleResponsePublish, user not found, uid: " + uid);
            return;
        }
        ZalRtcPeerObj.SetRemoteSdp(json.sdp);
    }

    //对端离开
    handleRemotePeerLeave(json) { }

    handleResponsePullStream(json) 
    { 
        console.log("handleResponsePublish");
        console.log(json.sdp.sdp);
        let roomid = json.roomId;
        if (roomid != this.room_id) {
            console.error("handleResponsePublish, roomid error: " + roomid);
            return;
        }
        let ZalRtcPeerObj = this.room.get(json.uid);
        if (typeof (ZalRtcPeerObj) == "undefined") {
            console.error("handleResponsePublish, user not found, uid: " + uid);
            return;
        }
        ZalRtcPeerObj.SetRemoteSdp(json.sdp);
    }

    //主动的动作
    JoinRoom(roomId, Uid)
    {
        this.room_id = roomId;
        let peer = new ZalRtcPeer(roomId, Uid, this.ws_connection);
        peer.OpenLocalStream();
        //peer.SendJoin(peer);
        this.room.set(Uid, peer);
        this.local_uid = Uid;
    }

}

let zal_rtc = new ZalRtc("ws://192.168.110.78:5000/signaling");
zal_rtc.CreateToServer();

//action
document.getElementById('joinBtn').onclick = function () {
    let roomId = document.getElementById('roomId').value;
    if (roomId == "" || roomId == "请输入房间ID") {
        alert("请输入房间ID");
        return;
    }

    let uid = document.getElementById('uid').value;
    if (uid == "" || uid == "请输入用户ID") {
        alert("请输入用户ID");
        return;
    }
    if (isNaN(Number(roomId)) || isNaN(Number(uid))) {
        alert("not a number...");
        return;
    }
    console.log("加入按钮被点击, roomId: " + roomId);
    zal_rtc.JoinRoom(roomId, uid);
}

document.getElementById('leaveBtn').onclick = function () {
    console.log("离开按钮被点击");
    doLeave();
}

document.getElementById('publishBtn').onclick = function () {
    console.log("发布按钮被点击");
    zal_rtc.publishStream();
}

document.getElementById('pullBtn').onclick = function () {
    console.log("拉流按钮被点击");
    zal_rtc.pullStream();
}