class Vertex {
    constructor(position, normal, uv) {
        this.position = position;
        this.normal = normal;
        this.uv = uv;
    }

    position() {
        return this.position;
    }

    normal() {
        return this.normal;
    }

    color() {
        return this.color;
    }

    uv() {
        return this.uv;
    }

    static vertexArray(vertices) {
        var vertexArray = [];
        for (var i = 0; i < vertices.length; i++) {
            vertexArray.push(vertices[i].position[0]);
            vertexArray.push(vertices[i].position[1]);
            vertexArray.push(vertices[i].position[2]);
            vertexArray.push(vertices[i].normal[0]);
            vertexArray.push(vertices[i].normal[1]);
            vertexArray.push(vertices[i].normal[2]);
            vertexArray.push(vertices[i].uv[0]);
            vertexArray.push(vertices[i].uv[1]);
        }
        return vertexArray;
    }
}