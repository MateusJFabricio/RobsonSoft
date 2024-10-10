import React from 'react'
import MenuSuperior from '../../components/MenuSuperior/MenuSuperior'
import MenuLateral from '../../components/MenuLateral/MenuLateral'
import { Outlet } from 'react-router-dom'
import Dropdown from 'react-bootstrap/Dropdown';

const HomePage = () => {
  return (
    <div style={{padding: '10px'}}>
        <Dropdown>
        <Dropdown.Toggle variant="success" id="dropdown-basic">
            Modo de Operação
        </Dropdown.Toggle>

        <Dropdown.Menu>
            <Dropdown.Item href="#/action-1">Action</Dropdown.Item>
            <Dropdown.Item href="#/action-2">Another action</Dropdown.Item>
            <Dropdown.Item href="#/action-3">Something else</Dropdown.Item>
        </Dropdown.Menu>
        </Dropdown>
    </div>
  )
}

export default HomePage