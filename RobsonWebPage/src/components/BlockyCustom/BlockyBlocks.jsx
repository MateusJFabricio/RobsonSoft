import * as Blockly from 'blockly';

export const jsonGenerator = Blockly.common.createBlockDefinitionsFromJsonArray([{
    "type": "object",
    "message0": "{ %1 %2 }",
    "args0": [
      {
        "type": "input_dummy"
      },
      {
        "type": "input_statement",
        "name": "MEMBERS"
      }
    ],
    "output": null,
    "colour": 230,
  },
  {
    "type": "member",
    "message0": "%1 %2 %3",
    "args0": [
      {
        "type": "field_input",
        "name": "MEMBER_NAME",
        "text": ""
      },
      {
        "type": "field_label",
        "name": "COLON",
        "text": ":"
      },
      {
        "type": "input_value",
        "name": "MEMBER_VALUE"
      }
    ],
    "previousStatement": null,
    "nextStatement": null,
    "colour": 230,
  }]);

  export const toolbox = {
    kind: "categoryToolbox",
    contents: [
      {
        kind: "category",
        name: "Logic",
        colour: "#5C81A6",
        contents: [
          {
            kind: "block",
            type: "controls_if",
          },
          {
            kind: "block",
            type: "logic_compare",
          },
        ],
      },
      {
        kind: "category",
        name: "Math",
        colour: "#5CA65C",
        contents: [
          {
            kind: "block",
            type: "math_round",
          },
          {
            kind: "block",
            type: "math_number",
          },
        ],
      },
      {
        kind: "category",
        name: "Custom",
        colour: "#5CA699",
        contents: [
          {
            kind: "block",
            type: "new_boundary_function",
          },
          {
            kind: "block",
            type: "return",
          },
        ],
      },
    ],
  };