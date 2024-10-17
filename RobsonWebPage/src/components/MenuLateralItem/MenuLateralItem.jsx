import React from 'react'

const MenuLateralItem = ({children, text, icon, isSelected, buttonClick}) => {
  return (
    <div style={{display: 'flex',justifyContent: 'left', alignItems: 'center',height: '45px',fontSize: '20px'}}>
        <button style={
            {
                display: 'flex',
                alignContent: 'center',
                alignItems: 'center',
                gap: '15px',
                padding: '10px',
                color: 'white',
                backgroundColor: isSelected ? '#5D87FF' : 'transparent', 
                width: '100%', 
                height: '100%',
                border: 'none', 
                borderRadius: '10px'
            }
            } onClick={buttonClick}>
            {children}
            {text}
        </button>
    </div>
  )
}

export default MenuLateralItem