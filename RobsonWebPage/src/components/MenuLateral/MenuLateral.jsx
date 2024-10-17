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
import { FaGithub } from "react-icons/fa6";
import { VscSaveAll,VscSettingsGear  } from "react-icons/vsc";
import { TbManualGearbox,TbCodeCircle2  } from "react-icons/tb";
import { GiRobotAntennas,GiSatelliteCommunication } from "react-icons/gi";

const MenuLateral = () => {
    const navigate = useNavigate()
    const [homeSelected, setHomeSelected] = useState(true)
    const [configSelected, setConfigSelected] = useState(false)
    const [controlSelected, setControlSelected] = useState(false)
    const [projectSelected, setProjectSelected] = useState(false)
    const [softwareSelected, setSoftwareSelected] = useState(false)
    const [backupRestoreSelected, setBackupRestoreSelected] = useState(false)
    const [commSelected, setCommSelected] = useState(false)

    const handleFlyClick = ()=>{
        navigate("/Fly")
      }

    const homeClick = ()=>{
        setHomeSelected(true)
        setConfigSelected(false)
        setControlSelected(false)
        setProjectSelected(false)
        setSoftwareSelected(false)
        setBackupRestoreSelected(false)
        setCommSelected(false)
        navigate("/")
    }

    const configClick = ()=>{
        setHomeSelected(false)
        setConfigSelected(true)
        setControlSelected(false)
        setProjectSelected(false)
        setSoftwareSelected(false)
        setBackupRestoreSelected(false)
        setCommSelected(false)
        navigate("/configuracao")
    }

    const controlClick = ()=>{
        setHomeSelected(false)
        setConfigSelected(false)
        setControlSelected(true)
        setProjectSelected(false)
        setSoftwareSelected(false)
        setBackupRestoreSelected(false)
        setCommSelected(false)
        navigate("/control")
    }

    const communicationClick = ()=>{
        setHomeSelected(false)
        setConfigSelected(false)
        setControlSelected(false)
        setProjectSelected(false)
        setSoftwareSelected(false)
        setBackupRestoreSelected(false)
        setCommSelected(true)
        navigate("/communication")
    }

    const softwareClick = ()=>{
        setHomeSelected(false)
        setConfigSelected(false)
        setControlSelected(false)
        setProjectSelected(false)
        setSoftwareSelected(true)
        setBackupRestoreSelected(false)
        setCommSelected(false)
        navigate("/software")
    }

    const backupRestoreClick = ()=>{
        setHomeSelected(false)
        setConfigSelected(false)
        setControlSelected(false)
        setProjectSelected(false)
        setSoftwareSelected(false)
        setBackupRestoreSelected(true)
        setCommSelected(false)
        navigate("/backup")
    }

    const projectClick = ()=>{
        setHomeSelected(false)
        setConfigSelected(false)
        setControlSelected(false)
        setProjectSelected(true)
        setSoftwareSelected(false)
        setBackupRestoreSelected(false)
        setCommSelected(false)
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
                    <MenuLateralItem text={"Home"} isSelected={homeSelected} buttonClick={homeClick}>
                        <GiRobotAntennas />
                    </MenuLateralItem>
                    <MenuLateralItem text={"Software"} isSelected={softwareSelected} buttonClick={softwareClick}>
                        <TbCodeCircle2 />  
                    </MenuLateralItem>
                    <MenuLateralItem text={"Configuração"}isSelected={configSelected} buttonClick={configClick}>
                        <VscSettingsGear />
                    </MenuLateralItem>
                    <MenuLateralItem text={"Comunicação PLC"} isSelected={commSelected} buttonClick={communicationClick}>
                        <GiSatelliteCommunication />
                    </MenuLateralItem>
                    <MenuLateralItem text={"Controle"} isSelected={controlSelected} buttonClick={controlClick}>
                        <TbManualGearbox />
                    </MenuLateralItem>
                </MenuLateralCategoria>
                <MenuLateralCategoria text={"Outras Coisas"}>
                    <MenuLateralItem text={"Backup/Restore"} isSelected={backupRestoreSelected} buttonClick={backupRestoreClick}>
                        <VscSaveAll />
                    </MenuLateralItem>
                    <MenuLateralItem text={"Projeto"} isSelected={projectSelected} buttonClick={projectClick}>
                        <FaGithub />
                    </MenuLateralItem>
                </MenuLateralCategoria>
            </div>
        </div>
    </>
  )
}

export default MenuLateral