import React, {useContext, createRef } from 'react'
import Button from 'react-bootstrap/Button';
import Form from 'react-bootstrap/Form';
import { ApiContext } from '../../context/APIContext.jsx'
import Spinner from 'react-bootstrap/Spinner';


const Conexao = () => {
    const {setUrl, connectionStatus, sendData, ip} = useContext(ApiContext);
    const inputRef = createRef("robsonsoft");

    const connectionSubmit = (e)=>{
        e.preventDefault();
        setUrl(inputRef.current.value)
    }
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
            <Button variant= {connectionStatus !== 'Open' ? "warning" : "success"} type="submit">
                {connectionStatus === 'Connecting' && "Conectando"}
                {connectionStatus === 'Open' && "Conectado"}
                {connectionStatus === 'Closed' && "Conectar"}
            </Button>
            {connectionStatus !== 'Open' &&
                <Button variant="danger" disabled>
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
            }
        </Form>
    </div>
  )
}

export default Conexao