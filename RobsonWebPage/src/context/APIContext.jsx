import { createContext, useRef, useState, useEffect, useContext } from "react";
import useWebSocket, { ReadyState } from 'react-use-websocket';
import {JoystickContext} from './JoystickContext'
import { RobotStatusContext } from './RobotStatusContext';

export const ApiContext = createContext();

export const ApiContextProvider = ({children})=>{
    const {controlData, robotStatus, SetModoOperacao, SetMoveType, SetSpeed, setRobotIp} = useContext(RobotStatusContext)
    const {joystick} = useContext(JoystickContext);
    const joystickRef = useRef(joystick)
    const controlDataRef = useRef(controlData)
    const connectionStatusRef = useRef('Closed')
    const [socketUrl, setSocketUrl] = useState("ws://" + (localStorage.getItem('IP_ROBOT') || 'robsonsoft') + ":81/");
    const [ip, setIp] = useState((localStorage.getItem('IP_ROBOT') || 'robsonsoft'));
    const enableRobot = useRef(false);

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
      setRobotIp(ip);
    }

    useEffect(() => {
      const interval = setInterval(()=>{
        if(joystickRef.current !== null){
          if (connectionStatusRef.current === 'Open'){
            let ctrlData = controlDataRef.current;
            console.log(ctrlData.MOVE_JOINT.JOINT3)
            sendMessage(JSON.stringify(ctrlData));
          }
        }
      }, 50);
      
      return () =>{ 
        clearInterval(interval)
      }
    }, [])

    /* useEffect(() => {
      const interval = setInterval(()=>{
        if (connectionStatusRef.current === 'Open'){
          sendMessage(JSON.stringify({lifebit: true}))
        }
      }, 1000);
      
      return () =>{ 
        clearInterval(interval)
      }
    }, []) */  

    useEffect(() => {
      joystickRef.current = joystick;
    }, [joystick])

    useEffect(() => {
        controlDataRef.current = controlData;
      }, [controlData])

    useEffect(() => {
      connectionStatusRef.current = connectionStatus;
      setRobotIp(ip);
    }, [connectionStatus])
    

    return(
        <ApiContext.Provider value={{setUrl, connectionStatus, ip}}>
            {children}
        </ApiContext.Provider >
    )
}