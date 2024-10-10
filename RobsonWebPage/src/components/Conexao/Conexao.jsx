import React, {useContext, createRef } from 'react'
import Button from 'react-bootstrap/Button';
import Form from 'react-bootstrap/Form';
import { ApiContext } from '../../context/APIContext.jsx'


const Conexao = () => {
    const {setUrl, connectionStatus, sendData, ip} = useContext(ApiContext);
    const inputRef = createRef("robsonsoft");

    const connectionSubmit = (e)=>{
        e.preventDefault();
        console.log( inputRef.current.value)
    }
  return (
    <div style={{
        display: 'flex',
        flexDirection: 'row'
    }}>
        <Form onSubmit={connectionSubmit} style={{
            display: 'flex',
            flexDirection: 'row',
            gap:'5px',
            color: 'white',
            alignItems: 'center',
            alignContent: 'center'
            }}>
            <Form.Label>IP:</Form.Label>
            <Form.Control type="text" ref={inputRef}/>
            <Button variant="success" type="submit">
                Conectar
            </Button>
        </Form>
    </div>
  )
}

export default Conexao