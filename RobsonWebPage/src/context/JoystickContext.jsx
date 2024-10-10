import { createContext, useState, useEffect, useRef } from "react";
import { useGamepads } from 'react-gamepads';

export const JoystickContext = createContext();

export const JoystickContextProvider = ({children})=>{
    

    const [joystick, setJoystick] = useState(null);
    
    const handleGamepadDisconnected = (event) => {
        setJoystick(null);
    };

    window.addEventListener('gamepaddisconnected', handleGamepadDisconnected);

    useGamepads(gamepads => {
        if (gamepads != null){
            if (gamepads[0] !== 'undefined'){
                setJoystick(gamepads[0]);
            }else{
                setJoystick(gamepads[2]);
            }
        }else{
            setJoystick(null);
        }
    });

    return(
        <JoystickContext.Provider value={{joystick}}>
            {children}
        </JoystickContext.Provider >
    )
}