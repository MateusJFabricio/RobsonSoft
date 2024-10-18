import React, { useState, useContext } from 'react';
import BlockyCustom from '../../components/BlockyCustom/BlockyCustom';
import Form from 'react-bootstrap/Form';
import Button from 'react-bootstrap/Button';
import { VscNewFile } from 'react-icons/vsc';
import { MdFileOpen, MdSaveAlt, MdCode } from 'react-icons/md';
import { FaRegSave, FaPlay,  } from 'react-icons/fa';
import { GrPowerReset  } from 'react-icons/gr';
import { FiCodepen  } from 'react-icons/fi';
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col'
import Container from 'react-bootstrap/esm/Container';
import { ApiContext } from '../../context/APIContext';

const SoftwarePage = () => {
  const {setUrl, connectionStatus, ip} = useContext(ApiContext)
  const [script, setScript] = useState("")
  const handleSaveCode = ()=>{
    if (script !== ""){
      postData({"code" : script}, "postSaveExternalCode" );
    }
  }

  const handleScript = (val)=>{
    setScript(val)
  }
  
  const handlePlayCode = ()=>{
    postData({}, "postStartExternalCode")
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

  return (
    <div style={{display: 'flex', flexDirection: 'column', width: '100%', height: '100%', padding: '2%', gap: '1%'}}>
      <div style={{display: 'flex', flexDirection: 'row', height: '5%', width: '100%', alignContent: 'center', alignItems: 'center', gap: '10px'}}>
        <h3>Software</h3>
        <div style={{display: 'flex', flexDirection: 'row', width: '100%', alignContent: 'center', alignItems: 'center', justifyContent: 'right'}}>
          <Button variant="light"><VscNewFile />Novo</Button>
          <Button variant="light"><MdFileOpen/> Abrir</Button>
          <Button variant="light"><FaRegSave/>Salvar</Button>
          <Button variant="light"><MdSaveAlt/>Exportar</Button>
        </div>
      </div>
      <div style={{width: '100%'}}>
        <Row>
          <Col xs={4}>
            <Form.Control type="text" placeholder="Nome do Prog" value={"Prog1"} onChange={()=>{}} />
          </Col>
          <Col xs={4}>
            <Button variant="light" onClick={handleSaveCode}><FaRegSave/>Salvar</Button>
          </Col>
          <Col xs={4}>
            <Button variant="info"><MdCode/>Código</Button>
            {' '}
            <Button variant="light"><FiCodepen/>Bloco</Button>
          </Col>
        </Row>
      </div>
      <BlockyCustom handleScriptChanged ={handleScript}/>
      <Container>
        <Row>
          <Col>
          </Col>
          <Col md="auto">
            <Button variant="light"><GrPowerReset/>Home</Button>
            {' '}
            <Button variant="primary" onClick={handlePlayCode}><FaPlay/>Play</Button>
          </Col>
          <Col>
          </Col>
        </Row>
      </Container>
    </div>
  );
}

export default SoftwarePage