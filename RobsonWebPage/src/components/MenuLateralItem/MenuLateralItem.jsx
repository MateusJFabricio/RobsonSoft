import React from 'react'

const MenuLateralItem = ({text, icon, isSelected, buttonClick}) => {
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
            <img src={icon} alt="icon"/>
            {text}
        </button>
    </div>
  )
}

export default MenuLateralItem