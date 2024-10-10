import { createContext, useRef, useState, useEffect, useContext } from "react";
import useWebSocket, { ReadyState } from 'react-use-websocket';
import {JoystickContext} from './JoystickContext'

export const ApiContext = createContext();

export const ApiContextProvider = ({children})=>{
    const {joystick} = useContext(JoystickContext);
    const joystickRef = useRef(joystick)
    const connectionStatusRef = useRef('Closed')
    const [socketUrl, setSocketUrl] = useState("wss://" + (localStorage.getItem('IP_ROBOT') || 'robsonsoft') + ":81/");
    const [ip, setIp] = useState((localStorage.getItem('IP_ROBOT') || 'robsonsoft'));
    const enableRobot = useRef(false);
    const stopRobot = useRef(false);

    const { sendMessage, lastMessage, readyState } = useWebSocket(socketUrl, {
        onError: (event)=>{
        },
        shouldReconnect: (closeEvent) => {
          return true;
        },
        heartbeat: true,
        reconnectAttempts: 2,
        reconnectInterval: 1000,
      });
    
    const connectionStatus = {
        [ReadyState.CONNECTING]: 'Connecting',
        [ReadyState.OPEN]: 'Open',
        [ReadyState.CLOSING]: 'Closing',
        [ReadyState.CLOSED]: 'Closed',
        [ReadyState.UNINSTANTIATED]: 'Uninstantiated',
      }[readyState];

    const setUrl = (ip)=>{
      localStorage.setItem('IP_ROBOT', ip);
      setSocketUrl("ws://"+ ip + ":81");
      setIp(ip);
    }
    
    const controlData = ()=>{
      //Enable
      if(joystickRef.current.buttons[5].pressed){
        enableRobot.current = true;
        joystickRef.current.vibrationActuator.playEffect("dual-rumble", {
          startDelay: 0,
          duration: 200,
          weakMagnitude: 1.0,
          strongMagnitude: 1.0,
        });
      }

      if(joystickRef.current.buttons[4].pressed){
        enableRobot.current = false;
        joystickRef.current.vibrationActuator.playEffect("dual-rumble", {
          startDelay: 0,
          duration: 200,
          weakMagnitude: 1.0,
          strongMagnitude: 1.0,
        });
      }
      //joystickRef.current.axes[0],
      return {
        MOVE_TYPE:{
            MOVE_ANGLE: false,
            MOVE_JOINT: true,
            MOVE_LINEAR: false
        },
        MOVE_ANGLE:{
            MOVE_X: joystickRef.current.axes[0],
            MOVE_Y: joystickRef.current.axes[1] * -1,
            MOVE_Z: joystickRef.current.axes[2],
            SPEED: 10, //°/s
        },
        MOVE_LINEAR:{
            MOVE_X: joystickRef.current.axes[0],
            MOVE_Y: joystickRef.current.axes[1] * -1,
            MOVE_Z: joystickRef.current.axes[2],
            SPEED: 10, //mm/s
        },
        MOVE_JOINT: {
            MOVE: joystickRef.current.axes[2],
            JOINT_NUMBER: 0,
            SPEED: 10, //°/s
        },
        GENERAL: {
            HOMEM_MORTO: false,
            ENABLE_MANUAL_MOVE: enableRobot.current,
            START_AUTOMATIC: false
        }
      }
    }

    useEffect(() => {
      const interval = setInterval(()=>{
        if(joystickRef.current !== null){
          if (connectionStatusRef.current === 'Open'){
            let ctrlData = controlData();
            sendMessage(JSON.stringify(ctrlData));
          }
        }
      }, 50);
      
      return () =>{ 
        clearInterval(interval)
      }
    }, [])

    useEffect(() => {
      const interval = setInterval(()=>{
        if (connectionStatusRef.current === 'Open'){
          sendMessage(JSON.stringify({lifebit: true}))
        }
      }, 1000);
      
      return () =>{ 
        clearInterval(interval)
      }
    }, [])  

    useEffect(() => {
      joystickRef.current = joystick;
    }, [joystick])

    useEffect(() => {
      connectionStatusRef.current = connectionStatus;
    }, [connectionStatus])

    return(
        <ApiContext.Provider value={{setUrl, connectionStatus, ip}}>
            {children}
        </ApiContext.Provider >
    )
}