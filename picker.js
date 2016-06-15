var canvas,
    button,
    dark,
    device,
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

function anyDevice() {
  // This is the closest we can get for now to get all devices.
  // https://github.com/WebBluetoothCG/web-bluetooth/issues/234
  return Array.from('0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ')
      .map(c => ({namePrefix: c}))
      .concat({name: ''});
}

function ble(color) {
        device.gatt.connect()
	.then(server => {
		return server.getPrimaryService(serviceUUID)
	})
	.then(service => {
		return service.getCharacteristic(characteristicUUID)
	})
	.then(characteristic => {
                writePromise = characteristic.writeValue(color)
		device.gatt.disconnect()
		return writePromise
	})
	//.catch(error => {
	//})
}
	
document.addEventListener("DOMContentLoaded", function() {
	canvas = document.getElementById("colors")
	button = document.getElementById("chooser")
	dark = true
	
	resize()
	drawPicker()	
	
	document.getElementById("outer").addEventListener("click", function () {
		togglePicker()
	})
	window.addEventListener("resize", function () {
		resize()
		drawPicker()
	})
	button.addEventListener("click", function (event) {
		navigator.bluetooth.requestDevice({filters: anyDevice(), optionalServices: [serviceUUID]})
		.then(d => {
			device = d
		})
	})
	canvas.addEventListener("click", function (event) {
		var color = pick(event)
		if(color != null) {
			//ajax(color)
			ble(color)
		}
		event.stopPropagation()
	})
})