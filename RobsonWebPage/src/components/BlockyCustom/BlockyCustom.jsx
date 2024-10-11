import React, { useState } from 'react'
import './BlockyCustom.css'
import { BlocklyWorkspace } from 'react-blockly';

const MY_TOOLBOX = {
    "kind": "categoryToolbox",
    "contents": [
      {
        "kind": "category",
        "name": "Condicionais",
        'colour':'green',
        "contents": [
          { "kind": "block", "type": "controls_if" },
          { "kind": "block", "type": "controls_repeat_ext" }
        ]
      },
      {
        "kind": "category",
        "name": "Variáveis",
        'colour':'red',
        "custom": "VARIABLE"
      },
      {
        "kind": "category",
        "name": "Matemática",
        'colour':'blue',
        "contents": [
          { "kind": "block", "type": "math_number" },
          { "kind": "block", "type": "math_arithmetic" },
          { "kind": "block", "type": "math_single" },
          { "kind": "block", "type": "math_trig" },
          { "kind": "block", "type": "math_random_int" },
          { "kind": "block", "type": "math_modulo" }
        ]
      },
      {
        "kind": "category",
        "name": "Funções",
        'colour':'purple',
        "custom": "PROCEDURE"
      },
      {
        "kind": "category",
        "name": "Comentários",
        'colour':'olive',
        "contents": [
          {
            "kind": "block",
            "type": "text",
            "message0": "// %1",
            "args0": [
              {
                "type": "field_input",
                "name": "COMMENT",
                "text": "comentário"
              }
            ],
            "previousStatement": null,
            "nextStatement": null,
            "colour": 160
          }
        ]
      },
      {
        "kind": "category",
        "name": "Movimento",
        'colour':'gray',
        "contents": [
          {
            "kind": "block",
            "type": "move_joint",
            "message0": "MoveJoint %1",
            "args0": [
              {
                "type": "field_input",
                "name": "JOINT",
                "text": "junta"
              }
            ],
            "previousStatement": null,
            "nextStatement": null,
            "colour": 210
          },
          {
            "kind": "block",
            "type": "move_linear",
            "message0": "MoveLinear %1",
            "args0": [
              {
                "type": "field_input",
                "name": "LINEAR",
                "text": "linear"
              }
            ],
            "previousStatement": null,
            "nextStatement": null,
            "colour": 210
          }
        ]
      }
    ]
};

const BlockyCustom = () => {
    const [xml, setXml] = useState('');

  return (
    <div style={{display: 'flex', width: '100%', height: '70%', backgroundColor: 'gray'}}>
      <BlocklyWorkspace
        className="blockcustom-container"
        toolboxConfiguration={MY_TOOLBOX} // Configuração do toolbox como JSON
        initialXml={xml}
        onXmlChange={setXml} // Atualiza o estado do XML quando há mudanças
      />
    </div>
  )
}

export default BlockyCustom