import React, {useState} from 'react'
import { useParams, useNavigate } from 'react-router-dom'
import MenuLateralItem from '../MenuLateralItem/MenuLateralItem';
import HomeIcon from '../../assets/home-icon.svg';
import ControlIcon from '../../assets/control-icon.svg'
import ConfigIcon from '../../assets/setting-icon.svg'
import Logo from '../../assets/robotic-arm.png'
import ProjectIcon from '../../assets/project-icon.svg'
import SoftwareIcon from '../../assets/software-icon.svg'
import MenuLateralCategoria from '../MenuLateralCategoria/MenuLateralCategoria';

const MenuLateral = () => {
    const navigate = useNavigate()
    const [homeSelected, setHomeSelected] = useState(true)
    const [configSelected, setConfigSelected] = useState(false)
    const [controlSelected, setControlSelected] = useState(false)
    const [projectSelected, setProjectSelected] = useState(false)
    const [softwareSelected, setSoftwareSelected] = useState(false)

    const handleFlyClick = ()=>{
        navigate("/Fly")
      }

    const homeClick = ()=>{
        setHomeSelected(true)
        setConfigSelected(false)
        setControlSelected(false)
        setProjectSelected(false)
        setSoftwareSelected(false)
        navigate("/")
    }

    const configClick = ()=>{
        setHomeSelected(false)
        setConfigSelected(true)
        setControlSelected(false)
        setProjectSelected(false)
        setSoftwareSelected(false)
        navigate("/configuracao")
    }

    const controlClick = ()=>{
        setHomeSelected(false)
        setConfigSelected(false)
        setControlSelected(true)
        setProjectSelected(false)
        setSoftwareSelected(false)
        navigate("/control")
    }

    const softwareClick = ()=>{
        setHomeSelected(false)
        setConfigSelected(false)
        setControlSelected(false)
        setProjectSelected(false)
        setSoftwareSelected(true)
        navigate("/software")
    }

    const projectClick = ()=>{
        setHomeSelected(false)
        setConfigSelected(false)
        setControlSelected(false)
        setProjectSelected(true)
        setSoftwareSelected(false)
    }
  return (
    <>
        <div style={{height: '100%', backgroundColor: '#212529', color: 'white', padding: '10px'}}>
            <div style={
                { 
                    width: '280px', 
                    height: '100%', 
                    display: 'flex', 
                    flexDirection: 'column', 
                    gap: '1px'
                    }
                }>
                <div style={
                    {
                        display: 'flex',
                        gap: '20px',
                        height: '80px',
                        paddingBottom: '40px',
                        justifyContent: 'center', 
                        alignItems: 'center', 
                        fontSize: '25px',
                        fontWeight: 'bold',
                        color: '#3772FD'
                    }}>
                    <img src={Logo} alt="icon"/>
                     RobsonSoft
                </div>
                <MenuLateralCategoria text={"View"}>
                    <MenuLateralItem text={"Home"} icon={HomeIcon} isSelected={homeSelected} buttonClick={homeClick}/>
                    <MenuLateralItem text={"Software"} icon={SoftwareIcon} isSelected={softwareSelected} buttonClick={softwareClick}/>
                    <MenuLateralItem text={"Configuração"} icon={ConfigIcon} isSelected={configSelected} buttonClick={configClick}/>
                </MenuLateralCategoria>
                <MenuLateralCategoria text={"Outras Coisas"} icon={HomeIcon}>
                    <MenuLateralItem text={"Controle"} icon={ControlIcon} isSelected={controlSelected} buttonClick={controlClick}/>
                    <MenuLateralItem text={"Projeto"} icon={ProjectIcon} isSelected={projectSelected} buttonClick={projectClick}/>
                </MenuLateralCategoria>
            </div>
        </div>
    </>
  )
}

export default MenuLateral