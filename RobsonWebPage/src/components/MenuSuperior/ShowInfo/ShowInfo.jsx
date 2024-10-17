import React, {useState, useEffect, useRef, useContext} from 'react'
import Toast from 'react-bootstrap/Toast';
import ToastContainer from 'react-bootstrap/ToastContainer';
import { ApiContext } from '../../../context/APIContext';

const ShowInfo = () => {
    const {setUrl, connectionStatus, ip} = useContext(ApiContext)
    const robotIpRef = useRef(ip)
    const showRef = useRef(false)
    const [actualAngles, setActualAngles] = useState({
        Joint1: "NoData",
        Joint2: "NoData",
        Joint3: "NoData",
        Joint4: "NoData",
        Joint5: "NoData",
        Gripper: "NoData"
    })

    const handleShowInfo = ()=>{
        showRef.current = !showRef.current;
    }
    useEffect(() => {
        robotIpRef.current = ip;
      }, [ip])

    useEffect(() => {
        const interval = setInterval(()=>{
            if (robotIpRef.current !== "" && showRef.current){
                try{
                    fetch("http://" + robotIpRef.current + ":80/getJointAngles")
                    .then((response) => response.json())
                    .then((json) => {
                        setActualAngles(json);
                    })
                }catch{
                    setActualAngles({
                        Joint1: "NoData",
                        Joint2: "NoData",
                        Joint3: "NoData",
                        Joint4: "NoData",
                        Joint5: "NoData",
                        Gripper: "NoData"
                    })
                }
            }
          }
        , 1000);
        
        return () =>{ 
          clearInterval(interval)
        }
      }, []) 

  return (
    <div style={{
        display: 'flex',
        flexDirection: 'column',
        width: '50px',
        alignContent: 'center',
        alignItems: 'center',
        justifyContent: 'center',
        fontSize: '15px',
        lineHeight: '10px',
        borderRadius: '5px',
    }} onClick={handleShowInfo}>
        <button style={{width: '100%', height: '100%', borderRadius: '20%', backgroundColor: '#0D6EFD', color: 'white'}}><b>INFO</b></button>
        <ToastContainer
          className="p-1"
          position={'bottom-end'}
          style={{ zIndex: 1 }}
          color='black'
        >
          <Toast onClose={() => showRef.current = false} show={showRef.current}>
            <Toast.Header closeButton={false}>
              <img
                src="holder.js/20x20?text=%20"
                className="rounded me-2"
                alt=""
              />
              <strong className="me-auto">Informações</strong>
            </Toast.Header>
            <Toast.Body style={{color: 'black'}}>
                <p>Eixo 1: { actualAngles.Joint1 + "°"} </p>
                <p>Eixo 2: { actualAngles.Joint2 + "°"} </p>
                <p>Eixo 3: { actualAngles.Joint3 + "°"} </p>
                <p>Eixo 4: { actualAngles.Joint4 + "°"} </p>
                <p>Eixo 5: { actualAngles.Joint5 + "°"} </p>
                <p>Gripper: { actualAngles.Gripper + "°"} </p>
            </Toast.Body>
          </Toast>
        </ToastContainer>
    </div>
  )
}

export default ShowInfo