import { createContext, useState, useEffect, useRef } from "react";

export const MenuLateralContext = createContext();

export const MenuLateralContextProvider = ({children})=>{

    return(
        <MenuLateralContext.Provider value={{joystick}}>
            {children}
        </MenuLateralContext.Provider >
    )
}