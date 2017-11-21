loadCollada();

let squareRotation = 0.0;

var verts;
var totalCount;
var uvHead;
var uvCount;

const boyAttributes = [
	"Boy01_Hair_Geo-lib",
	"Boy01_Hands_Geo-lib",
	"Boy01_Head_Geo-lib",
	"Boy01_LowerBody_Geo-lib",
	"Boy01_Scarf_Geo-lib",
	"Boy01_Shoes_Geo-lib",
	"Boy01_UpperBody_Geo-lib"
];

var objectVerts = [];
var objectUVs = [];
var objectTextures = [];

function loadCollada(){
	let loader = new THREE.ColladaLoader();

	loader.load('http://localhost:8000/Walking.dae', function(obj){
		console.log(obj);
		verts = obj.library.geometries["Boy01_Head_Geo-lib"].build.polylist.data.attributes.position.array; //.count
		uvHead = obj.library.geometries["Boy01_Head_Geo-lib"].build.polylist.data.attributes.uv.array;
		uvCount = obj.library.geometries["Boy01_Head_Geo-lib"].build.polylist.data.attributes.uv.count;
		totalCount = obj.library.geometries["Boy01_Head_Geo-lib"].build.polylist.data.attributes.position.count;
		//console.log(Object.values(obj.library.geometries)[0]);

		Object.values(obj.library.geometries).forEach((geometry) =>{
			objectVerts.push(geometry.build.polylist.data.attributes.position.array)
			objectVerts.push(geometry.build.polylist.data.attributes.uv.array)
		});

		Object.values(obj.library.images).forEach((image) =>{
			objectTextures.push(image.build);
		});

		//console.log(objectVerts);
		//console.log(objectTextures);

		//console.log(Object.values(obj.library.images));
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

	const buffers = initBuffers(gl);

	const texture = loadTexture(gl, 'http://localhost:8000/textures/Boy_Head_diffuse.png');

	let then = 0;

	function render(now){
		now *= 0.001;
		const deltaTime = now - then;
		then = now;

		drawScene(gl, programInfo, buffers, texture, deltaTime);

		requestAnimationFrame(render);
	}
	requestAnimationFrame(render);

	//drawScene(gl, programInfo, buffers, 0);
}

function initBuffers(gl) {
	const positionBuffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);

	gl.bufferData(gl.ARRAY_BUFFER,
					new Float32Array(verts),
					gl.STATIC_DRAW);

	 const textureCoordBuffer = gl.createBuffer();
	 gl.bindBuffer(gl.ARRAY_BUFFER, textureCoordBuffer);
	 gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(uvHead), gl.STATIC_DRAW);


	return {
		position: positionBuffer,
		textureCoord: textureCoordBuffer,
	};
}

function drawScene(gl, programInfo, buffers, texture, deltaTime){
	gl.clearColor(0.0, 0.0, 0.0, 1.0);
	gl.clearDepth(1.0);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);

	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	const fieldOfView = 45 * Math.PI / 180;   // in radians
	const aspect = gl.canvas.clientWidth / gl.canvas.clientHeight;
	const zNear = 0.1;
	const zFar = 200.0;
	const projectionMatrix = mat4.create();

	mat4.perspective(projectionMatrix,
					fieldOfView,
					aspect,
					zNear,
					zFar);

	const modelViewMatrix = mat4.create();

	mat4.translate(modelViewMatrix,
					modelViewMatrix,
					[-0.0, -160.0, -120.0]);

	mat4.rotate(modelViewMatrix,  // destination matrix
					modelViewMatrix,  // matrix to rotate
					squareRotation,   // amount to rotate in radians
					[0, 1, 0]);       // axis to rotate around

	{
		const numComponents = 3;
		const type = gl.FLOAT;
		const normalize = false;
		const stride = 0;
		const offset = 0;
		gl.bindBuffer(gl.ARRAY_BUFFER, buffers.position);
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
	   gl.bindBuffer(gl.ARRAY_BUFFER, buffers.textureCoord);
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
	gl.bindTexture(gl.TEXTURE_2D, texture);
	gl.uniform1i(programInfo.uniformLocations.uSampler, 0);

		{
			const offset = 0;
			const vertexCount = totalCount;
			gl.drawArrays(gl.TRIANGLES, offset, vertexCount);
		}

	squareRotation += deltaTime;
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