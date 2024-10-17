import React, {useState, useContext, useEffect} from 'react'
import Button from 'react-bootstrap/Button';
import Col from 'react-bootstrap/Col';
import Form from 'react-bootstrap/Form';
import InputGroup from 'react-bootstrap/InputGroup';
import Row from 'react-bootstrap/Row';
import { ApiContext } from '../../context/APIContext';

const ConfiguracaoPage = () => {
  const {setUrl, connectionStatus, ip} = useContext(ApiContext)
  const [validated, setValidated] = useState(false);
  const [limites, setLimites] = useState({
    Joint1: {
      MinAngle: "NoData",
      MaxAngle: "NoData",
    },
    Joint2: {
      MinAngle: "NoData",
      MaxAngle: "NoData",
    },
    Joint3: {
      MinAngle: "NoData",
      MaxAngle: "NoData",
    },
    Joint4: {
      MinAngle: "NoData",
      MaxAngle: "NoData",
    },
    Joint5: {
      MinAngle: "NoData",
      MaxAngle: "NoData",
    },
    Gripper: {
      MinAngle: "NoData",
      MaxAngle: "NoData",
    }
})

  const handleSubmit = (e)=>{
    e.preventDefault();
    const json = {
      Joint1: {
        MinAngle: document.getElementById('eixo1-min').value,
        MaxAngle: document.getElementById('eixo1-max').value,
      },
      Joint2: {
        MinAngle: document.getElementById('eixo2-min').value,
        MaxAngle: document.getElementById('eixo2-max').value,
      },
      Joint3: {
        MinAngle: document.getElementById('eixo3-min').value,
        MaxAngle: document.getElementById('eixo3-max').value,
      },
      Joint4: {
        MinAngle: document.getElementById('eixo4-min').value,
        MaxAngle: document.getElementById('eixo4-max').value,
      },
      Joint5: {
        MinAngle: document.getElementById('eixo5-min').value,
        MaxAngle: document.getElementById('eixo5-max').value,
      },
      Gripper: {
        MinAngle: document.getElementById('gripper-min').value,
        MaxAngle: document.getElementById('gripper-max').value,
      }
  }
    setLimites(json)
    postData(json, "postJointLimits")
  }

  const handleResetPosition = (index)=>{
    const nome = {};
    nome["Joint" + index] = true;
    postData(nome, "postJointCalibration")
  }

  const postData = (json, endereco)=>{
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

  const getData =()=>{
    if (connectionStatus === 'Open'){
      try{
          fetch("http://" + ip + ":80/getJointLimits")
          .then((response) => response.json())
          .then((json) => {
            setLimites(json);
          })
      }catch{
          
      }
    }
  }

  useEffect(()=>{
    getData();
  }, [])

  useEffect(() => {
    if (limites !== null){
      for (let index = 1; index < 6; index++) {
        
        document.getElementById('eixo' + index +'-min').value = limites["Joint" + index].MinAngle;
      }

      for (let index = 1; index < 6; index++) {
        
        document.getElementById('eixo' + index +'-max').value = limites["Joint" + index].MaxAngle;
      }

      document.getElementById('gripper-min').value = limites["Gripper"].MinAngle;
      document.getElementById('gripper-max').value = limites["Gripper"].MaxAngle;
    }
  
  }, [limites])
  

  return (
    <div style={{display:'flex', flexDirection: 'row', backgroundColor: 'none', width: '100%', padding: '10px', gap: '15px'}}>
      <Form style={{backgroundColor: '#bde7f0', borderRadius: '10px', padding: '15px'}} onSubmit={handleSubmit}>
        <Row className="mb-3">
          <h4>LIMITE DOS EIXOS</h4>
        </Row>
        <Row>
          <Form.Label style={{alignContent: 'center'}} column="lg" lg={3}><b>Eixo 1:</b></Form.Label>
          <Col>
            <Form.Label lg={5}>Valor Min:</Form.Label>
            <Form.Control size="sm" type="text" id='eixo1-min' />
          </Col>
          <Col>
            <Form.Label lg={5}>Valor Max:</Form.Label>
            <Form.Control size="sm" type="text" id='eixo1-max'/>
          </Col>
          <Col style={{display: 'flex', justifyContent: 'center', alignContent: 'center', alignItems: 'flex-end'}}>
            <Button style={{height: '60%'}} onClick={()=>{handleResetPosition(1)}}>Reset position</Button>
          </Col>
        </Row>
        <br/>
        <Row>
          <Form.Label style={{alignContent: 'center'}} column="lg" lg={3}><b>Eixo 2:</b></Form.Label>
          <Col>
            <Form.Label lg={5}>Valor Min:</Form.Label>
            <Form.Control size="sm" type="text" id='eixo2-min'/>
          </Col>
          <Col>
            <Form.Label lg={5}>Valor Max:</Form.Label>
            <Form.Control size="sm" type="text"  id='eixo2-max'/>
          </Col>
          <Col style={{display: 'flex', justifyContent: 'center', alignContent: 'center', alignItems: 'flex-end'}}>
            <Button style={{height: '60%'}} onClick={()=>{handleResetPosition(2)}}>Reset position</Button>
          </Col>
        </Row>
        <br/>
        <Row>
          <Form.Label style={{alignContent: 'center'}} column="lg" lg={3}><b>Eixo 3:</b></Form.Label>
          <Col>
            <Form.Label lg={5}>Valor Min:</Form.Label>
            <Form.Control size="sm" type="text" id='eixo3-min'/>
          </Col>
          <Col>
            <Form.Label lg={5}>Valor Max:</Form.Label>
            <Form.Control size="sm" type="text" id='eixo3-max'/>
          </Col>
          <Col style={{display: 'flex', justifyContent: 'center', alignContent: 'center', alignItems: 'flex-end'}}>
            <Button style={{height: '60%'}} onClick={()=>{handleResetPosition(3)}}>Reset position</Button>
          </Col>
        </Row>
        <br/>
        <Row>
          <Form.Label style={{alignContent: 'center'}} column="lg" lg={3}><b>Eixo 4:</b></Form.Label>
          <Col>
            <Form.Label lg={5}>Valor Min:</Form.Label>
            <Form.Control size="sm" type="text" id='eixo4-min'/>
          </Col>
          <Col>
            <Form.Label lg={5}>Valor Max:</Form.Label>
            <Form.Control size="sm" type="text" id='eixo4-max'/>
          </Col>
          <Col style={{display: 'flex', justifyContent: 'center', alignContent: 'center', alignItems: 'flex-end'}}>
            <Button style={{height: '60%'}} onClick={()=>{handleResetPosition(4)}}>Reset position</Button>
          </Col>
        </Row>
        <br/>
        <Row>
          <Form.Label style={{alignContent: 'center'}} column="lg" lg={3}><b>Eixo 5:</b></Form.Label>
          <Col>
            <Form.Label lg={5}>Valor Min:</Form.Label>
            <Form.Control size="sm" type="text" id='eixo5-min'/>
          </Col>
          <Col>
            <Form.Label lg={5}>Valor Max:</Form.Label>
            <Form.Control size="sm" type="text" id='eixo5-max'/>
          </Col>
          <Col style={{display: 'flex', justifyContent: 'center', alignContent: 'center', alignItems: 'flex-end'}}>
            <Button style={{height: '60%'}} onClick={()=>{handleResetPosition(5)}}>Reset position</Button>
          </Col>
        </Row>
        <br/>
        <Row>
          <br />
        </Row>
        <Button onClick={()=>{getData()}}>Update</Button>
        {'     '}
        <Button type="submit">Salvar</Button>
      </Form>

      <Form style={{backgroundColor: '#bde7f0', borderRadius: '10px', padding: '15px'}} noValidate validated={validated} onSubmit={handleSubmit}>
        <Row className="mb-3">
          <h4>Gripper</h4>
        </Row>
        <Row>
          <Col>
            <Form.Label lg={5}>Valor Min:</Form.Label>
            <Form.Control size="sm" type="text" id='gripper-min'/>
          </Col>
          <Col>
            <Form.Label lg={5}>Valor Max:</Form.Label>
            <Form.Control size="sm" type="text" id='gripper-max'/>
          </Col>
        </Row>
        <Row>
          <Col>
            <Form.Label lg={5}>Valor Open:</Form.Label>
            <Form.Control size="sm" type="text"/>
          </Col>
          <Col>
            <Form.Label lg={5}>Valor Close:</Form.Label>
            <Form.Control size="sm" type="text"/>
          </Col>
        </Row>
        <Row>
          <br />
        </Row>
        <Button type="submit">Salvar</Button>
      </Form>
    </div>
  )
}

export default ConfiguracaoPage