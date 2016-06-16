'use strict'

if (location.hostname.endsWith('.github.io') && location.protocol != 'https:') {
	location.protocol = 'https:';
}

var canvas,
    dark = true,
    device = null,
    keepConnected = false,
    serviceUUID = '5248ccfc-3290-11e6-ac61-9e71128cae77',
    characteristicUUID = 'ecc0e918-3290-11e6-ac61-9e71128cae77'

function relMouseCoords(event){
	var offX = 0,
		offY = 0,
		canvasX = 0,
		canvasY = 0,
		curr = this
	
	do {
		offX += curr.offsetLeft - curr.scrollLeft
		offY += curr.offsetTop - curr.scrollTop
	} while(curr = curr.offsetParent)
    
	canvasX = event.pageX - offX
	canvasY = event.pageY - offY
    
	return {x:canvasX, y:canvasY}
}
HTMLCanvasElement.prototype.relMouseCoords = relMouseCoords

function drawPicker() {
	var CX = canvas.width / 2,
		CY = canvas.height/ 2,
		sx = CX,
		sy = CY,
		g = canvas.getContext("2d"),
		step = 1,
		thickness = (30 + sx) * Math.sin(step * (2*Math.PI) / 360),
		center = dark ? "black" : "white"

	for (var i = 0; i < 360; i+= step) {
		var rad = i * (2*Math.PI) / 360
		var grad = g.createLinearGradient(CX, CY, CX + sx * Math.cos(rad), CY + sy * Math.sin(rad)) 
		grad.addColorStop(0, center)
		grad.addColorStop(1, "hsla("+i+", 100%, 50%, 1.0)")
		g.beginPath()
		g.fillStyle = grad
		g.moveTo(CX, CY)
		g.lineTo(CX + sx * Math.cos(rad), CY + sy * Math.sin(rad))
		g.lineTo(CX + sx * Math.cos(rad) + thickness * Math.sin(rad), CY + sy * Math.sin(rad) - thickness * Math.cos(rad))
		g.lineTo(CX + Math.sin(rad), CY - Math.cos(rad))
		g.closePath()
		g.fill()
	}
}

function togglePicker() {
	dark = !dark
	drawPicker()
}

function pick(event) {
	var coords = canvas.relMouseCoords(event),
		g = canvas.getContext("2d"),
		color = g.getImageData(coords.x, coords.y,1,1).data
	if (color["3"] == 0) {
		color = null
		togglePicker()
	} else {
		color = [color["0"],color["1"],color["2"]]
	}
	return color
}

function resize() {
	var min = Math.min(window.innerWidth, window.innerHeight)
	canvas.width = min / 1.5
	canvas.height = min / 1.5
}

function ajax(color) {
	var sender = new XMLHttpRequest()
	sender.open("GET", "setcolor?"+JSON.stringify(color), true)
	sender.send()
}

function setCharacteristicValue(characteristic, color) {
	let buffer = new ArrayBuffer(3)
	let view = new Uint8Array(buffer)
	view.set(color)
	return characteristic.writeValue(buffer)
}

function getCharacteristic(connectionPromise) {
	return connectionPromise
		.then(server => {
			return server.getPrimaryService(serviceUUID)
		})
		.then(service => {
			return service.getCharacteristic(characteristicUUID)
		})
}

function disconnect() {
	if(!keepConnected) {
		setTimeout(() => device.gatt.disconnect(), 50)
	}
}

function ble(color) {
	getCharacteristic(device.gatt.connect())
	.then(characteristic => setCharacteristicValue(characteristic, color))
	.then(_ => disconnect())
}
	
document.addEventListener("DOMContentLoaded", function() {
	canvas = document.getElementById("colors")
	var button = document.getElementById("chooser")
	var keep = document.getElementById("keep")

	resize()
	drawPicker()	
	
	document.getElementById("outer").addEventListener("click", _ => {
		togglePicker()
	})
	window.addEventListener("resize", _ => {
		resize()
		drawPicker()
	})
	button.addEventListener("click", event => {
		event.stopPropagation()
		navigator.bluetooth.requestDevice({filters: [{services: [serviceUUID]}]})
		.then(d => {
			device = d
		})
	})
	keep.addEventListener("change", event => {
		keepConnected = event.target.checked
		if(device != null) {
			if(keepConnected) {
				device.gatt.connect()
			} else {
				device.gatt.disconnect()
			}
		}
	})
	canvas.addEventListener("click", event => {
		event.stopPropagation()
		var color = pick(event)
		if(color != null) {
			//ajax(color)
			ble(color)
		}
	})
})