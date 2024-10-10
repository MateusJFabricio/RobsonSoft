import React, {useContext} from 'react'
import PS4Joystick from '../../components/PS4Joystick/PS4Joystick'
import {JoystickContext} from '../../context/JoystickContext'

const ControlRobot = () => {
    const {joystick} = useContext(JoystickContext);
  return (
    <div>
        <PS4Joystick joystick={joystick}/>
    </div>
  )
}

export default ControlRobot