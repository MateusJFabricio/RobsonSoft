import { useState } from 'react'
import './App.css'
import 'bootstrap/dist/css/bootstrap.min.css';
import { Outlet } from 'react-router-dom'
import MenuSuperior from './components/MenuSuperior/MenuSuperior'
import MenuLateral from './components/MenuLateral/MenuLateral'
import Button from 'react-bootstrap/Button';
import Conexao from './components/Conexao/Conexao';
import JoystickConnected from './components/JoystickConnected/JoystickConnected';

function App() {

  return (
    <>
      <div style={{height: '100vh'}}>
        <div style={{display: 'flex', flexDirection: 'row', height: '100%', width: '100vw'}}>
            <MenuLateral/>
            <div style={{width: '100%'}}>
              <MenuSuperior>
                <Conexao/>
                <JoystickConnected/>
              </MenuSuperior>
              <Outlet/>
            </div>
        </div>
    </div>
    </>
  )
}

export default App
