class VAO {
    constructor(){
        // this.id = gl.createVertexArray();
    }

    linkAttribute(vbo, layout, numComponents, type, stride, offset){
        vbo.bind();
        gl.vertexAttribPointer(layout, numComponents, type, false, stride, offset);
        gl.enableVertexAttribArray(layout);
        vbo.unbind();
    }

    bind(){
        // gl.bindVertexArray(this.id);
    }

    unbind(){
        // gl.bindVertexArray(null);
    }

    delete(){
        // gl.deleteVertexArray(this.id);
    }
}