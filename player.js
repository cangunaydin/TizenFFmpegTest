var ffmpeg_module = new Proxy(
  new ModuleLoader(
    "wasm_modules/ffmpeg_module/CurrentBin/ffmpeg_module.wasm",
    ffmpeg_module,
    null,
    null
  ),
  ModuleLoaderProxyHandler
);

var images = [];
var currentImageIndex = 0;
// Create the WebGL context
var canvas;
var gl;
var program;
var positionBuffer;
var texCoordBuffer;
// Split the YUV data into separate Y, U, and V arrays
// var width = 480;
// var height = 270;
// Get the locations of the attributes and uniforms
var positionLocation;
var texCoordLocation;
var textureYLocation;
var textureULocation;
var textureVLocation;

function drawFrame(yData, uData, vData, width, height) {
  // Get the current image
  //   let buffer = images[currentImageIndex];

  //   var yData = new Uint8Array(buffer, 0, width * height);
  //   var uData = new Uint8Array(buffer, width * height, (width * height) / 4);
  //   var vData = new Uint8Array(
  //     buffer,
  //     (width * height * 5) / 4,
  //     (width * height) / 4
  //   );

  // Create the Y, U, and V textures
  var textureY = createTexture(gl, yData, width, height);
  var textureU = createTexture(gl, uData, width / 2, height / 2);
  var textureV = createTexture(gl, vData, width / 2, height / 2);

  // Draw the textured quad
  gl.useProgram(program);
  gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
  gl.enableVertexAttribArray(positionLocation);
  gl.vertexAttribPointer(positionLocation, 2, gl.FLOAT, false, 0, 0);
  gl.bindBuffer(gl.ARRAY_BUFFER, texCoordBuffer);
  gl.enableVertexAttribArray(texCoordLocation);
  gl.vertexAttribPointer(texCoordLocation, 2, gl.FLOAT, false, 0, 0);
  gl.activeTexture(gl.TEXTURE0);
  gl.bindTexture(gl.TEXTURE_2D, textureY);
  gl.uniform1i(textureYLocation, 0);
  gl.activeTexture(gl.TEXTURE1);
  gl.bindTexture(gl.TEXTURE_2D, textureU);
  gl.uniform1i(textureULocation, 1);
  gl.activeTexture(gl.TEXTURE2);
  gl.bindTexture(gl.TEXTURE_2D, textureV);
  gl.uniform1i(textureVLocation, 2);
  gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
  // Switch to the next image for the next frame
  currentImageIndex = (currentImageIndex + 1) % images.length;
}

window.onload = function () {
  // Create the WebGL context
  canvas = document.getElementById("myCanvas");
  gl = canvas.getContext("webgl");
  // Create the shader program
  program = createShaderProgram(gl, vertexShaderSource, fragmentShaderSource);

  // Create the position and texCoord buffers
  positionBuffer = createBuffer(
    gl,
    new Float32Array([-1, -1, 1, -1, -1, 1, 1, 1])
  );
  texCoordBuffer = createBuffer(gl, new Float32Array([0, 1, 1, 1, 0, 0, 1, 0]));

  // Get the locations of the attributes and uniforms
  positionLocation = gl.getAttribLocation(program, "a_position");
  texCoordLocation = gl.getAttribLocation(program, "a_texCoord");
  textureYLocation = gl.getUniformLocation(program, "u_textureY");
  textureULocation = gl.getUniformLocation(program, "u_textureU");
  textureVLocation = gl.getUniformLocation(program, "u_textureV");

  document.addEventListener("moduleLoaded", function (e) {
    console.log("ffmpeg module initialized");
    const module = e.detail.module;
    module.onRuntimeInitialized = function () {
      // Fetch the images
      fetch("bun33s.mp4")
        .then((response) => response.arrayBuffer())
        .then((videoBuffer) => {
          // Store the images in memory
          // Convert ArrayBuffer to Uint8Array
          console.log("bun33s.mp4 fetched");
          const uint8Array = new Uint8Array(videoBuffer);

          // Write the file to the Emscripten file system
          module.FS.writeFile("/bun33s.mp4", uint8Array);
          module._play();
          // Start the drawing loop
          // setInterval(drawFrame, 5000);
        });
    };
  });
};
