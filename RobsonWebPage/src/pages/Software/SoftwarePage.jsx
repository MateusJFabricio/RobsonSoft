import React, {useState, Suspense, useRef,useEffect } from 'react'
const BlocklyWorkspace = React.lazy(() => import('react-blockly'));

const SoftwarePage = () => {
  const [xml, setXml] = useState('<xml xmlns="http://www.w3.org/1999/xhtml"><block type="text" x="70" y="30"><field name="TEXT"></field></block></xml>');

  const MY_TOOLBOX = {
    "kind": "categoryToolbox",
    "contents": [
      {
        "kind": "category",
        "name": "Logic",
        "contents": [
          {
            "kind": "block",
            "type": "controls_if"
          },
          {
            "kind": "block",
            "type": "logic_compare"
          }
        ]
      }
    ]
  };
  
  return (
    <div style={{ width: '100%', height: '30rem' }}>
      <Suspense fallback={<div>Loading Blockly...</div>}>
        <BlocklyWorkspace
          className="width-100"
          toolboxConfiguration={MY_TOOLBOX}
          initialXml={xml}
          onXmlChange={setXml}
          style={{ height: '100%', width: '100%' }}
        />
      </Suspense>
    </div>
  )
}

export default SoftwarePage