import React, {useState, useEffect, useContext} from 'react'
import ButtonGroup from 'react-bootstrap/ButtonGroup';
import ToggleButton from 'react-bootstrap/ToggleButton';
import Col from 'react-bootstrap/esm/Col';
import Container from 'react-bootstrap/esm/Container';
import Form from 'react-bootstrap/Form';
import Button from 'react-bootstrap/esm/Button';

const HomePage = () => {
    const [manualMode, setManualMode] = useState(false)
    const [manualCemMode, setManualCemMode] = useState(false)
    const [autoMode, setAutoMode] = useState(false)
    const [checked, setChecked] = useState(false);
    const [radioValue, setRadioValue] = useState('1');

    const radios = [
        { name: 'Manual', value: '1' },
        { name: 'Manual 100%', value: '2' },
        { name: 'Automatico', value: '3' },
      ];

  return (
    <div style={{padding: '10px'}}>
        <Container>
            <Col>
                <h6>Modo de Operação:</h6>
                <ButtonGroup>
                {radios.map((radio, idx) => (
                <ToggleButton
                    key={idx}
                    id={`radio-${idx}`}
                    type="radio"
                    variant={idx !== 2 ? 'outline-success' : 'outline-danger'}
                    name="radio"
                    value={radio.value}
                    checked={radioValue === radio.value}
                    onChange={(e) => setRadioValue(e.currentTarget.value)}
                >
                    {radio.name}
                </ToggleButton>
                ))}
                </ButtonGroup>
            </Col>
            <Col>
            </Col>
            
        </Container>
    </div>
  )
}

export default HomePage