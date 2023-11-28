import static org.lwjgl.opengl.GL15.*;
import java.util.*;
import glm.*;

public class VBO {
    public int id;

    public VBO(List<Vertex> vertices) {
        id = glGenBuffers();
        float[] vertexArray = Vertex.vertexArray(vertices);
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, vertexArray, GL_STATIC_DRAW);
    }

    public VBO(ArrayList<vec3> vectors) {
        id = glGenBuffers();
        float[] vectorArray = vectorArray(vectors);
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, vectorArray, GL_STATIC_DRAW);
    }

    public void bind() {
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }

    public void unbind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    public void delete() {
        glDeleteBuffers(id);
    }

    private float[] vectorArray(ArrayList<vec3> vectors) {
        float[] vectorArray = new float[vectors.size() * 3];
        for (int i = 0; i < vectors.size(); i++) {
            vectorArray[i * 3 + 0] = vectors.get(i).x;
            vectorArray[i * 3 + 1] = vectors.get(i).y;
            vectorArray[i * 3 + 2] = vectors.get(i).z;
        }
        return vectorArray;
    }
}
