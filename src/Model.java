import java.io.*;
import java.nio.*;
import java.util.*;
import java.util.Map.*;
import de.javagl.obj.*;
import glm.*;

public class Model {
    public List<Mesh> meshes = new ArrayList<Mesh>();
    public String modelPath = System.getProperty("user.dir") + "/res/models/";
    
    public Model(String fileName) throws Exception {
        parseFile(fileName);
    }

    public void draw(Shader shader, Camera camera) throws Exception {
        for (Mesh mesh : meshes) {
            mesh.draw(shader, camera);
        }
    }

    private void parseFile(String fileName) throws Exception {
        // get renderable object and triangulate the faces
        Obj object = ObjUtils.triangulate(ObjUtils.convertToRenderable(ObjReader.read(new FileInputStream(modelPath + fileName))));

        // collect the materials
        List<Mtl> materials = new ArrayList<Mtl>();
        for (String mtlFileName : object.getMtlFileNames()) {
            materials.addAll(MtlReader.read(new FileInputStream(modelPath + mtlFileName)));
        }

        // split the object into groups
        Map<String, Obj> materialGroups = ObjSplitting.splitByMaterialGroups(object);
        for (Entry<String, Obj> entry : materialGroups.entrySet()) {
            String materialName = entry.getKey();
            Obj materialGroup = entry.getValue();

            // get the material for this group
            Mtl material = findMaterial(materialName, materials);
            if (material == null) {
                throw new IllegalArgumentException("No material found with name [" + materialName + "]");
            }

            // construct mesh from material group
            Mesh mesh = constructMesh(materialGroup, material);
            meshes.add(mesh);
        }
    }

    private Mtl findMaterial(String materialName, List<Mtl> materials) {
        for (Mtl material : materials) {
            if (material.getName().equals(materialName)) {
                return material;
            }
        }
        return null;
    }

