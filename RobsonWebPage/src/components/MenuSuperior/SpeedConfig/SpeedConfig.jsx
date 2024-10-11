import React, {useState, useContext} from 'react'
import Dropdown from 'react-bootstrap/Dropdown';
import { RobotStatusContext } from '../../../context/RobotStatusContext';

const SpeedConfig = () => {
    const [speed, setSpeed] = useState(100)
    const {controlData, robotStatus, SetModoOperacao, SetMoveType, SetSpeed} = useContext(RobotStatusContext)

    const changeSpeed = (speed)=>{
        setSpeed(speed)
        SetSpeed(speed)
    }
  return (
    <div style={{
        display: 'flex',
        flexDirection: 'column',
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
        <Dropdown className="d-inline mx-">
        <Dropdown.Toggle id="dropdown-autoclose-true" variant="success" style={{width: '150px'}}>
            {"Speed : " + speed + "%"}
        </Dropdown.Toggle>
  
        <Dropdown.Menu>
          <Dropdown.Item onClick={()=>{changeSpeed(1)}} >1%</Dropdown.Item>
          <Dropdown.Item onClick={()=>{changeSpeed(5)}}  >5%</Dropdown.Item>
          <Dropdown.Item onClick={()=>{changeSpeed(10)}}  >10%</Dropdown.Item>
          <Dropdown.Item onClick={()=>{changeSpeed(25)}}  >25%</Dropdown.Item>
          <Dropdown.Item onClick={()=>{changeSpeed(50)}}  >50%</Dropdown.Item>
          <Dropdown.Item onClick={()=>{changeSpeed(100)}}  >100%</Dropdown.Item>
        </Dropdown.Menu>
      </Dropdown>
    </div>
  )
}

export default SpeedConfig