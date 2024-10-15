import React, { useState, useEffect, createRef } from 'react'
import './BlockyCustom.css'
import { BlocklyWorkspace } from 'react-blockly';
import * as Blockly from 'blockly/core';
import {javascriptGenerator, Order} from 'blockly/javascript';
import CodeMirror from '@uiw/react-codemirror';
import { javascript } from '@codemirror/lang-javascript';
import {androidstudio} from '@uiw/codemirror-theme-androidstudio';

const BlockyCustom = () => {
    const scriptRef = createRef("");
    const [xml, setXml] = useState('');
    const [javascriptCode, setJavascriptCode] = useState(
`
/*
  Funcões predefinidas:
    MoveJ(j1, j2, j3, j4, j5);
    Wait(millisegundos);
    OpenGripper();
    CloseGripper();
    SetAprox(valor);
    SetSpeed(valor);
*/

function Trajetoria(){
  OpenGripper();
  MoveJ(20, 0, 0, 0, 90);
  CloseGripper();
  Wait(1000);
  MoveJ(20, 10, 10, 10, 10);
}

//Config
SetSpeed(60); // °/s
SetAprox(1); // ° -> Target

//Chama Prog
Trajetoria();`);

    function workspaceDidChange(workspace) {
      const code = javascriptGenerator.workspaceToCode(workspace);
      setJavascriptCode(code);
    }

    useEffect(() => {
      //Blockly.common.defineBlocks(jsonGenerator);
        
    }, [])   
    
    const onChange = React.useCallback((val, viewUpdate) => {
      setJavascriptCode(val);
    }, []);

  return (
    <div style={{display: 'flex', width: '100%', height: '70%', backgroundColor: 'gray'}}>
      {/* <BlocklyWorkspace
        className="blockcustom-container"
        toolboxConfiguration={toolbox} // Configuração do toolbox como JSON
        initialXml={xml}
        onXmlChange={setXml} // Atualiza o estado do XML quando há mudanças
        workspaceConfiguration={{
          grid: {
            spacing: 20,
            length: 3,
            colour: "#ccc",
            snap: true,
          },
        }}
        onWorkspaceChange={workspaceDidChange}
      /> */}
      <CodeMirror 
        value={javascriptCode} 
        height="100%"
        style={{width: '100%', height: '100%'}} 
        extensions={[javascript({ jsx: true })]} 
        onChange={onChange} 
        theme={androidstudio}
        />;
    </div>
  )
}

export default BlockyCustom