    private Mesh constructMesh(Obj materialGroup, Mtl materialObject) {
        // extract material properties
        FloatTuple ambientColor = materialObject.getKa();
        FloatTuple diffuseColor = materialObject.getKd();
        FloatTuple specularColor = materialObject.getKs();
        FloatTuple emissiveColor = materialObject.getKe();
        Float shininess = materialObject.getNs();
        Float refractionIndex = materialObject.getNi();
        Float opacity = materialObject.getD();
        // Integer illuminationModel = materialObject.getIllum();
        String textureMap = materialObject.getMapKd();
        String specularMap = materialObject.getMapKs();
        String bumpMap = materialObject.getBump();

        // extract geometry data
        FloatBuffer vertices = ObjData.getVertices(materialGroup);
        FloatBuffer normals = ObjData.getNormals(materialGroup);
        FloatBuffer uvs = ObjData.getTexCoords(materialGroup, 2);
        IntBuffer indices = ObjData.getFaceVertexIndices(materialGroup);
        ArrayList<vec3> tangents = new ArrayList<vec3>();
        ArrayList<vec3> bitangents = new ArrayList<vec3>();

        // create mesh
        ArrayList<Vertex> vertexList = new ArrayList<Vertex>();
        ArrayList<vec3> tangentsList = new ArrayList<vec3>();
        ArrayList<vec3> bitangentsList = new ArrayList<vec3>();
        ArrayList<Integer> indexList = new ArrayList<Integer>();
        ArrayList<Texture> textureList = new ArrayList<Texture>();

        // add vertices and indices to mesh
        int i;
        for (i = 0; i < vertices.capacity() / 3; ++i) {
            float x = vertices.get(i * 3 + 0);
            float y = vertices.get(i * 3 + 1);
            float z = vertices.get(i * 3 + 2);
            float nx = normals.get(i * 3 + 0);
            float ny = normals.get(i * 3 + 1);
            float nz = normals.get(i * 3 + 2);
            float r = diffuseColor.get(0);
            float g = diffuseColor.get(1);
            float b = diffuseColor.get(2);
            float u = uvs.get(i * 2 + 0);
            float v = uvs.get(i * 2 + 1);
            Vertex vertex = new Vertex(new vec3(x, y, z), new vec3(nx, ny, nz), new vec3(r, g, b), new vec2(u, v));
            vertexList.add(vertex);
        }

        for (i = 0; i < indices.capacity(); ++i) {
            indexList.add(indices.get(i));
        }

        // calculate tangents and bitangents
        for (i = 0; i < indexList.size(); i += 3) {
            // get vertex indices
            int vertexIndex0 = indexList.get(i + 0);
            int vertexIndex1 = indexList.get(i + 1);
            int vertexIndex2 = indexList.get(i + 2);

            // get vertex positions
            vec3 position0 = new vec3(vertices.get(vertexIndex0 * 3 + 0), vertices.get(vertexIndex0 * 3 + 1), vertices.get(vertexIndex0 * 3 + 2));
            vec3 position1 = new vec3(vertices.get(vertexIndex1 * 3 + 0), vertices.get(vertexIndex1 * 3 + 1), vertices.get(vertexIndex1 * 3 + 2));
            vec3 position2 = new vec3(vertices.get(vertexIndex2 * 3 + 0), vertices.get(vertexIndex2 * 3 + 1), vertices.get(vertexIndex2 * 3 + 2));

            // get vertex uvs
            vec2 uv0 = new vec2(uvs.get(vertexIndex0 * 2 + 0), uvs.get(vertexIndex0 * 2 + 1));
            vec2 uv1 = new vec2(uvs.get(vertexIndex1 * 2 + 0), uvs.get(vertexIndex1 * 2 + 1));
            vec2 uv2 = new vec2(uvs.get(vertexIndex2 * 2 + 0), uvs.get(vertexIndex2 * 2 + 1));

            // calculate tangent and bitangent
            vec3 deltaPos1 = position1.sub(position0);
            vec3 deltaPos2 = position2.sub(position0);
            vec2 deltaUV1 = uv1.sub(uv0);
            vec2 deltaUV2 = uv2.sub(uv0);

            float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
            vec3 tangent = deltaPos1.scale(deltaUV2.y).sub(deltaPos2.scale(deltaUV1.y)).scale(r);
            vec3 bitangent = deltaPos2.scale(deltaUV1.x).sub(deltaPos1.scale(deltaUV2.x)).scale(r);

            tangents.add(tangent);
            tangents.add(tangent);
            tangents.add(tangent);

            bitangents.add(bitangent);
            bitangents.add(bitangent);
            bitangents.add(bitangent);
        }

        // add tangents and bitangents to mesh
        for (i = 0; i < indexList.size(); i++) {
            int index = indexList.get(i);

            vec3 tangent = tangents.get(index);
            vec3 bitangent = bitangents.get(index);

            tangentsList.add(tangent);
            bitangentsList.add(bitangent);
        }

        // create material
        vec3 ambient = new vec3(ambientColor.get(0), ambientColor.get(1), ambientColor.get(2));
        vec3 specular = new vec3(specularColor.get(0), specularColor.get(1), specularColor.get(2));
        vec3 emissive = null;
        if (emissiveColor != null) {
            emissive = new vec3(emissiveColor.get(0), emissiveColor.get(1), emissiveColor.get(2));
        }
        Material material = new Material(ambient, specular, emissive, shininess, refractionIndex, opacity);
        
        // create textures
        if (textureMap != null) {
            Texture texture = new Texture(textureMap, "diffuse", 0);
            textureList.add(texture);
        }
        if (specularMap != null) {
            Texture texture = new Texture(specularMap, "specular", 1);
            textureList.add(texture);
        }
        if (bumpMap != null) {
            Texture texture = new Texture(bumpMap, "normal", 2);
            textureList.add(texture);
        }

        return new Mesh(vertexList, tangentsList, bitangentsList, indexList, material, textureList);
    }
}