class Texture {
    constructor(filePath, textureType, slot) {
        this.type = textureType;

        this.image = new Image();
        this.image.crossOrigin = "";
        this.image.src = "/res/textures/" + filePath;
        this.image.onload = function () {
            this.numColorChannels = this.image

            this.id = gl.createTexture();

            gl.activeTexture(gl.TEXTURE0 + slot);
            this.unit = slot;

            gl.activeTexture(slot); // activate the texture unit
            gl.bindTexture(gl.TEXTURE_2D, this.id); // bind texture to texture unit

            // set interpolation type (Configures the type of algorithm that is used to make the image smaller or bigger)
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST); // minification filter
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR_MIPMAP_NEAREST); // magnification filter

            // set texture mapping (Configures the way the texture repeats)
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);

            // extra lines for if gl.CLAMP_TO_BORDER is used for texture mapping
            // var flatColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
            // gl.texParameterfv(gl.TEXTURE_2D, gl.TEXTURE_BORDER_COLOR, flatColor);

            // assign image to texture
            if (this.type == "normal") {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGB, gl.RGBA, gl.UNSIGNED_BYTE, this.image);
            } else {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.SRGB_ALPHA, gl.RGBA, gl.UNSIGNED_BYTE, this.image);
            }

            gl.generateMipmap(gl.TEXTURE_2D); // generate mipmaps
            gl.bindTexture(gl.TEXTURE_2D, null); // unbind texture
        };
    }

    textureUnit(shader, uniform, unit) {
        var textureUnit = gl.getUniformLocation(shader, uniform);
        gl.useProgram(shader);
        gl.uniform1i(textureUnit, unit);
    }

    bind() {
        gl.activeTexture(gl.TEXTURE0 + this.unit);
        gl.bindTexture(gl.TEXTURE_2D, this.id);
    }

    unbind() {
        gl.bindTexture(gl.TEXTURE_2D, null);
    }

    delete() {
        gl.deleteTexture(this.id);
    }

    loadTexture(filePath) {
        var image = new Image();

        // load image from file
        image.src = filePath;

        // return image
        return image;
    }
}

function loadTexture(textureURL) {
    
}