import java.util.*;
import glm.*;
import static org.lwjgl.opengl.GL11C.*;
import static org.lwjgl.opengl.GL20C.*;

public class Mesh {
    public ArrayList<Vertex> vertices;
    public ArrayList<Integer> indices;
    public Material material;
    public ArrayList<Texture> textures;
    public VAO vao = new VAO();

    public Mesh(ArrayList<Vertex> vertices, ArrayList<vec3> tangents, ArrayList<vec3> bitangents, ArrayList<Integer> indices, Material material, ArrayList<Texture> textures) {
        this.vertices = vertices;
        this.indices = indices;
        this.material = material;
        this.textures = textures;

        vao.bind();
        VBO vbo = new VBO(vertices); // generates VBO and links it to vertices
        VBO tangentVBO = new VBO(tangents); // generates VBO and links it to tangents
        VBO bitangentVBO = new VBO(bitangents); // generates VBO and links it to bitangents
        EBO ebo = new EBO(indices); // generates EBO and links it to indices

        // link VBO attributes to VAO
        vao.linkAttribute(vbo, 0, 3, GL_FLOAT, 11 * 4, 0);
        vao.linkAttribute(vbo, 1, 3, GL_FLOAT, 11 * 4, 3 * 4);
        vao.linkAttribute(tangentVBO, 2, 3, GL_FLOAT, 3 * 4, 0);
        vao.linkAttribute(bitangentVBO, 3, 3, GL_FLOAT, 3 * 4, 0);
        vao.linkAttribute(vbo, 4, 3, GL_FLOAT, 11 * 4, 6 * 4);
        vao.linkAttribute(vbo, 5, 2, GL_FLOAT, 11 * 4, 9 * 4);

        // unbind all to prevent accidental modification
        vao.unbind();
        vbo.unbind();
        tangentVBO.unbind();
        bitangentVBO.unbind();
        ebo.unbind();
    }

    public void draw(Shader shader, Camera camera) throws Exception {
        shader.activate();
        vao.bind();

        int numDiffuse = 0;
        int numSpecular = 0;
        int numNormal = 0;

        for (int i = 0; i < textures.size(); i++) {
            String num = "";
            String type = textures.get(i).type;
            if (type.equals("diffuse")) {
                num = Integer.toString(numDiffuse++);
            }  else if (type.equals("specular")) {
                num = Integer.toString(numSpecular++);
            } else if (type.equals("normal")) {
                num = Integer.toString(numNormal++);
            }
            textures.get(i).textureUnit(shader, type + num, i);
            textures.get(i).bind();
        }

        material.setUniforms(shader);

        glUniform3f(glGetUniformLocation(shader.id, "cameraPosition"), camera.position.x, camera.position.y, camera.position.z);
        camera.update(shader, "cameraMatrix");

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
}