import React from 'react'

const MenuSuperior = ({children}) => {
  return (
    <>
      <div style={
        { 
          display: 'flex', 
          flexDirection: 'row-reverse',
          gap: '5px',
          width: '100%', 
          height: '50px', 
          backgroundColor: '#212529',
          padding: '5px'
        }}>   
        {children} 
      </div>
    </>
  )
}

export default MenuSuperior