class Mesh {
    constructor(vertices, indices, material, textures, shader) {
        this.vertices = vertices;
        this.indices = indices;
        this.material = material;
        this.backupMaterial = material;
        this.textures = textures;
        this.isSelected = 0;
        this.vao = new VAO();        
    }

    select() {
        this.material = this.backupMaterial;

        var selectedMaterial = new Material("selectedMaterial");
        selectedMaterial.ambient = vec3(1.0, 0.0, 0.0);
        selectedMaterial.diffuse = vec3(1.0, 0.0, 0.0);
        selectedMaterial.specular = vec3(1.0, 0.0, 0.0);
        selectedMaterial.shininess = 10.0;

        this.backupMaterial = this.material;
        this.material = selectedMaterial;
        this.isSelected = 1;
    }

    selectK() {
        this.material = this.backupMaterial;

        var selectedMaterial = new Material("selectedMaterial");
        selectedMaterial.ambient = vec3(1.0, 0.75, 0.8);
        selectedMaterial.diffuse = vec3(1.0, 0.75, 0.8);
        selectedMaterial.specular = vec3(1.0, 0.75, 0.8);
        selectedMaterial.shininess = 10.0;

        this.backupMaterial = this.material;
        this.material = selectedMaterial;
        this.isSelected = 2;
    }

    deselect() {
        this.material = this.backupMaterial;
        this.isSelected = 0;
    }

    draw(shader, camera) {
        gl.useProgram(shader);
        this.vao.bind();

        // bind textures
        var numDiffuse = 0;
        var numSpecular = 0;
        var numNormal = 0;
        for (var i = 0; i < this.textures.length; i++) {
            var num = "";
            var type = this.textures[i].type;
            if (type == "diffuse") {
                num = numDiffuse++;
            } else if (type == "specular") {
                num = numSpecular++;
            } else if (type == "normal") {
                num = numNormal++;
            }
            this.textures[i].textureUnit(shader, type + num, i);
        }

        // define attribute layouts
        var vertexPosition = gl.getAttribLocation(shader, "vertexPosition");
        var vertexNormal = gl.getAttribLocation(shader, "vertexNormal");
        var vertexColor = gl.getAttribLocation(shader, "vertexColor");
        var vertexUV = gl.getAttribLocation(shader, "vertexUV");
        var vertexAmbient = gl.getAttribLocation(shader, "vertexAmbient");
        var vertexSpecular = gl.getAttribLocation(shader, "vertexSpecular");
        var vertexShininess = gl.getAttribLocation(shader, "vertexShininess");

        // duplicate material for each vertex
        this.material.vertexCount = this.vertices.length;

        // bind VAO
        this.vao.bind();
        var vbo = new VBO(this.vertices);
        var materialVBO = new VBO(this.material);
        var ebo = new EBO(this.indices);

        // link VBO attributes to VAO
        this.vao.linkAttribute(vbo, vertexPosition, 3, gl.FLOAT, 8 * 4, 0 * 4);
        this.vao.linkAttribute(vbo, vertexNormal, 3, gl.FLOAT, 8 * 4, 3 * 4);
        this.vao.linkAttribute(vbo, vertexUV, 2, gl.FLOAT, 8 * 4, 6 * 4);
        this.vao.linkAttribute(materialVBO, vertexAmbient, 3, gl.FLOAT, 10 * 4, 0 * 4);
        this.vao.linkAttribute(materialVBO, vertexColor, 3, gl.FLOAT, 10 * 4, 3 * 4);
        this.vao.linkAttribute(materialVBO, vertexSpecular, 3, gl.FLOAT, 10 * 4, 6 * 4);
        this.vao.linkAttribute(materialVBO, vertexShininess, 1, gl.FLOAT, 10 * 4, 9 * 4);

        // bind EBO
        ebo.bind();

        // unbind all to prevent accidental modification
        this.vao.unbind();
        vbo.unbind();
        materialVBO.unbind();

        // bind camera
        gl.uniform3fv(gl.getUniformLocation(shader, "cameraPosition"), flatten(camera.position));
        camera.update(shader, "cameraMatrix");

        // draw elements
        gl.drawElements(gl.TRIANGLES, this.indices.length, gl.UNSIGNED_SHORT, 0);
    }
}