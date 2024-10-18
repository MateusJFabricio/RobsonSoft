import React,{useState, useContext, useEffect} from 'react'
import Button from 'react-bootstrap/Button';
import Form from 'react-bootstrap/Form';
import { ApiContext } from '../../context/APIContext';
import { TbMobiledata, TbMobiledataOff } from "react-icons/tb";

const CommPLCPage = () => {
  const {setUrl, connectionStatus, ip} = useContext(ApiContext)
  const [validated, setValidated] = useState(false);
  const [plcConnected, setPlcConnected] = useState(false)
  const [jsonRecebido, setJsonRecebido] = useState(null)

  useEffect(()=>{
    if (jsonRecebido !== null){
      setPlcConnected(jsonRecebido.CONNECTION)
    }
  }, [jsonRecebido])

  useEffect(()=>{
    getData();
  }, [])

  const getData =()=>{
    if (connectionStatus === 'Open'){
      try{
          fetch("http://" + ip + ":80/getPLCConectionStatus")
          .then((response) => response.json())
          .then((json) => {
            setJsonRecebido(json)
          })
      }catch{
          console.log("Teste");
      }
    }
  }

  const salvarPLCData = (e)=>{
    e.preventDefault();
    
    const ip =  document.getElementById('formIpAddress').value;
    const rack =  document.getElementById('formRack').value;
    const slot =  document.getElementById('formSlot').value;

    const ipValid = checkIpAddress(ip);

    setValidated(ipValid);
    
    if (!ipValid) return;
    
    const ipArray = ip.split(".");
    const json = {
      IP: [
        parseInt(ipArray[0]), 
        parseInt(ipArray[1]), 
        parseInt(ipArray[2]), 
        parseInt(ipArray[3])
      ],
      RACK: rack,
      SLOT: slot
    }
    postData(json, "postPLCConfig");
  }

  const postData = (json, endereco)=>{
    console.log(connectionStatus)
    if (connectionStatus === 'Open'){
        try{
            fetch("http://" + ip + ":80/" + endereco, {
            method: "POST",
            body: JSON.stringify(json),
            headers: {
                "Content-type": "text/plain;"
            }
            })
            .then((response) => response.json())
            
        }catch(erro){
            console.log(erro)
        }
    }
  }

  function checkIpAddress(ip) {
    return /^(\d{1,3}\.){3}\d{1,3}$/.test(ip);
  }

  const updateData = ()=>{
    getData();

    document.getElementById('formRack').value = jsonRecebido.PLC.RACK;
    document.getElementById('formSlot').value = jsonRecebido.PLC.SLOT;
    document.getElementById('formIpAddress').value = jsonRecebido.PLC.IP[0] + "." + jsonRecebido.PLC.IP[1] + "." +jsonRecebido.PLC.IP[2] + "." + jsonRecebido.PLC.IP[3];
  }

  const handleConectar = ()=>{
    postData({}, "postPLCConectar")
  }

  const handleDesconectar = ()=>{
    postData({}, "postPLCDesconectar")
  }
  return (
    <div style={{display: 'flex', flexDirection: 'column', width: '90%', height: '90%', padding: '2%', gap: '1%'}}>
      <div style={{display: 'flex', flexDirection: 'row', height: '5%', width: '60%', alignContent: 'center', alignItems: 'center', gap: '10px'}}>
        <h3>Comunicação com o PLC</h3>
      </div>
      <div style={{display: 'flex', width: '50%'}}>
          <div style={{display:'flex', gap: '15px', padding: '5px', color: 'white', justifyContent: 'center', alignItems: 'center', alignContent: 'center', borderRadius: '10px', backgroundColor: plcConnected ? "green" : "red"}}>
            {plcConnected ? <TbMobiledata/> : <TbMobiledataOff/>}
            {plcConnected ? <span>CONECTADO</span>  : <span>DESCONECTADO</span>}
          </div>
          <div style={{display: 'flex', width: '100%', justifyContent: 'flex-end'}}>
            <Button variant="light" onClick={handleConectar}>Conectar</Button>
            <Button variant="light" onClick={handleDesconectar}>Desconectar</Button>
          </div>
      </div>
      <div style={{width: '50%'}}>
        <Form onSubmit={salvarPLCData}>
          <Form.Group className="mb-3" controlId="formIpAddress">
            <Form.Label>Endereço IP</Form.Label>
            <Form.Control type="text" placeholder="Endereço IP" required isInvalid={!validated} isValid={validated} defaultValue={"192.168.1.2"} />
          </Form.Group>

          <Form.Group className="mb-3" controlId="formRack">
            <Form.Label>Rack</Form.Label>
            <Form.Control type="number" placeholder="Rack" defaultValue={"0"}/>
            <Form.Text>
              S7 300, S7400, S71200 e S71500 sempre vão com o Rack 0
            </Form.Text>
          </Form.Group>

          <Form.Group className="mb-3" controlId="formSlot">
            <Form.Label>Slot</Form.Label>
            <Form.Control type="number" placeholder="Slot" defaultValue={"1"}/>
            <Form.Text>
              S7 300 sempre tem o Slot 2. Verifique os outros modelos no Hardware Configuration no TIA Portal
            </Form.Text>
          </Form.Group>
          <Button variant="primary" type="button" onClick={updateData}>
            Update
          </Button>
          {'  '}
          <Button variant="primary" type="submit">
            Salvar
          </Button>
        </Form>
      </div>
    </div>
  )
}

export default CommPLCPage