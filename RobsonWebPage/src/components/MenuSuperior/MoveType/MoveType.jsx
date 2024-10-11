import React, {useState, useContext, useEffect} from 'react'
import Dropdown from 'react-bootstrap/Dropdown';
import { RobotStatusContext } from '../../../context/RobotStatusContext';

const MoveType = () => {
    const [texto, setTexto] = useState("TIPO MOV")
    const {controlData, robotStatus, SetModoOperacao, SetMoveType} = useContext(RobotStatusContext)
    
    useEffect(() => {
    if (robotStatus !== null){

        if (!robotStatus.MOVE_TYPE.LINEAR && 
            !robotStatus.MOVE_TYPE.JOINT &&
            !robotStatus.MOVE_TYPE.ROTATIONAL){
                setTexto("TIPO MOV")
            }

        if (robotStatus.MOVE_TYPE.LINEAR){
            setTexto("LINEAR")
        }

        if (robotStatus.MOVE_TYPE.JOINT){
            setTexto("JOINT")
        }

        if (robotStatus.MOVE_TYPE.ROTATIONAL){
            setTexto("ROTACIONAL")
        }
    }
    }, [robotStatus])

    const SetMove = (joint, linear, rotational)=>{
        SetMoveType(joint, linear, rotational)

        if (joint){
            setTexto("JOINT")
        }

        if (linear){
            setTexto("LINEAR")
        }

        if (rotational){
            setTexto("ROTACIONAL")
        }
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
        <Dropdown.Toggle id="dropdown-autoclose-true" style={{width: '170px'}}>
            {texto}
        </Dropdown.Toggle>

        <Dropdown.Menu>
          <Dropdown.Item onClick={()=>{SetMove(true, false, false)}}>Joint</Dropdown.Item>
          <Dropdown.Item onClick={()=>{SetMove(false, true, false)}}>Linear</Dropdown.Item>
          <Dropdown.Item onClick={()=>{SetMove(false, false, true)}}>Rotacional</Dropdown.Item>
        </Dropdown.Menu>
      </Dropdown>
    </div>
  )
}

export default MoveType