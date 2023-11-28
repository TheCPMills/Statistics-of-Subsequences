class EBO {
    constructor(indices) {
        this.ebo = gl.createBuffer();
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.ebo);
        gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indices), gl.STATIC_DRAW);
    }

    bind() {
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.ebo);
    }

    unbind() {
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
    }

    bufferData(data) {
        gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, data, gl.STATIC_DRAW);
    }
}