import React, {useContext} from 'react'
import {JoystickContext} from '../../../context/JoystickContext.jsx'

const JoystickConnected = () => {
    const {joystick} = useContext(JoystickContext);
  return (
    <div style={{
        display: 'flex',
        flexDirection: 'column',
        backgroundColor: joystick !== null ? 'green' : 'red',
        width: '150px',
        alignContent: 'center',
        alignItems: 'center',
        justifyContent: 'center',
        fontSize: '10px',
        color: 'white',
        lineHeight: '10px',
        padding: '5px',
        borderRadius: '5px',
    }}>
        <span>Joystick</span>
        <br />
        {joystick !== null ?  <span>Conectado</span> :  <span>Desconectado</span>}
    </div>
  )
}

export default JoystickConnected