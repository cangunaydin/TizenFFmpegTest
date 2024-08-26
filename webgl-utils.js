// Vertex shader
var vertexShaderSource = `
attribute vec2 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
void main() {
    gl_Position = vec4(a_position, 0.0, 1.0);
    v_texCoord = a_texCoord;
}`;

      // Fragment shader
      var fragmentShaderSource = `
precision mediump float;
uniform sampler2D u_textureY;
uniform sampler2D u_textureU;
uniform sampler2D u_textureV;
varying vec2 v_texCoord;
void main() {
    float y = texture2D(u_textureY, v_texCoord).r;
    float u = texture2D(u_textureU, v_texCoord).r;
    float v = texture2D(u_textureV, v_texCoord).r;
    u = u - 0.5;
    v = v - 0.5;
    float r = y + 1.403 * v;
    float g = y - 0.344 * u - 0.714 * v;
    float b = y + 1.770 * u;
    gl_FragColor = vec4(r, g, b, 1.0);
}`;

      function createTexture(gl, data, width, height) {
        // Create a new texture
        var texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, texture);

        // Set the parameters so we can render any size image
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

        // Upload the image into the texture
        gl.texImage2D(
          gl.TEXTURE_2D,
          0,
          gl.LUMINANCE,
          width,
          height,
          0,
          gl.LUMINANCE,
          gl.UNSIGNED_BYTE,
          data
        );

        return texture;
      }
      function createShaderProgram(
        gl,
        vertexShaderSource,
        fragmentShaderSource
      ) {
        // Create the vertex shader
        var vertexShader = gl.createShader(gl.VERTEX_SHADER);
        gl.shaderSource(vertexShader, vertexShaderSource);
        gl.compileShader(vertexShader);
        if (!gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS)) {
          throw new Error(
            "Error compiling vertex shader: " +
              gl.getShaderInfoLog(vertexShader)
          );
        }

        // Create the fragment shader
        var fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
        gl.shaderSource(fragmentShader, fragmentShaderSource);
        gl.compileShader(fragmentShader);
        if (!gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS)) {
          throw new Error(
            "Error compiling fragment shader: " +
              gl.getShaderInfoLog(fragmentShader)
          );
        }

        // Create the shader program
        var program = gl.createProgram();
        gl.attachShader(program, vertexShader);
        gl.attachShader(program, fragmentShader);
        gl.linkProgram(program);
        if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
          throw new Error(
            "Error linking shader program: " + gl.getProgramInfoLog(program)
          );
        }

        return program;
      }
      function createBuffer(gl, data) {
        // Create a new buffer
        var buffer = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer);

        // Upload the data into the buffer
        gl.bufferData(gl.ARRAY_BUFFER, data, gl.STATIC_DRAW);

        return buffer;
      }