import React from 'react'

const MenuLateralCategoria = ({children, text}) => {
  return (
    <div style={{
            display: 'flex',
            flexDirection: 'column',
            gap: '10px',
            paddingBottom: '30px'
        }}>
        <b>{text}</b>
        {children}
    </div>
  )
}

export default MenuLateralCategoria