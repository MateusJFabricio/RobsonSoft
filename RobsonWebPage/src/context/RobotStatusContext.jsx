import { createContext, useRef, useState, useEffect, useContext } from "react";
import {JoystickContext} from './JoystickContext'

export const RobotStatusContext = createContext();

export const RobotStatusContextProvider = ({children})=>{
    const [robotIp, setRobotIp] = useState("")
    const robotIpRef = useRef("")
    const {joystick} = useContext(JoystickContext);
    const joystickRef = useRef(joystick)
    const enableRobot = useRef(false);
    const [controlData, setControlData] = useState(
        {
            MOVE_ANGLE:{
                MOVE_X: 0,
                MOVE_Y: 0,
                MOVE_Z: 0,
                SPEED: 0, //°/s
                RAMP: 0,
            },
            MOVE_LINEAR:{
                MOVE_X: 0,
                MOVE_Y: 0,
                MOVE_Z: 0,
                SPEED: 0, //mm/s
                RAMP: 0,
            },
            MOVE_JOINT: {
                JOINT1: 0,
                JOINT2: 0,
                JOINT3: 0,
                JOINT4: 0,
                JOINT5: 0,
                SPEED: 0, //°/s
                RAMP: 0,
            },
            GRIPPER: {
                OPEN: false,
                CLOSE: false
            },
            GENERAL: {
                DEAD_MAN: false,
                START_AUTOMATIC: false
            }
          }
    )
    const [robotStatus, setRobotStatus] = useState({
        MODO_OPERACAO: {
            MANUAL: false,
            MANUAL_CEM: false,
            AUTOMATICO: false
        },
        DEAD_MAN: false,
        SPEED: 0,
        MOVE_TYPE:{
            MOVE_LINEAR: false,
            MOVE_JOINT: false,
            MOVE_ANGLE: false
        }
    })
    const getControlData = ()=>{
      //Enable
      if (!enableRobot.current && 
        joystickRef.current.buttons[7].value === 1 && 
        joystickRef.current.buttons[6].value === 1)
        {
            enableRobot.current = true;
            joystickRef.current.vibrationActuator.playEffect("dual-rumble", {
            startDelay: 0,
            duration: 200,
            weakMagnitude: 1.0,
            strongMagnitude: 1.0,
            });
        }
        if (enableRobot.current && (
        joystickRef.current.buttons[7].value !== 1 || 
        joystickRef.current.buttons[6].value !== 1)){
            enableRobot.current = false;
            joystickRef.current.vibrationActuator.playEffect("dual-rumble", {
            startDelay: 0,
            duration: 200,
            weakMagnitude: 1.0,
            strongMagnitude: 1.0,
            });
        }

      return {
        MOVE_ANGLE:{
            MOVE_X: joystickRef.current.axes[0].toFixed(2),
            MOVE_Y: joystickRef.current.axes[1].toFixed(2) * -1,
            MOVE_Z: joystickRef.current.axes[2].toFixed(2),
        },
        MOVE_LINEAR:{
            MOVE_X: joystickRef.current.axes[0].toFixed(2),
            MOVE_Y: joystickRef.current.axes[1].toFixed(2) * -1,
            MOVE_Z: joystickRef.current.axes[2].toFixed(2),
        },
        MOVE_JOINT: {
            JOINT1: getAxis1Value(),
            JOINT2: joystickRef.current.axes[0].toFixed(2),
            JOINT3: joystickRef.current.axes[1].toFixed(2) * -1,
            JOINT4: joystickRef.current.axes[2].toFixed(2),
            JOINT5: joystickRef.current.axes[3].toFixed(2) * -1,
        },
        GRIPPER: {
            OPEN: joystickRef.current.buttons[14].pressed,
            CLOSE: joystickRef.current.buttons[15].pressed,
            MOVE_OPEN: joystickRef.current.buttons[12].pressed,
            MOVE_CLOSE: joystickRef.current.buttons[13].pressed
        },
        GENERAL: {
            DEAD_MAN: enableRobot.current,
            START_AUTOMATIC: false
        }
      }
    }

    const getAxis1Value = ()=>{
        if (joystickRef.current.buttons[0].pressed){
            return -1;
        }

        if (joystickRef.current.buttons[3].pressed){
            return 1;
        }

        return 0;
    }

    useEffect(() => {
      joystickRef.current = joystick;
      if (joystickRef.current !== null){
        let contData = getControlData()
        setControlData(contData);

        let statusTemp = robotStatus;
        //statusTemp.DEAD_MAN = contData.GENERAL.DEAD_MAN;
        setRobotStatus(statusTemp)
      }
    }, [joystick])

    const SetModoOperacao = (manual, manual_cem, auto)=>{
        let statusTemp = robotStatus;
        statusTemp.MODO_OPERACAO = {
            MANUAL: manual,
            MANUAL_CEM: manual_cem,
            AUTOMATICO: auto
        }

        sendRobotStatus(statusTemp);
    }
    const SetMoveType = (joint, linear, rotational)=>{
        let statusTemp = robotStatus;
        statusTemp.MOVE_TYPE = {
            MOVE_LINEAR: linear,
            MOVE_JOINT: joint,
            MOVE_ANGLE: rotational
        }

        sendRobotStatus(statusTemp);
    }
    const SetSpeed = (speed)=>{
        let statusTemp = robotStatus;
        statusTemp.SPEED = speed;
        console.log(statusTemp)
        sendRobotStatus(statusTemp);
    }
    useEffect(() => {
        const interval = setInterval(()=>{
            if (robotIpRef.current !== ""){
                try{
                    fetch("http://" + robotIpRef.current + ":80/getRobotStatus")
                    .then((response) => response.json())
                    .then((json) => {
                        setRobotStatus(json);
                    })
                }catch{
    
                }
            }
          }
        , 1000);
        
        return () =>{ 
          clearInterval(interval)
        }
      }, []) 

      useEffect(() => {
        robotIpRef.current = robotIp;
      }, [robotIp])
      

    const sendRobotStatus = (json) => {
        if (robotIp !== ""){
            
            if (robotStatus != null){
                try{
                    fetch("http://" + robotIp + ":80/postRobotState", {
                    method: "POST",
                    body: JSON.stringify(json),
                    headers: {
                        "Content-type": "text/plain;"
                    }
                    })
                    .then((response) => response.json())
                    
                }catch(erro){
                    console.log(erro)
                }
            }
        }
    }
    
    return(
        <RobotStatusContext.Provider value={{controlData, robotStatus, SetModoOperacao, SetMoveType, SetSpeed, setRobotIp}}>
            {children}
        </RobotStatusContext.Provider >
    )
}