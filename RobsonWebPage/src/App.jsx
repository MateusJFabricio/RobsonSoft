import { useState } from 'react'
import './App.css'
import 'bootstrap/dist/css/bootstrap.min.css';
import { Outlet } from 'react-router-dom'
import MenuSuperior from './components/MenuSuperior/MenuSuperior'
import MenuLateral from './components/MenuLateral/MenuLateral'
import Button from 'react-bootstrap/Button';
import Conexao from './components/MenuSuperior/Conexao/Conexao';
import JoystickConnected from './components/MenuSuperior/JoystickConnected/JoystickConnected';
import DeadMan from './components/MenuSuperior/DeadMan/DeadMan';
import MoveType from './components/MenuSuperior/MoveType/MoveType';
import OperationMode from './components/MenuSuperior/OperationMode/OperationMode';
import SpeedConfig from './components/MenuSuperior/SpeedConfig/SpeedConfig';

function App() {

  return (
    <>
      <div style={{height: '100vh'}}>
        <div style={{display: 'flex', flexDirection: 'row', height: '100%', width: '100vw'}}>
            <MenuLateral/>
            <div style={{width: '100%'}}>
              <MenuSuperior>
                <DeadMan/>
                <MoveType/>
                <OperationMode/>
                <SpeedConfig/>
                <JoystickConnected/>
                <Conexao/>
              </MenuSuperior>
              <Outlet/>
            </div>
        </div>
    </div>
    </>
  )
}

export default App
