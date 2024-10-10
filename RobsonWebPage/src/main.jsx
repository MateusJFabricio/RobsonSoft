import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import App from './App.jsx'
import './index.css'
import { ApiContextProvider } from './context/APIContext.jsx'
import { JoystickContextProvider } from './context/JoystickContext.jsx';
import {createBrowserRouter,RouterProvider} from "react-router-dom";
import HomePage from './pages/HomePage/HomePage.jsx';
import ControlRobot from './pages/ControlRobot/ControlRobot.jsx'

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
          path: "/control",
          element: <ControlRobot/>
        },
        {
          path: "/Teste",
          element: <HomePage/>
        },
    ] 
  }
]);

createRoot(document.getElementById('root')).render(
  <StrictMode>
    <RouterProvider router={router} />
  </StrictMode>,
)