import React, {useContext, createRef, useEffect } from 'react'
import Button from 'react-bootstrap/Button';
import Form from 'react-bootstrap/Form';
import { ApiContext } from '../../../context/APIContext.jsx'
import Spinner from 'react-bootstrap/Spinner';


const Conexao = () => {
    const {setUrl, connectionStatus, sendData, ip} = useContext(ApiContext);
    const inputRef = createRef("robsonsoft");

    const connectionSubmit = (e)=>{
        e.preventDefault();
        setUrl(inputRef.current.value)
    }
    useEffect(() => {
        inputRef.current.value = ip;
      }, [ip])
    //variant="success"
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
            <Button style={{width: '150px'}} variant= {connectionStatus !== 'Open' ? "warning" : "success"} type="submit">
                {connectionStatus === 'Connecting' && "Conectando"}
                {connectionStatus === 'Open' && "Conectado"}
                {connectionStatus === 'Closed' && "Conectar"}
            </Button>
            {/* nnectionStatus !== 'Open' &&
                <Button variant="danger" disabled style={{width: '60px'}}>
                    {connectionStatus === 'Connecting' &&
                        <Spinner
                        as="div"
                        animation="grow"
                        size="sm"
                        role="status"
                        aria-hidden="true"
                        />
                    }
                </Button>
                */
            }
        </Form>
    </div>
  )
}

export default Conexao