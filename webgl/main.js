loadCollada();

let objectRotation = 0.0;


var compositeObject = {};

function loadCollada(){
	let loader = new THREE.ColladaLoader();

	loader.load('http://localhost:8000/Walking.dae', function(obj){

		Object.values(obj.library.geometries).forEach((geometry) =>{
			compositeObject[geometry.name] = {};
			compositeObject[geometry.name].position = geometry.build.polylist.data.attributes.position.array;
			compositeObject[geometry.name].uv = geometry.build.polylist.data.attributes.uv.array;
			compositeObject[geometry.name].verticesCount = geometry.build.polylist.data.attributes.position.count;
		});


		Object.values(obj.library.images).forEach((image, index) =>{
			compositeObject[Object.keys(compositeObject)[index]].texture = image.build;
		});

		console.log(compositeObject);

		main();
	});
}

function main(){

	let canvas = document.getElementById('walkingBoy');
	canvas.height = window.innerHeight;
	canvas.width = window.innerWidth;
	const gl = canvas.getContext('webgl');

	if(!gl){
		alert('unable to initilize webgl');
		return;
	}
	
	const vertexShaderSource = `
		attribute vec4 aVertexPosition;
		attribute vec2 aTextureCoord;

		uniform mat4 uModelViewMarix;
		uniform mat4 uProjectionMatrix;

		varying highp vec2 vTextureCoord;

		void main(){
			gl_Position = uProjectionMatrix * uModelViewMarix * aVertexPosition;
			vTextureCoord = aTextureCoord;
		}
	`;

	const fragmentShaderSource = `
		varying highp vec2 vTextureCoord;

		uniform sampler2D uSampler;

		void main() {
			gl_FragColor = texture2D(uSampler, vTextureCoord);
		}
	`;

	const shaderProgram = initShaderProgram(gl, vertexShaderSource, fragmentShaderSource);

	const programInfo = {
		program: shaderProgram,
		attribLocations: {
			vertexPosition: gl.getAttribLocation(shaderProgram, 'aVertexPosition'),
			textureCoord: gl.getAttribLocation(shaderProgram, 'aTextureCoord'),
		},
		uniformLocations: {
			projectionMatrix: gl.getUniformLocation(shaderProgram, 'uProjectionMatrix'),
			modelViewMatrix: gl.getUniformLocation(shaderProgram, 'uModelViewMarix'),
			uSampler: gl.getUniformLocation(shaderProgram, 'uSampler'),
		},
	};

	let objects = [];

	Object.keys(compositeObject).forEach((objectName) =>{
		let object = {};
		object.buffers = initBuffers(gl, compositeObject[objectName].position, compositeObject[objectName].uv);
		object.texture = loadTexture(gl, 'http://localhost:8000/' + compositeObject[objectName].texture);
		object.verticesCount = compositeObject[objectName].verticesCount;
		objects.push(object);
	});

	let then = 0;

	function render(now){
		now *= 0.001;
		const deltaTime = now - then;
		then = now;

		drawScene(gl, programInfo, objects, deltaTime);

		requestAnimationFrame(render);
	}
	requestAnimationFrame(render);

	//drawScene(gl, programInfo, buffers, 0);
}

function initBuffers(gl, positionVertices, uvCoords) {
	const positionBuffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);

	gl.bufferData(gl.ARRAY_BUFFER,
					new Float32Array(positionVertices),
					gl.STATIC_DRAW);

	 const textureCoordBuffer = gl.createBuffer();
	 gl.bindBuffer(gl.ARRAY_BUFFER, textureCoordBuffer);
	 gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(uvCoords), gl.STATIC_DRAW);


	return {
		position: positionBuffer,
		textureCoord: textureCoordBuffer,
	};
}

function drawScene(gl, programInfo, objects, deltaTime){
	gl.clearColor(0.0, 0.0, 0.0, 1.0);
	gl.clearDepth(1.0);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);

	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	const fieldOfView = 45 * Math.PI / 180;   // in radians
	const aspect = gl.canvas.clientWidth / gl.canvas.clientHeight;
	const zNear = 0.1;
	const zFar = 10000.0;
	const projectionMatrix = mat4.create();

	mat4.perspective(projectionMatrix,
					fieldOfView,
					aspect,
					zNear,
					zFar);

	const modelViewMatrix = mat4.create();

	mat4.translate(modelViewMatrix,
					modelViewMatrix,
					[-0.0, -160.0, -200.0]);

	mat4.rotate(modelViewMatrix,  // destination matrix
					modelViewMatrix,  // matrix to rotate
					objectRotation,   // amount to rotate in radians
					[0, 1, 0]);       // axis to rotate around

	objects.forEach((object) =>{

		{
			const numComponents = 3;
			const type = gl.FLOAT;
			const normalize = false;
			const stride = 0;
			const offset = 0;
			gl.bindBuffer(gl.ARRAY_BUFFER, object.buffers.position);
			gl.vertexAttribPointer(
				programInfo.attribLocations.vertexPosition,
				numComponents,
				type,
				normalize,
				stride,
				offset);

			gl.enableVertexAttribArray(programInfo.attribLocations.vertexPosition);
		}

		{
		   const numComponents = 2;
		   const type = gl.FLOAT;
		   const normalize = false;
		   const stride = 0;
		   const offset = 0;
		   gl.bindBuffer(gl.ARRAY_BUFFER, object.buffers.textureCoord);
		   gl.vertexAttribPointer(
		       programInfo.attribLocations.textureCoord,
		       numComponents,
		       type,
		       normalize,
		       stride,
		       offset);
		   gl.enableVertexAttribArray(
		       programInfo.attribLocations.textureCoord);
		}

		gl.useProgram(programInfo.program);

		gl.uniformMatrix4fv(
			programInfo.uniformLocations.projectionMatrix,
			false,
			projectionMatrix);

		gl.uniformMatrix4fv(
			programInfo.uniformLocations.modelViewMatrix,
			false,
			modelViewMatrix);

		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, object.texture);
		gl.uniform1i(programInfo.uniformLocations.uSampler, 0);

		{
			const offset = 0;
			const vertexCount = object.verticesCount;
			gl.drawArrays(gl.TRIANGLES, offset, vertexCount);
		}
	});

	objectRotation += deltaTime;
}


function initShaderProgram(gl, vertexSource, fragmentSource){
	const vertexShader = loadShader(gl, gl.VERTEX_SHADER, vertexSource);
	const fragmentShader = loadShader(gl, gl.FRAGMENT_SHADER, fragmentSource);

	const shaderProgram = gl.createProgram();
	gl.attachShader(shaderProgram, vertexShader);
	gl.attachShader(shaderProgram, fragmentShader);
	gl.linkProgram(shaderProgram);

	if(!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)){
		alert('unable to initilize shader program: ' + gl.getProgramInfoLog(shaderProgram));
		return null;
	}
	return shaderProgram;
}

function loadShader(gl, type, source){
	const shader = gl.createShader(type);
	gl.shaderSource(shader, source);
	gl.compileShader(shader);
	if(!gl.getShaderParameter(shader, gl.COMPILE_STATUS)){
		alert('unable to compile shaders: ' + gl.getShaderInfoLog(shader));
		gl.deleteShader(shader);
		return null;
	}
	return shader;
}