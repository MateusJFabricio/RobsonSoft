import { createContext, useRef, useState, useEffect, useContext } from "react";
import {JoystickContext} from './JoystickContext'

export const RobotStatusContext = createContext();

export const RobotStatusContextProvider = ({children})=>{
    const {joystick} = useContext(JoystickContext);
    const joystickRef = useRef(joystick)
    const enableRobot = useRef(false);
    const [controlData, setControlData] = useState(
        {
            MOVE_TYPE:{
                MOVE_ANGLE: false,
                MOVE_JOINT: false,
                MOVE_LINEAR: false
            },
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
        MOVE_TYPE:{
            LINEAR: false,
            JOINT: false,
            ROTATIONAL: false
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
        MOVE_TYPE:{
            MOVE_ANGLE: controlData.MOVE_TYPE.MOVE_ANGLE,
            MOVE_JOINT: controlData.MOVE_TYPE.MOVE_JOINT,
            MOVE_LINEAR: controlData.MOVE_TYPE.MOVE_LINEAR
        },
        MOVE_ANGLE:{
            MOVE_X: joystickRef.current.axes[0],
            MOVE_Y: joystickRef.current.axes[1] * -1,
            MOVE_Z: joystickRef.current.axes[2],
            SPEED: controlData.MOVE_ANGLE.SPEED, //°/s
            RAMP: 5,
        },
        MOVE_LINEAR:{
            MOVE_X: joystickRef.current.axes[0],
            MOVE_Y: joystickRef.current.axes[1] * -1,
            MOVE_Z: joystickRef.current.axes[2],
            SPEED: controlData.MOVE_LINEAR.SPEED, //mm/s
            RAMP: 5,
        },
        MOVE_JOINT: {
            JOINT1: getAxis1Value(),
            JOINT2: joystickRef.current.axes[0],
            JOINT3: joystickRef.current.axes[1],
            JOINT4: joystickRef.current.axes[2],
            JOINT5: joystickRef.current.axes[3],
            SPEED: controlData.MOVE_JOINT.SPEED, //°/s
            RAMP: 5,
        },
        GRIPPER: {
            OPEN: false,
            CLOSE: false
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
        SetDeadMan(contData.GENERAL.DEAD_MAN);
      }
    }, [joystick])

    const SetModoOperacao = (manual, manual_cem, auto)=>{
        let statusTemp = robotStatus;
        statusTemp.MODO_OPERACAO = {
            MANUAL: manual,
            MANUAL_CEM: manual_cem,
            AUTOMATICO: auto
        }

        setRobotStatus(statusTemp)
    }
    const SetMoveType = (joint, linear, rotational)=>{
        let statusTemp = robotStatus;
        statusTemp.MOVE_TYPE = {
            MOVE_LINEAR: linear,
            MOVE_JOINT: joint,
            MOVE_ANGLE: rotational
        }

        setRobotStatus(statusTemp)
    }
    const SetDeadMan = (deadMan)=>{
        let statusTemp = robotStatus;
        statusTemp.DEAD_MAN = deadMan;

        setRobotStatus(statusTemp)
    }
    const SetSpeed = (speed)=>{
        let controlTemp = controlData;
        controlTemp.MOVE_ANGLE.SPEED = speed;
        controlTemp.MOVE_JOINT.SPEED = speed;
        controlTemp.MOVE_LINEAR.SPEED = speed;

        setControlData(controlTemp);
    }
    return(
        <RobotStatusContext.Provider value={{controlData, robotStatus, SetModoOperacao, SetMoveType, SetSpeed}}>
            {children}
        </RobotStatusContext.Provider >
    )
}