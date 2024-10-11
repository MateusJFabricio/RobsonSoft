import React, {useState, useContext, useEffect} from 'react'
import Dropdown from 'react-bootstrap/Dropdown';
import { RobotStatusContext } from '../../../context/RobotStatusContext';

const OperationMode = () => {
  const [texto, setTexto] = useState("MODO")
    const {controlData, robotStatus, SetModoOperacao, SetMoveType} = useContext(RobotStatusContext)
    
    useEffect(() => {
    if (robotStatus !== null){

        if (!robotStatus.MODO_OPERACAO.MANUAL && 
            !robotStatus.MODO_OPERACAO.MANUAL_CEM &&
            !robotStatus.MODO_OPERACAO.AUTOMATICO){
                setTexto("MODO")
            }

        if (robotStatus.MODO_OPERACAO.MANUAL){
            setTexto("MANUL")
        }

        if (robotStatus.MODO_OPERACAO.MANUAL_CEM){
            setTexto("MANUAL 100%")
        }

        if (robotStatus.MODO_OPERACAO.AUTOMATICO){
            setTexto("AUTO")
        }
    }
    }, [robotStatus])

    const SetModo = (manual, manual_cem, auto)=>{
      SetModoOperacao(manual, manual_cem, auto)

        if (manual){
            setTexto("MANUAL")
        }

        if (manual_cem){
            setTexto("MANUAL 100%")
        }

        if (auto){
            setTexto("AUTO")
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
      <Dropdown.Toggle id="dropdown-autoclose-true" variant="success" style={{width: '150px'}}>
          {texto}
      </Dropdown.Toggle>

      <Dropdown.Menu>
        <Dropdown.Item onClick={()=>{SetModo(true, false, false)}}>Manual</Dropdown.Item>
        <Dropdown.Item onClick={()=>{SetModo(false, true, false)}}>Manual 100%</Dropdown.Item>
        <Dropdown.Item onClick={()=>{SetModo(false, false, true)}}>Auto</Dropdown.Item>
      </Dropdown.Menu>
    </Dropdown>
  </div>
  )
}

export default OperationMode