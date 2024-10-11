import React, { useContext } from 'react'
import { RobotStatusContext } from '../../../context/RobotStatusContext'

const DeadMan = () => {
    const {robotStatus} = useContext(RobotStatusContext)
  return (
    <div style={{
        display: 'flex',
        flexDirection: 'column',
        backgroundColor: robotStatus.DEAD_MAN ? 'green' : 'red',
        width: '150px',
        alignContent: 'center',
        alignItems: 'center',
        justifyContent: 'center',
        fontSize: '15px',
        color: 'white',
        lineHeight: '10px',
        padding: '5px',
        borderRadius: '5px',
    }}>
        <b>DeadMan</b>
    </div>
  )
}

export default DeadMan