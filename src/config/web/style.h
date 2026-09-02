#pragma once
#include <Arduino.h>

const char style_css[] PROGMEM = R"rawliteral(
body{
  background-color: #ffe;
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100vh;
  width: 100vw;
  margin: 0;
  font-family: monospace;
  font-size: 1rem;
}
.main-container{
  background-color: #fff;
  max-height: 96vh;
  max-width: 70vw;
  width: 60vh;
  border: 2px solid #aaa;
  border-radius: 5px;
  padding: 0.8rem 1.5rem 1rem 1.5rem;
  box-sizing: border-box;
  display: flex;
  flex-direction: column;
  overflow-y: auto;
}
.container-header{
  font-size: 1.5rem;
  font-weight: bold;
}

.container {
  background-color: white;
  border: 2px solid #aaa;
  padding: 0.8rem 1rem 1rem 1rem;
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
  box-sizing: border-box;
  position: relative;
  width: 100%;
  border-radius: 4px;
  margin-top: 1.5rem;
}
.title {
  background-color: white;
  padding: 5px;
  position: absolute;
  top: 0;
  transform: translateY(calc(-50% - 2px));
}

.input-container {
  display: flex;
  flex-direction: column;
}

.input {
  background-color: #ffe;
  outline: none;
  border: 1px solid #aaa;
  border-radius: 2px;
  padding: 5px;
  box-sizing: border-box;
  font-family: monospace;
  font-size: 1rem;
}

.time-input-container {
  display: flex;
  gap: 5px;
  align-items: center;
}

.time-segment-input {
  box-sizing: border-box;
  flex: 1;
  width: 30%;
  border-radius: 2px;
  background-color: #ffe;
  outline: none;
  border: 1px solid #aaa;
  padding: 5px;
} 


.save-button-container {
  width: 100%;
  display: flex;
  margin-top: 10px;
  justify-content: end;
}

.save-button {
  padding: 4px 30px;
  border-radius: 3px;
  border: 1px solid #aaa;
  background-color: #ddf;
  width: fit-content;
  cursor: pointer;
}











)rawliteral";