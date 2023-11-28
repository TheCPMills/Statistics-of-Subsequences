import java.util.*;
import static org.lwjgl.opengl.GL15.*;

public class EBO {
    public int id;

    public EBO(ArrayList<Integer> indices) {
        id = glGenBuffers();
        int[] indexArray = indices.stream().mapToInt(i -> i).toArray();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexArray, GL_STATIC_DRAW);
    }

    public void bind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    }

    public void unbind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    public void delete() {
        glDeleteBuffers(id);
    }
}
