class WebClient{
    constructor(){

    }
    start(){
        this.socket = new WebSocket("ws://localhost:3010/");
        this.socket.onmessage = (event)=>{
            let message = JSON.parse(event.data);
            switch(message.type)
            {
                case "tick":
                    if(this.onTick != undefined) this.onTick(parseInt(message.tick));
                    if(this.onOutCount != undefined) this.onOutCount(parseInt(message.outCount));
                    break;
                case "runningState":
                    if(this.onChangeState != undefined) this.onChangeState(message.state);
                    break;
            }
            
        };
        this.socket.onopen = (event)=>{
            if(this.onOpen != undefined) this.onOpen();
        };
        this.socket.onerror = (event)=>{
            if(this.onError != undefined) this.onError();
        };
        this.socket.onclose = (event)=>{
            if(this.onError != undefined) this.onClose();
        };
    }
}