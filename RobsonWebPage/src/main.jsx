import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import App from './App.jsx'
import './index.css'
import { ApiContextProvider } from './context/APIContext.jsx'
import { JoystickContextProvider } from './context/JoystickContext.jsx';
import {createBrowserRouter,RouterProvider} from "react-router-dom";
import HomePage from './pages/HomePage/HomePage.jsx';
import ControlRobot from './pages/ControlRobot/ControlRobot.jsx'
import SoftwarePage from './pages/Software/SoftwarePage.jsx'
import ConfiguracaoPage from './pages/ConfiguracaoPage/ConfiguracaoPage.jsx'

const router = createBrowserRouter([
  {
    path: "/",
    element: (
        <JoystickContextProvider>
          <ApiContextProvider>
            <App/>
          </ApiContextProvider>
        </JoystickContextProvider>
      ),
      children:[
        {
          path: "/",
          element: <HomePage/>
        },
        {
          path: "/configuracao",
          element: <ConfiguracaoPage/>
        },
        {
          path: "/software",
          element: <SoftwarePage/>
        },
        {
          path: "/control",
          element: <ControlRobot/>
        },
    ] 
  }
]);

createRoot(document.getElementById('root')).render(
  <StrictMode>
    <RouterProvider router={router} />
  </StrictMode>,
)