class Model {
    constructor(fileName, shader) {
        this.meshes = [];
        this.selectedMesh = -1;
        this.kSelectedMesh = -1;
        this.modelLoaded = false;
        this.parseFile(fileName, this.meshes, shader);
    }

    draw(shader, camera) {
        for (var i = 0; i < this.meshes.length; i++) {
            this.meshes[i].draw(shader, camera);
        }
    }

    select(index) {
        if (this.selectedMesh != -1) {
            this.deselectAll();
        }
        this.meshes[index].select();
        this.selectedMesh = index;
    }

    selectK(index) {
        if (this.kSelectedMesh != -1) {
            this.deselectK();
        }
        this.meshes[index].selectK();
        this.kSelectedMesh = index;
    }

    deselect() {
        this.meshes[this.selectedMesh].deselect();
        this.selectedMesh = -1;
    }

    deselectK() {
        this.meshes[this.kSelectedMesh].deselect();
        this.kSelectedMesh = -1;
    }

    deselectAll() {
        this.deselect();
        this.deselectK();
    }

    static constructMesh(objectGroup, material, shader) {
        // extract material properties
        var diffuseColor = material.getKd();
        var textureMap = material.getMapKd();
        var specularMap = material.getMapKs();
        var bumpMap = material.getBump();

        // extract geometry data
        var vertices = [];
        var normals = [];
        var uvs = [];
        var indices = [];

        var meshVertices = objectGroup.vertices;
        var meshNormals = objectGroup.normals;
        var meshUVs = objectGroup.uvs;

        for (var i = 0; i < objectGroup.faces.length; i++) {
            var face = objectGroup.faces[i];
            var faceVertices = face.vertexIndices;
            var faceNormals = face.normalIndices;
            var faceUVs = face.uvIndices;

            for (var j = 0; j < faceVertices.length; j++) {
                var vertex = meshVertices[faceVertices[j]];
                var normal = meshNormals[faceNormals[j]];
                var uv = meshUVs[faceUVs[j]];

                vertices.push(vertex[0]);
                vertices.push(vertex[1]);
                vertices.push(vertex[2]);

                normals.push(normal[0]);
                normals.push(normal[1]);
                normals.push(normal[2]);

                uvs.push(uv[0]);
                uvs.push(uv[1]);

                indices.push(indices.length);
            }
        }

        // create mesh
        var vertexList = [];
        var indexList = [];
        var textureList = [];

        // add vertices and indices to mesh
        var i;
        for (i = 0; i < vertices.length / 3; i++) {
            var x = vertices[i * 3 + 0];
            var y = vertices[i * 3 + 1];
            var z = vertices[i * 3 + 2];
            var nx = normals[i * 3 + 0];
            var ny = normals[i * 3 + 1];
            var nz = normals[i * 3 + 2];
            var r = diffuseColor[0];
            var g = diffuseColor[1];
            var b = diffuseColor[2];
            var u = uvs[i * 2 + 0];
            var v = uvs[i * 2 + 1];
            var vertex = new Vertex(vec3(x, y, z), vec3(nx, ny, nz), vec3(r, g, b), vec2(u, v));
            vertexList.push(vertex);
        }

        for (i = 0; i < indices.length; i++) {
            indexList.push(indices[i]);
        }

        // create textures
        if (textureMap != null) {
            var texture = new Texture(textureMap, "diffuse", 0);
            textureList.push(texture);
        }
        if (specularMap != null) {
            var texture = new Texture(specularMap, "specular", 1);
            textureList.push(texture);
        }
        if (bumpMap != null) {
            var texture = new Texture(bumpMap, "normal", 2);
            textureList.push(texture);
        }

        return new Mesh(vertexList, indexList, material, textureList, shader);
    }

    async parseFile(fileName, meshList, shader) {
        // initialize matrix dimensions
        // var indexOf_ = fileName.indexOf("_");
        // var indexOfX = fileName.indexOf("x");
        // this.n = parseInt(fileName.substring(indexOf_ + 1, indexOfX));
        // this.m = parseInt(fileName.substring(indexOfX + 1, fileName.length - 4));
        
        // get renderable objects
        var obj = new OBJ();

        // get obj file contents
        const objResponse = await fetch("https://thecpmills.com/mqp/res/models/" + fileName);
        obj.objFile = await objResponse.text();

        // Split and sanitize OBJ file input
        var objLines = obj.objFile.split('\n');
        objLines = objLines.filter(line => {
            return (line.search(/\S/) !== -1);
        });
        objLines = objLines.map(line => {
            return line.trim();
        });

        // Get lines in OBJ file until material library definition
        var currLine = 0;
        var line = objLines[currLine];
        while (!line.startsWith("mtllib")) { // Material library definition
            line = objLines[++currLine];
        }

        var materialLibraryName = line.split(" ")[1];

        // get MTL file contents
        const mtlResponse = await fetch("https://thecpmills.com/mqp/res/models/" + materialLibraryName);
        var mtlFile = await mtlResponse.text();

        // parse MTL file
        obj.parseMTLFile(mtlFile);

        // parse OBJ file
        obj.parseOBJFile(objLines, currLine + 1);

        // get object groups
        var objectGroups = obj.getObjectGroups();

        // construct meshes
        for (var i = 0; i < objectGroups.length; i++) {
            var objectGroup = objectGroups[i];
            objectGroup.triangulate();
            var material = objectGroups[i].material;
            var mesh = Model.constructMesh(objectGroup, material, shader);
            meshList.push(mesh);
        }

        this.modelLoaded = true;
    }
}